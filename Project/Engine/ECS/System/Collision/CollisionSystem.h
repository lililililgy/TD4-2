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

/// ///////////////////////////////////////////////////
/// 衝突判定を計算したときの情報を格納する構造体
/// ///////////////////////////////////////////////////
namespace ONEngine {

struct CollisionInfo {
	Vector3 normal;       /// 衝突法線 (この方向に押し戻す)
	float penetration;    /// めり込み
	Vector3 contactPoint; /// 接触点
};

/// ///////////////////////////////////////////////////
/// 衝突判定の計算を行い、コールバック関数を呼び出すシステム
/// ///////////////////////////////////////////////////
class CollisionSystem : public ECSISystem {
public:
	/// =======================================
	/// public : methods
	/// =======================================

	CollisionSystem();
	virtual ~CollisionSystem() = default;

	void RuntimeUpdate(class ECSGroup* ecs);

	/// コールバック関数の呼び出し
	void CallEnterFunc(const std::string& ecsGroupName);
	void CallStayFunc(const std::string& ecsGroupName);
	void CallExitFunc(const std::string& ecsGroupName);

	/// @brief AとBの押し戻しを行う
	/// @param a Aエンティティのポインタ
	/// @param aState Aエンティティのコリジョン状態
	/// @param b Bエンティティのポインタ
	/// @param bState Bエンティティのコリジョン状態
	/// @param info AとBの衝突情報
	void PushBack(
		class GameEntity* a, CollisionState aState,
		class GameEntity* b, CollisionState bState,
		const CollisionInfo& info
	);
private:
	/// =======================================
	/// private : objects
	/// =======================================

	using CollisionPair = std::pair<class GameEntity*, class GameEntity*>;

	std::deque<CollisionPair> collidedPairs_;

	/// ----- call back ----- ///
	std::deque<CollisionPair> enterPairs_; /// 衝突が開始したペア
	std::deque<CollisionPair> stayPairs_;  /// 衝突が継続しているペア
	std::deque<CollisionPair> exitPairs_;  /// 衝突が終了したペア


	/// collision check 
	using CollisionCheckFunc = std::function<bool(const CollisionPair&, CollisionInfo*)>;
	std::unordered_map<std::string, CollisionCheckFunc> collisionCheckMap_;

};


class SphereCollider;
class BoxCollider;

/*
* Check関数のA->Bに衝突しているかを判定する
* そのためCollisionInfoの法線情報がBからAへの法線になるように設定する
* B(Box) A(Sphere)のとき、法線は衝突した面の外向き法線になる
*/

namespace CheckMethod {
	bool CollisionCheckSphereVsSphere(SphereCollider* s1, SphereCollider* s2, CollisionInfo* info);
	bool CollisionCheckSphereVsBox(SphereCollider* s, BoxCollider* b, CollisionInfo* info);
	bool CollisionCheckBoxVsSphere(BoxCollider* b, SphereCollider* s, CollisionInfo* info);
	bool CollisionCheckBoxVsBox(BoxCollider* b1, BoxCollider* b2, CollisionInfo* info);
}

} /// ONEngine
