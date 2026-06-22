#pragma once

/// engine
#include "../Interface/ECSISystem.h"
#include "Engine/ECS/Component/Array/ComponentArray.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/SphereCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/CircleCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider2D.h"

/// ///////////////////////////////////////////////////////
/// コライダーの可視化を行うシステム
/// ///////////////////////////////////////////////////////
namespace ONEngine {

class ColliderRenderQueueSystem : public ECSISystem {
public:
	/// =========================================================
	/// public : methods
	/// =========================================================

	ColliderRenderQueueSystem();
	~ColliderRenderQueueSystem() override = default;

	void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;
	void RuntimeUpdate(class ECSGroup*) override {};

	/// @brief SphereのデータからGizmoを更新する
	void UpdateSphereCollider(ComponentArray<SphereCollider>* sphereColliderArray);

	/// @brief BoxのデータからGizmoを更新する
	void UpdateBoxCollider(ComponentArray<BoxCollider>* boxColliderArray);

	/// @brief CircleのデータからGizmoを更新する
	void UpdateCircleCollider(ComponentArray<CircleCollider>* circleColliderArray);

	/// @brief Box2DのデータからGizmoを更新する
	void UpdateBoxCollider2D(ComponentArray<BoxCollider2D>* boxCollider2DArray);

};


} /// ONEngine
