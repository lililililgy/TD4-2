#pragma once

/// std
#include <deque>
#include <unordered_map>
#include <functional>
#include <string>

/// engine
#include "../Interface/ECSISystem.h"
#include "Engine/Core/Utility/Utility.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/ICollider.h"
#include "Engine/ECS/System/Collision/CollisionSystem.h"

namespace ONEngine {

/// 3DのCollisionInfoを流用するか、もしくは2D用の情報にする。
/// 押し戻し処理（PushBack）やコールバックでは法線とペネトレーションが必要なので、既存のCollisionInfo(法線はVector3、接触点もVector3)をそのまま使用するのが親和性が高い。
/// そのため、CollisionSystem.hにあるCollisionInfoを使用する。
struct CollisionInfo;

class Collision2DSystem : public ECSISystem {
public:
	/// =======================================
	/// public : methods
	/// =======================================

	Collision2DSystem();
	virtual ~Collision2DSystem() = default;

	void RuntimeUpdate(class ECSGroup* ecs) override;

	/// コールバック関数の呼び出し（2D用）
	void CallEnterFunc(const std::string& ecsGroupName);
	void CallStayFunc(const std::string& ecsGroupName);
	void CallExitFunc(const std::string& ecsGroupName);

	/// @brief AとBの押し戻しを行う（2D平面上）
	void PushBack(
		class GameEntity* a, CollisionState aState,
		class GameEntity* b, CollisionState bState,
		const CollisionInfo& info
	);
private:
	/// =======================================
	/// private : objects
	/// =======================================

	std::deque<CollisionPair> collidedPairs_;

	/// ----- call back ----- ///
	std::deque<CollisionPair> enterPairs_; /// 衝突が開始したペア
	std::deque<CollisionPair> stayPairs_;  /// 衝突が継続しているペア
	std::deque<CollisionPair> exitPairs_;  /// 衝突が終了したペア

	/// collision check 
	using CollisionCheckFunc = std::function<bool(class GameEntity*, class GameEntity*, CollisionInfo*)>;
	std::unordered_map<std::string, CollisionCheckFunc> collisionCheckMap_;

};

class CircleCollider;
class BoxCollider2D;

namespace CheckMethod2D {
	bool CollisionCheckCircleVsCircle(CircleCollider* c1, CircleCollider* c2, CollisionInfo* info);
	bool CollisionCheckCircleVsBox2D(CircleCollider* c, BoxCollider2D* b, CollisionInfo* info);
	bool CollisionCheckBox2DVsCircle(BoxCollider2D* b, CircleCollider* c, CollisionInfo* info);
	bool CollisionCheckBox2DVsBox2D(BoxCollider2D* b1, BoxCollider2D* b2, CollisionInfo* info);
}

} /// ONEngine
