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

	// 1パス目：横ブラー (Scene -> postProcessResult)
	int horizontal = 1;
	command->SetComputeRoot32BitConstants(0, 1, &horizontal, 0);
	command->SetComputeRootDescriptorTable(1, textures[textureIndices_[0]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(2, textures[textureIndices_[1]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(3, textures[textureIndices_[2]].GetUAVGPUHandle());
	command->Dispatch(
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.x), 16),
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.y), 16),
		1
	);

	// バリア遷移：
	// postProcessResult: UAV -> SRV
	// Scene: SRV -> UAV
	const_cast<Asset::Texture&>(textures[textureIndices_[2]]).GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		dxCommand
	);
	const_cast<Asset::Texture&>(textures[textureIndices_[0]]).GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		dxCommand
	);

	// 2パス目：縦ブラー (postProcessResult -> Scene)
	int vertical = 0;
	command->SetComputeRoot32BitConstants(0, 1, &vertical, 0);
	command->SetComputeRootDescriptorTable(1, textures[textureIndices_[2]].GetSRVGPUHandle()); // postProcessResult
	command->SetComputeRootDescriptorTable(2, textures[textureIndices_[1]].GetSRVGPUHandle()); // Flags
	command->SetComputeRootDescriptorTable(3, textures[textureIndices_[0]].GetUAVGPUHandle()); // Scene
	command->Dispatch(
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.x), 16),
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.y), 16),
		1
	);

	// バリア遷移：
	// Scene: UAV -> SRV
	// postProcessResult: SRV -> UAV
	const_cast<Asset::Texture&>(textures[textureIndices_[0]]).GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		dxCommand
	);
	const_cast<Asset::Texture&>(textures[textureIndices_[2]]).GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		dxCommand
	);

	// 大本のsceneテクスチャに結果をコピー
	CopyResource(
		textures[textureIndices_[2]].GetDxResource().Get(),
		textures[textureIndices_[0]].GetDxResource().Get(),
		command
	);
}
