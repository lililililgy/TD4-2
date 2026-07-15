#include "TransformUpdateSystem.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"

void TransformUpdateSystem::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
	Update(ecs);
}

void TransformUpdateSystem::RuntimeUpdate(ECSGroup* ecs) {
	Update(ecs);
}


void TransformUpdateSystem::Update(ECSGroup* ecs) {
	/// ----- Transformの行列を更新する ----- ///

	ComponentArray<Transform>* transformArray = ecs->GetComponentArray<Transform>();
	if (!transformArray || transformArray->GetUsedComponents().empty()) {
		return;
	}

	for (auto& transform : transformArray->GetUsedComponents()) {
		/// 行列を更新しない条件
		if (!transform || !transform->enable) {
			continue;
		}

		/// 行列を更新
		if (GameEntity* owner = transform->GetOwner()) {
			owner->UpdateTransform();
		}
	}

}
