#include "Rigidbody2DUpdateSystem.h"
#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Rigidbody2D/Rigidbody2D.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"
#include "Engine/Core/Utility/Time/Time.h"
#include "Engine/Core/Utility/Time/CPUTimeStamp.h"
#include "Engine/ECS/Component/Array/ComponentArray.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include <algorithm>

namespace ONEngine {

void Rigidbody2DUpdateSystem::RuntimeUpdate(ECSGroup* ecs) {
	CPUTimeStamp::GetInstance().BeginTimeStamp(CPUTimeStampID::PhysicsUpdate);
	if (!ecs) {
		CPUTimeStamp::GetInstance().EndTimeStamp(CPUTimeStampID::PhysicsUpdate);
		return;
	}

	auto* rbArray = ecs->GetComponentArray<Rigidbody2D>();
	if (!rbArray) {
		CPUTimeStamp::GetInstance().EndTimeStamp(CPUTimeStampID::PhysicsUpdate);
		return;
	}

	// 2Dの標準重力値 (Y軸負の方向)
	Vector2 gravity(0.0f, -9.81f);

	for (auto& rb : rbArray->GetUsedComponents()) {
		if (!rb || !rb->enable) {
			continue;
		}

		Vector2 vel = rb->GetVelocity();

		// 重力の適用
		if (rb->GetUseGravity()) {
			vel += gravity * rb->GetGravityScale() * Time::DeltaTime();
		}

		// 軸固定の適用
		if (rb->IsFreezeX()) vel.x = 0.0f;
		if (rb->IsFreezeY()) vel.y = 0.0f;

		rb->SetVelocity(vel);
	}
	CPUTimeStamp::GetInstance().EndTimeStamp(CPUTimeStampID::PhysicsUpdate);
}

} /// namespace ONEngine
