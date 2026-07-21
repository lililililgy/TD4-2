#include "ColliderRenderQueueSystem.h"

using namespace ONEngine;

/// engine
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"

ColliderRenderQueueSystem::ColliderRenderQueueSystem() {}

void ColliderRenderQueueSystem::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
	UpdateSphereCollider(ecs->GetComponentArray<SphereCollider>());
	UpdateBoxCollider(ecs->GetComponentArray<BoxCollider>());
	UpdateCircleCollider(ecs->GetComponentArray<CircleCollider>());
	UpdateBoxCollider2D(ecs->GetComponentArray<BoxCollider2D>());
}


void ColliderRenderQueueSystem::UpdateSphereCollider(ComponentArray<SphereCollider>* sphereColliderArray) {

	/// SphereColliderが存在するか確認(空ならreturn)
	if (!sphereColliderArray || sphereColliderArray->GetUsedComponents().empty()) {
		return;
	}


	for (auto& sphereCollider : sphereColliderArray->GetUsedComponents()) {
		if (!CheckComponentEnable(sphereCollider)) {
			continue;
		}
		GameEntity* owner = sphereCollider->GetOwner();

		Vector3 position = owner->GetPosition();
		float radius = sphereCollider->GetRadius();
		if (sphereCollider->IsUseOwnerScale()) {
			Transform* t = owner->GetTransform();
			radius *= (std::max)({ t->scale.x, t->scale.y, t->scale.z });
		}
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
		if (!CheckComponentEnable(boxCollider)) {
			continue;
		}
		GameEntity* owner = boxCollider->GetOwner();

		const Vector3 position = owner->GetPosition();
		Vector3 size = boxCollider->GetSize();
		if (boxCollider->IsUseOwnerScale()) {
			Transform* t = owner->GetTransform();
			size.x *= t->scale.x;
			size.y *= t->scale.y;
			size.z *= t->scale.z;
		}
		const Quaternion& rotate = owner->GetTransform()->GetRotate();
		// Gizmoを使って立方体を描画する
		Gizmo::DrawWireCube(position, size, rotate, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	}

}

void ColliderRenderQueueSystem::UpdateCircleCollider(ComponentArray<CircleCollider>* circleColliderArray) {
	if (!circleColliderArray || circleColliderArray->GetUsedComponents().empty()) {
		return;
	}

	for (auto& circleCollider : circleColliderArray->GetUsedComponents()) {
		if (!CheckComponentEnable(circleCollider)) {
			continue;
		}
		GameEntity* owner = circleCollider->GetOwner();

		Vector3 position = owner->GetPosition();
		float radius = circleCollider->GetRadius();
		if (circleCollider->IsUseOwnerScale()) {
			Transform* t = owner->GetTransform();
			radius *= (std::max)(t->scale.x, t->scale.y);
		}
		Gizmo::DrawWireSphere2D(position, radius, Vector4(1.0f, 0.5f, 0.0f, 1.0f));
	}
}

void ColliderRenderQueueSystem::UpdateBoxCollider2D(ComponentArray<BoxCollider2D>* boxCollider2DArray) {
	if (!boxCollider2DArray || boxCollider2DArray->GetUsedComponents().empty()) {
		return;
	}

	for (auto& boxCollider2D : boxCollider2DArray->GetUsedComponents()) {
		if (!CheckComponentEnable(boxCollider2D)) {
			continue;
		}
		GameEntity* owner = boxCollider2D->GetOwner();

		const Vector3 position = owner->GetPosition();
		Vector2 size2D = boxCollider2D->GetSize();
		if (boxCollider2D->IsUseOwnerScale()) {
			Vector3 worldScale = owner->GetScale();
			size2D.x *= worldScale.x;
			size2D.y *= worldScale.y;
		}
		Vector3 size(size2D.x, size2D.y, 0.0f);
		const Quaternion rotate = Quaternion::Normalize(owner->GetRotateQuaternion());
		Gizmo::DrawWireCube2D(position, size, rotate, Vector4(0.0f, 1.0f, 0.5f, 1.0f));
	}
}

