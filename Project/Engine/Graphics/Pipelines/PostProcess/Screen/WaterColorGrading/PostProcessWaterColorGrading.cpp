#include "PostProcessWaterColorGrading.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/Component/Array/ComponentArray.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/RendererComponents/ScreenPostEffectTag/ScreenPostEffectTag.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"

void PostProcessWaterColorGrading::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {
	{
		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"Packages/Shader/PostProcess/Screen/WaterColorGrading/WaterColorGrading.cs.hlsl", L"cs_6_6", Shader::Type::cs);

		pipeline_ = std::make_unique<ComputePipeline>();
		pipeline_->SetShader(&shader);

		pipeline_->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 0); // ColorGradingParams
		pipeline_->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 1); // Camera
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // SceneColor
		pipeline_->AddDescriptorRange(1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // WorldPosition
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV); // OutputColor
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 0); // SRV 0
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 1); // SRV 1
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 2); // UAV 0
		pipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_ALL, 0);
		pipeline_->CreatePipeline(dxm->GetDxDevice());
	}

	paramsBuffer_.Create(dxm->GetDxDevice());
}

void PostProcessWaterColorGrading::Execute(const std::string& textureName, DxCommand* dxCommand, Asset::AssetCollection* assetCollection, EntityComponentSystem* entityComponentSystem, ECSGroup* ecsGroup) {
	ECSGroup* currentGroup = ecsGroup ? ecsGroup : entityComponentSystem->GetCurrentGroup();
	if (!currentGroup) {
		static bool logged = false;
		if (!logged) {
			ONEngine::Console::LogWarning("[WaterColorGrading] Execute skipped: currentGroup is null", LogCategory::Engine);
			logged = true;
		}
		return;
	}

	CameraComponent* camera = nullptr;
	if (textureName.find("debug") != std::string::npos) {
		ECSGroup* debugGroup = entityComponentSystem->GetECSGroup("Debug");
		if (debugGroup) {
			camera = debugGroup->GetMainCamera2D();
		}
	}
	if (!camera) {
		camera = currentGroup->GetMainCamera2D();
	}

	if (!camera) {
		static bool logged = false;
		if (!logged) {
			ONEngine::Console::LogWarning("[WaterColorGrading] Execute skipped: Camera2D is null. TextureName=" + textureName, LogCategory::Engine);
			logged = true;
		}
		return;
	}

	ComponentArray<ScreenPostEffectTag>* screenPostEffectTagArray = currentGroup->GetComponentArray<ScreenPostEffectTag>();
	if (!screenPostEffectTagArray || screenPostEffectTagArray->GetUsedComponents().empty()) {
		static bool logged = false;
		if (!logged) {
			ONEngine::Console::LogWarning("[WaterColorGrading] Execute skipped: ScreenPostEffectTag not found in group: " + currentGroup->GetGroupName(), LogCategory::Engine);
			logged = true;
		}
		return;
	}

	ScreenPostEffectTag* tag = nullptr;
	for (auto& comp : screenPostEffectTagArray->GetUsedComponents()) {
		if (!comp || !comp->enable) {
			continue;
		}
		tag = comp;
		break;
	}

	if (!tag) {
		static bool logged = false;
		if (!logged) {
			ONEngine::Console::LogWarning("[WaterColorGrading] Execute skipped: Active ScreenPostEffectTag is null", LogCategory::Engine);
			logged = true;
		}
		return;
	}

	if (!tag->GetPostEffectEnable(PostEffectType_WaterColorGrading)) {
		return;
	}

	static bool isFirstExec = true;
	if (isFirstExec) {
		ONEngine::Console::Log("[WaterColorGrading] Execution started successfully. Group=" + currentGroup->GetGroupName() + ", TextureName=" + textureName, LogCategory::Engine);
		isFirstExec = false;
	}

	if (!camera->GetCameraPosBuffer().Get()) {
		static bool logged = false;
		if (!logged) {
			ONEngine::Console::LogWarning("[WaterColorGrading] Execute skipped: Camera pos buffer not initialized", LogCategory::Engine);
			logged = true;
		}
		return;
	}

	Vector2 offset = ScreenPostEffectTag::GetDispatchStartOffset(ecsGroup, entityComponentSystem);
	Vector2 dispatchSize = ScreenPostEffectTag::GetDispatchSize(ecsGroup, entityComponentSystem);
	// 定数バッファの更新
	ColorGradingParams params;
	params.absorption = tag->GetWaterAbsorptionCoefficients();
	params.contrast = tag->GetWaterContrast();
	params.saturation = tag->GetWaterSaturation();
	params.colorFilter = tag->GetWaterColorFilter();
	params.offsetX = static_cast<int32_t>(offset.x);
	params.offsetY = static_cast<int32_t>(offset.y);
	params.virtualWidth = static_cast<int32_t>(dispatchSize.x);
	params.virtualHeight = static_cast<int32_t>(dispatchSize.y);
	paramsBuffer_.SetMappedData(params);

	pipeline_->SetPipelineStateForCommandList(dxCommand);

	auto command = dxCommand->GetCommandList();
	auto& textures = assetCollection->GetTextures();

	textureIndices_[0] = assetCollection->GetTextureIndex(textureName + "Scene");
	textureIndices_[1] = assetCollection->GetTextureIndex(textureName + "WorldPosition");
	textureIndices_[2] = assetCollection->GetTextureIndex("postProcessResult");

	if (textureIndices_[0] == -1 || textureIndices_[1] == -1 || textureIndices_[2] == -1) {
		static bool logged = false;
		if (!logged) {
			ONEngine::Console::LogWarning("[WaterColorGrading] Execute skipped: Texture index not found. Name=" + textureName + 
				" Indices=" + std::to_string(textureIndices_[0]) + "," + std::to_string(textureIndices_[1]) + "," + std::to_string(textureIndices_[2]), 
				LogCategory::Engine);
			logged = true;
		}
		return;
	}

	paramsBuffer_.BindForComputeCommandList(command, CBV_PARAMS);
	camera->GetCameraPosBuffer().BindForComputeCommandList(command, CBV_CAMERA);

	command->SetComputeRootDescriptorTable(SRV_SCENE_COLOR, textures[textureIndices_[0]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(SRV_SCENE_WORLD_POSITION, textures[textureIndices_[1]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(UAV_OUTPUT_COLOR, textures[textureIndices_[2]].GetUAVGPUHandle());

	command->Dispatch(
		Math::DivideAndRoundUp(static_cast<uint32_t>(dispatchSize.x), 16),
		Math::DivideAndRoundUp(static_cast<uint32_t>(dispatchSize.y), 16),
		1
	);

	/// 大本のsceneテクスチャに結果をコピー
	CopyResource(
		textures[textureIndices_[2]].GetDxResource().Get(),
		textures[textureIndices_[0]].GetDxResource().Get(),
		command
	);
}
