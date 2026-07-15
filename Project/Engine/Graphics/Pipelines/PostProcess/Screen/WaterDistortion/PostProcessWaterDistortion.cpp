#include "PostProcessWaterDistortion.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/Component/Array/ComponentArray.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/RendererComponents/ScreenPostEffectTag/ScreenPostEffectTag.h"
#include "Engine/Core/Utility/Time/Time.h"

void PostProcessWaterDistortion::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {
	{
		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"Packages/Shader/PostProcess/Screen/WaterDistortion/WaterDistortion.cs.hlsl", L"cs_6_6", Shader::Type::cs);

		pipeline_ = std::make_unique<ComputePipeline>();
		pipeline_->SetShader(&shader);

		pipeline_->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 0); // DistortionParams
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // SceneColor
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV); // OutputColor
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 0); // SRV
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 1); // UAV
		pipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_ALL, 0);
		pipeline_->CreatePipeline(dxm->GetDxDevice());
	}

	paramsBuffer_.Create(dxm->GetDxDevice());
}

void PostProcessWaterDistortion::Execute(const std::string& textureName, DxCommand* dxCommand, Asset::AssetCollection* assetCollection, EntityComponentSystem* entityComponentSystem, ECSGroup* ecsGroup) {
	ECSGroup* currentGroup = ecsGroup ? ecsGroup : entityComponentSystem->GetCurrentGroup();
	if (!currentGroup) {
		static bool logged = false;
		if (!logged) {
			ONEngine::Console::LogWarning("[WaterDistortion] Execute skipped: currentGroup is null", LogCategory::Engine);
			logged = true;
		}
		return;
	}

	ComponentArray<ScreenPostEffectTag>* screenPostEffectTagArray = currentGroup->GetComponentArray<ScreenPostEffectTag>();
	if (!screenPostEffectTagArray || screenPostEffectTagArray->GetUsedComponents().empty()) {
		static bool logged = false;
		if (!logged) {
			ONEngine::Console::LogWarning("[WaterDistortion] Execute skipped: ScreenPostEffectTag not found in group: " + currentGroup->GetGroupName(), LogCategory::Engine);
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
			ONEngine::Console::LogWarning("[WaterDistortion] Execute skipped: Active ScreenPostEffectTag is null", LogCategory::Engine);
			logged = true;
		}
		return;
	}

	if (!tag->GetPostEffectEnable(PostEffectType_WaterDistortion)) {
		return;
	}

	static bool isFirstExec = true;
	if (isFirstExec) {
		ONEngine::Console::Log("[WaterDistortion] Execution started successfully. Group=" + currentGroup->GetGroupName() + ", TextureName=" + textureName, LogCategory::Engine);
		isFirstExec = false;
	}

	// 定数バッファの更新
	Vector2 offset = ScreenPostEffectTag::GetDispatchStartOffset(ecsGroup, entityComponentSystem);
	DistortionParams params;
	params.strength = tag->GetWaterDistortionStrength();
	params.speed = tag->GetWaterDistortionSpeed();
	params.frequency = tag->GetWaterDistortionFrequency();
	params.time = Time::GetTime();
	params.offsetX = static_cast<int32_t>(offset.x);
	params.offsetY = static_cast<int32_t>(offset.y);
	params.padding[0] = 0;
	params.padding[1] = 0;
	paramsBuffer_.SetMappedData(params);

	pipeline_->SetPipelineStateForCommandList(dxCommand);

	auto command = dxCommand->GetCommandList();
	auto& textures = assetCollection->GetTextures();
	textureIndices_[0] = assetCollection->GetTextureIndex(textureName + "Scene");
	textureIndices_[1] = assetCollection->GetTextureIndex("postProcessResult");

	if (textureIndices_[0] == -1 || textureIndices_[1] == -1) {
		static bool logged = false;
		if (!logged) {
			ONEngine::Console::LogWarning("[WaterDistortion] Execute skipped: Texture index not found. Indices=" + 
				std::to_string(textureIndices_[0]) + "," + std::to_string(textureIndices_[1]), LogCategory::Engine);
			logged = true;
		}
		return;
	}

	paramsBuffer_.BindForComputeCommandList(command, CBV_PARAMS);
	command->SetComputeRootDescriptorTable(SRV_SCENE_COLOR, textures[textureIndices_[0]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(UAV_OUTPUT_COLOR, textures[textureIndices_[1]].GetUAVGPUHandle());

	Vector2 dispatchSize = ScreenPostEffectTag::GetDispatchSize(ecsGroup, entityComponentSystem);
	command->Dispatch(
		Math::DivideAndRoundUp(static_cast<uint32_t>(dispatchSize.x), 16),
		Math::DivideAndRoundUp(static_cast<uint32_t>(dispatchSize.y), 16),
		1
	);

	/// 大本のsceneテクスチャに結果をコピー
	CopyResource(
		textures[textureIndices_[1]].GetDxResource().Get(),
		textures[textureIndices_[0]].GetDxResource().Get(),
		command
	);
}
