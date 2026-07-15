#include "PostProcessBloom.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"

void PostProcessBloom::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {
	Console::Log("[Bloom] Initializing PostProcessBloom...", LogCategory::Engine);
	// 1. 高輝度抽出パイプラインの初期化
	{
		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"Packages/Shader/PostProcess/PerObject/Bloom/BloomExtract.cs.hlsl", L"cs_6_6", Shader::Type::cs);

		extractPipeline_ = std::make_unique<ComputePipeline>();
		extractPipeline_->SetShader(&shader);

		extractPipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // scene color
		extractPipeline_->AddDescriptorRange(1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // flags
		extractPipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV); // bloomBright (UAV)
		extractPipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 0);
		extractPipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 1);
		extractPipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 2);
		extractPipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_ALL, 0);
		extractPipeline_->CreatePipeline(dxm->GetDxDevice());
	}

	// 2. ぼかしパイプラインの初期化
	{
		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"./Packages/Shader/PostProcess/PerObject/GaussianBlur/GaussianBlur.cs.hlsl", L"cs_6_6", Shader::Type::cs);

		blurPipeline_ = std::make_unique<ComputePipeline>();
		blurPipeline_->SetShader(&shader);

		blurPipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // bloomBright
		blurPipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV); // bloomBlur
		blurPipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 0);
		blurPipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 1);
		blurPipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_ALL, 0);
		blurPipeline_->CreatePipeline(dxm->GetDxDevice());
	}

	// 3. 合成パイプラインの初期化
	{
		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"Packages/Shader/PostProcess/PerObject/Bloom/BloomComposite.cs.hlsl", L"cs_6_6", Shader::Type::cs);

		compositePipeline_ = std::make_unique<ComputePipeline>();
		compositePipeline_->SetShader(&shader);

		compositePipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // original scene (t0)
		compositePipeline_->AddDescriptorRange(1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // bloomBlur (t1)
		compositePipeline_->AddDescriptorRange(2, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // flagsTex (t2)
		compositePipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV); // postProcessResult (u0)
		compositePipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 0);
		compositePipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 1);
		compositePipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 2);
		compositePipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 3);
		compositePipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_ALL, 0);
		compositePipeline_->CreatePipeline(dxm->GetDxDevice());
	}
}

void PostProcessBloom::Execute(
	const std::string& textureName,
	DxCommand* dxCommand,
	Asset::AssetCollection* assetCollection,
	[[maybe_unused]] EntityComponentSystem* entityComponentSystem,
	[[maybe_unused]] ECSGroup* ecsGroup
) {
	auto command = dxCommand->GetCommandList();
	auto& textures = assetCollection->GetTextures();

	textureIndices_[0] = assetCollection->GetTextureIndex(textureName + "Scene");
	textureIndices_[1] = assetCollection->GetTextureIndex(textureName + "Flags");
	textureIndices_[2] = assetCollection->GetTextureIndex("bloomBright");
	textureIndices_[3] = assetCollection->GetTextureIndex("bloomBlur");
	textureIndices_[4] = assetCollection->GetTextureIndex("postProcessResult");

	for (int i = 0; i < 5; ++i) {
		if (textureIndices_[i] == -1) {
			Console::LogError("[Bloom] Failed to get texture index for slot " + std::to_string(i) + " (textureName: " + textureName + ")");
			return;
		}
	}

	uint32_t dispatchX = Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.x), 16);
	uint32_t dispatchY = Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.y), 16);

	// 1. 高輝度抽出
	extractPipeline_->SetPipelineStateForCommandList(dxCommand);
	command->SetComputeRootDescriptorTable(0, textures[textureIndices_[0]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(1, textures[textureIndices_[1]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(2, textures[textureIndices_[2]].GetUAVGPUHandle());
	command->Dispatch(dispatchX, dispatchY, 1);

	// bloomBright を UAV から SRV に遷移
	const_cast<Asset::Texture&>(textures[textureIndices_[2]]).GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		dxCommand
	);

	// 2. ブラーをかける
	blurPipeline_->SetPipelineStateForCommandList(dxCommand);
	command->SetComputeRootDescriptorTable(0, textures[textureIndices_[2]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(1, textures[textureIndices_[3]].GetUAVGPUHandle());
	command->Dispatch(dispatchX, dispatchY, 1);

	// bloomBlur を UAV から SRV に遷移、bloomBright は UAV に戻す
	const_cast<Asset::Texture&>(textures[textureIndices_[3]]).GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		dxCommand
	);
	const_cast<Asset::Texture&>(textures[textureIndices_[2]]).GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		dxCommand
	);

	// 3. 元のカラーとブレンドして合成
	compositePipeline_->SetPipelineStateForCommandList(dxCommand);
	command->SetComputeRootDescriptorTable(0, textures[textureIndices_[0]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(1, textures[textureIndices_[3]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(2, textures[textureIndices_[4]].GetUAVGPUHandle());
	command->SetComputeRootDescriptorTable(3, textures[textureIndices_[1]].GetSRVGPUHandle());
	command->Dispatch(dispatchX, dispatchY, 1);

	// bloomBlur を UAV に戻す
	const_cast<Asset::Texture&>(textures[textureIndices_[3]]).GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		dxCommand
	);

	// 最終結果を元のSceneにコピーして戻す
	CopyResource(
		textures[textureIndices_[4]].GetDxResource().Get(),
		textures[textureIndices_[0]].GetDxResource().Get(),
		command
	);
}
