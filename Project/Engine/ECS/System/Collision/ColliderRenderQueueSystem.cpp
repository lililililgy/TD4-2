#include "ColliderRenderQueueSystem.h"

using namespace ONEngine;

/// engine
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"

ColliderRenderQueueSystem::ColliderRenderQueueSystem() {}

void ColliderRenderQueueSystem::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
	UpdateSphereCollider(ecs->GetComponentArray<SphereCollider>());
	UpdateBoxCollider(ecs->GetComponentArray<BoxCollider>());
}


void ColliderRenderQueueSystem::UpdateSphereCollider(ComponentArray<SphereCollider>* sphereColliderArray) {

	/// SphereColliderが存在するか確認(空ならreturn)
	if (!sphereColliderArray || sphereColliderArray->GetUsedComponents().empty()) {
		return;
	}


	for (auto& sphereCollider : sphereColliderArray->GetUsedComponents()) {
		if (!sphereCollider) {
			continue; // 無効なコライダーはスキップ
		}

		GameEntity* owner = sphereCollider->GetOwner();
		if (!owner) {
			continue; // オーナーが無効な場合はスキップ
		}

		Vector3 position = owner->GetPosition();
		float radius = sphereCollider->GetRadius();
		// Gizmoを使って球体を描画する
		Gizmo::DrawWireSphere(position, radius, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	}

}

void ColliderRenderQueueSystem::UpdateBoxCollider(ComponentArray<BoxCollider>* boxColliderArray) {

	/// BoxColliderが存在するか確認(空ならreturn)
	if (!boxColliderArray || boxColliderArray->GetUsedComponents().empty()) {
		return;
	}

	/// gizmoを使って表示する
	for (auto& boxCollider : boxColliderArray->GetUsedComponents()) {
		if (!boxCollider) {
			continue; // 無効なコライダーはスキップ
		}
		GameEntity* owner = boxCollider->GetOwner();
		if (!owner) {
			continue; // オーナーが無効な場合はスキップ
		}

		const Vector3 position = owner->GetPosition();
		const Vector3& size = boxCollider->GetSize();
		const Quaternion& rotate = owner->GetTransform()->GetRotate();
		// Gizmoを使って立方体を描画する
		Gizmo::DrawWireCube(position, size, rotate, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	}

}

