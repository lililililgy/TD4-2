#include "ShadowCasterUpdateSystem.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ShadowCaster/ShadowCaster.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Light/Light.h"

ShadowCasterUpdateSystem::ShadowCasterUpdateSystem() = default;
ShadowCasterUpdateSystem::~ShadowCasterUpdateSystem() = default;

void ShadowCasterUpdateSystem::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
	if (!DebugConfig::isDebugging) {
		Update(ecs);
	}

}

void ShadowCasterUpdateSystem::RuntimeUpdate(ECSGroup* ecs) {
	if (DebugConfig::isDebugging) {
		Update(ecs);
	}
}

void ShadowCasterUpdateSystem::Update(ECSGroup* ecs) {
	/// ----- ShadowCasterの更新 ----- ///


	/// ShadowCasterの配列を取得&空ではないかチェック
	ComponentArray<ShadowCaster>* shadowCasterArray = ecs->GetComponentArray<ShadowCaster>();
	if (!shadowCasterArray || shadowCasterArray->GetUsedComponents().empty()) {
		return;
	}


	/// DirectionalLightの配列を取得&空ではないかチェック
	ComponentArray<DirectionalLight>* dirLightArray = ecs->GetComponentArray<DirectionalLight>();
	DirectionalLight* dirLight = nullptr;
	if (dirLightArray && !dirLightArray->GetUsedComponents().empty()) {
		dirLight = dirLightArray->GetUsedComponents().front();
	}

	for (auto& shadowCaster : shadowCasterArray->GetUsedComponents()) {
		shadowCaster->CreateShadowCaster();

		/// DirectionalLightが存在する場合、ライトビュー行列を計算
		if (dirLight) {
			shadowCaster->CalculationLightViewMatrix(ecs, dirLight);
		}
	}
}
