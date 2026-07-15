#include "PostProcessGaussianBlurPerObject.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"

void PostProcessGaussianBlurPerObject::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {

	{
		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"./Packages/Shader/PostProcess/PerObject/GaussianBlur/GaussianBlur.cs.hlsl", L"cs_6_6", Shader::Type::cs);

		pipeline_ = std::make_unique<ComputePipeline>();
		pipeline_->SetShader(&shader);

		pipeline_->Add32BitConstant(D3D12_SHADER_VISIBILITY_ALL, 0, 1); // horizontal (b0)
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); /// scene tex (t0)
		pipeline_->AddDescriptorRange(1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); /// flagsTex (t1)
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV); /// output tex (u0)
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 0);
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 1);
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 2);
		pipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_ALL, 0);

		pipeline_->CreatePipeline(dxm->GetDxDevice());

	}

}

void PostProcessGaussianBlurPerObject::Execute(const std::string& textureName, DxCommand* dxCommand, Asset::AssetCollection* assetCollection, EntityComponentSystem* /*entityComponentSystem*/, ECSGroup* /*ecsGroup*/) {
	pipeline_->SetPipelineStateForCommandList(dxCommand);

	auto command = dxCommand->GetCommandList();
	auto& textures = assetCollection->GetTextures();
	textureIndices_[0] = assetCollection->GetTextureIndex(textureName + "Scene");
	textureIndices_[1] = assetCollection->GetTextureIndex(textureName + "Flags");
	textureIndices_[2] = assetCollection->GetTextureIndex("postProcessResult");
	textureIndices_[3] = assetCollection->GetTextureIndex("bloomBlur"); // 中間バッファとして借りる

	// 1パス目：横ブラー (Scene -> bloomBlur)
	int horizontal = 1;
	command->SetComputeRoot32BitConstants(0, 1, &horizontal, 0);
	command->SetComputeRootDescriptorTable(1, textures[textureIndices_[0]].GetSRVGPUHandle()); // t0: Scene
	command->SetComputeRootDescriptorTable(2, textures[textureIndices_[1]].GetSRVGPUHandle()); // t1: Flags
	command->SetComputeRootDescriptorTable(3, textures[textureIndices_[3]].GetUAVGPUHandle()); // u0: bloomBlur
	command->Dispatch(
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.x), 16),
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.y), 16),
		1
	);

	// バリア遷移：
	// bloomBlur: UAV -> SRV (次のパスで入力としてサンプリングするため)
	const_cast<Asset::Texture&>(textures[textureIndices_[3]]).GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		dxCommand
	);

	// 2パス目：縦ブラー (bloomBlur -> postProcessResult)
	int vertical = 0;
	command->SetComputeRoot32BitConstants(0, 1, &vertical, 0);
	command->SetComputeRootDescriptorTable(1, textures[textureIndices_[3]].GetSRVGPUHandle()); // t0: bloomBlur
	command->SetComputeRootDescriptorTable(2, textures[textureIndices_[1]].GetSRVGPUHandle()); // t1: Flags
	command->SetComputeRootDescriptorTable(3, textures[textureIndices_[2]].GetUAVGPUHandle()); // u0: postProcessResult (最終出力先)
	command->Dispatch(
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.x), 16),
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.y), 16),
		1
	);

	// バリア遷移：
	// bloomBlur: SRV -> UAV (次のフレームのために UAV に戻す)
	const_cast<Asset::Texture&>(textures[textureIndices_[3]]).GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		dxCommand
	);
}
