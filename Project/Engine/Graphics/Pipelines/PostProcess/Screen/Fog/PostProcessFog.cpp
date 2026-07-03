#include "PostProcessFog.h"

/// engine
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"

namespace ONEngine {

void PostProcessFog::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {

	{	/// shader
		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"./Packages/Shader/PostProcess/Screen/Fog/Fog.cs.hlsl", L"cs_6_6", Shader::Type::cs);

		pipeline_ = std::make_unique<ComputePipeline>();
		pipeline_->SetShader(&shader);

		pipeline_->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 0); // FogParams
		pipeline_->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 1); // CameraPos
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // SceneColor
		pipeline_->AddDescriptorRange(1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // SceneWorldPosition
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV); // OutputColor
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 0); // SRV
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 1); // SRV
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 2); // UAV

		pipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_ALL, 0);

		pipeline_->CreatePipeline(dxm->GetDxDevice());
	}

}

void PostProcessFog::Execute(const std::string& textureName, DxCommand* dxCommand, Asset::AssetCollection* assetCollection, EntityComponentSystem* entityComponentSystem, ECSGroup* ecsGroup) {

	ECSGroup* currentGroup = ecsGroup ? ecsGroup : entityComponentSystem->GetCurrentGroup();
	if(!currentGroup) {
		ONEngine::Console::LogError("PostProcessFog::Execute: current ECS group is null");
		return;
	}

	CameraComponent* mainCamera = currentGroup->GetMainCamera();
	if(!mainCamera) {
		ONEngine::Console::LogError("PostProcessFog::Execute: main camera is null");
		return;
	}


	pipeline_->SetPipelineStateForCommandList(dxCommand);

	auto cmdList = dxCommand->GetCommandList();

	// バッファが生成されているかチェック（シーン遷移直後などは未生成の可能性がある）
	if (!mainCamera->GetCameraPosBuffer().Get() || !mainCamera->GetFogParamsBuffer().Get()) {
		ONEngine::Console::LogWarning("PostProcessFog::Execute: main camera buffers not initialized yet.");
		return;
	}

	mainCamera->GetCameraPosBuffer().BindForComputeCommandList(cmdList, CBV_CAMERA_POS);
	mainCamera->GetFogParamsBuffer().BindForComputeCommandList(cmdList, CBV_FOG_PARAMS);

#ifdef DEBUG_MODE
	if(textureName.find_last_of("debug") != std::string::npos) {
		if(ECSGroup* debugGroup = entityComponentSystem->GetECSGroup("Debug")) {
			if(CameraComponent* debugCamera = debugGroup->GetMainCamera()) {
				if (debugCamera->GetCameraPosBuffer().Get()) {
					debugCamera->GetCameraPosBuffer().BindForComputeCommandList(cmdList, CBV_CAMERA_POS);
				}
			}
		}
	}
#endif



	auto& textures = assetCollection->GetTextures();

	textureIndices_[0] = assetCollection->GetTextureIndex(textureName + "Scene");
	textureIndices_[1] = assetCollection->GetTextureIndex(textureName + "WorldPosition");
	textureIndices_[2] = assetCollection->GetTextureIndex("postProcessResult");

	// 全て存在するかチェック
	if (textureIndices_[0] == -1 || textureIndices_[1] == -1 || textureIndices_[2] == -1) {
		ONEngine::Console::LogWarning("PostProcessFog::Execute: One or more required textures not found.");
		return;
	}

	cmdList->SetComputeRootDescriptorTable(SRV_SCENE_COLOR, textures[textureIndices_[0]].GetSRVGPUHandle());
	cmdList->SetComputeRootDescriptorTable(SRV_SCENE_WORLD_POSITION, textures[textureIndices_[1]].GetSRVGPUHandle());
	cmdList->SetComputeRootDescriptorTable(UAV_OUTPUT_COLOR, textures[textureIndices_[2]].GetUAVGPUHandle());

	cmdList->Dispatch(
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.x), 16),
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.y), 16),
		1
	);

	/// 大本のsceneテクスチャに結果をコピー
	CopyResource(
		textures[textureIndices_[2]].GetDxResource().Get(),
		textures[textureIndices_[0]].GetDxResource().Get(),
		cmdList
	);

}

} /// namespace ONEngine