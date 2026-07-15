#include "CollisionSystem.h"

using namespace ONEngine;

/// std
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <utility>

/// engine
#include "Engine/Core/Utility/Utility.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/Script/MonoScriptEngine.h"
#include "Engine/Core/Utility/Time/CPUTimeStamp.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/SphereCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/CollisionCheck/CollisionCheck.h"


namespace std {
template <>
struct hash<std::pair<int, int>> {
	std::size_t operator()(const std::pair<int, int>& p) const {
		return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
	}
};
}


CollisionSystem::CollisionSystem() {

	std::string sphereCompName = typeid(SphereCollider).name();
	std::string boxCompName = typeid(BoxCollider).name();

	/// 関数の登録をする
	collisionCheckMap_[sphereCompName + "Vs" + sphereCompName] = [](const CollisionPair& pair, CollisionInfo* info) -> bool {
		return CheckMethod::CollisionCheckSphereVsSphere(
			pair.first->GetComponent<SphereCollider>(),
			pair.second->GetComponent<SphereCollider>(),
			info
		);
	};

	collisionCheckMap_[sphereCompName + "Vs" + boxCompName] = [](const CollisionPair& pair, CollisionInfo* info) -> bool {
		return CheckMethod::CollisionCheckSphereVsBox(
			pair.first->GetComponent<SphereCollider>(),
			pair.second->GetComponent<BoxCollider>(),
			info
		);
	};

	collisionCheckMap_[boxCompName + "Vs" + sphereCompName] = [](const CollisionPair& pair, CollisionInfo* info) -> bool {
		return CheckMethod::CollisionCheckBoxVsSphere(
			pair.first->GetComponent<BoxCollider>(),
			pair.second->GetComponent<SphereCollider>(),
			info
		);
	};

	collisionCheckMap_[boxCompName + "Vs" + boxCompName] = [](const CollisionPair& pair, CollisionInfo* info) -> bool {
		return CheckMethod::CollisionCheckBoxVsBox(
			pair.first->GetComponent<BoxCollider>(),
			pair.second->GetComponent<BoxCollider>(),
			info
		);
	};

}


void CollisionSystem::RuntimeUpdate(ECSGroup* ecs) {
	CPUTimeStamp::GetInstance().BeginTimeStamp(CPUTimeStampID::PhysicsUpdate);

	enterPairs_.clear();
	stayPairs_.clear();
	exitPairs_.clear();

	/// 破棄済みエンティティを参照するペアを collidedPairs_ から除去する。
	/// collidedPairs_ は GameEntity* の生ポインタを保持しており、Entity.Destroy() で
	/// GameEntity が即時解放されるとダングリングになる。残したまま次の衝突パスへ進むと
	/// exitPairs_ 経由で CallExitFunc が解放済みメモリを参照し use-after-free で落ちる。
	/// 「生存中のエンティティ集合」に居ないペアだけを落とす（分離しただけの生存ペアは Exit を正しく発火させる）。
	{
		std::unordered_set<GameEntity*> liveEntities;
		for (const auto& e : ecs->GetEntities()) {
			liveEntities.insert(e.get());
		}
		collidedPairs_.erase(
			std::remove_if(collidedPairs_.begin(), collidedPairs_.end(),
				[&liveEntities](const CollisionPair& p) {
					return liveEntities.find(p.first) == liveEntities.end()
						|| liveEntities.find(p.second) == liveEntities.end();
				}),
			collidedPairs_.end());
	}

	/// 全てのコライダーを取得
	ComponentArray<SphereCollider>* sphereColliderArray = ecs->GetComponentArray<SphereCollider>();
	ComponentArray<BoxCollider>* boxColliderArray = ecs->GetComponentArray<BoxCollider>();

	/// コライダーの配列
	std::vector<ICollider*> colliders;

	/// sphere colliderを配列に格納する、インスタンスのnullチェックと有効フラグのチェックを行う
	if(sphereColliderArray) {
		for(auto& sphereCollider : sphereColliderArray->GetUsedComponents()) {
			if(sphereCollider && sphereCollider->enable) {
				colliders.push_back(sphereCollider);
			}
		}
	}

	/// box colliderを配列に格納する、インスタンスのnullチェックと有効フラグのチェックを行う
	if(boxColliderArray) {
		for(auto& boxCollider : boxColliderArray->GetUsedComponents()) {
			if(boxCollider && boxCollider->enable) {
				colliders.push_back(boxCollider);
			}
		}
	}

	/// 全てのColliderのprevPositionを更新する
	for(auto& collider : colliders) {
		collider->UpdatePrevPosition();
	}


	/// 衝突計算をしたフレームを記録するマップ
	using EntityIdPair = std::pair<int, int>;
	std::unordered_map<EntityIdPair, int> collisionFrameMap;

	/// 衝突判定
	std::string collisionType = "";
	for(auto& a : colliders) {
		for(auto& b : colliders) {
			/// 同じオブジェクト同士の衝突は無視
			if(a == b) {
				continue;
			}

			bool canCollide = (a->GetCategoryBits() & b->GetMaskBits()) != 0 &&
				(b->GetCategoryBits() & a->GetMaskBits()) != 0;

			/// 互いに衝突する設定でなければ、このペアの処理をスキップ
			if(!canCollide) {
				continue;
			}

			/// このフレームないで衝突計算をしているかチェック
			collisionType = typeid(*a).name() + std::string("Vs") + typeid(*b).name();
			EntityIdPair pairKey = std::make_pair(a->GetOwner()->GetId(), b->GetOwner()->GetId());

			/*
			* --- 衝突判定の流れ ---
			* 1, このフレームですでに衝突判定をとっているかチェック
			* 2, まだなら衝突判定を計算
			* 3, 衝突しているなら collisionPairs_ にペアを追加
			* 4, 衝突していないなら前 collisionPairs_ にペアがあれば削除し、releasePairs_にペアを追加
			* 5, call back関数の実行
			*/


			/// mapがペアを持っていないかどうか
			bool collisionFrameMapContains = false;
			if(!collisionFrameMap.contains(pairKey)) {
				/// 逆順でないかチェック
				pairKey = std::make_pair(b->GetOwner()->GetId(), a->GetOwner()->GetId());
				if(!collisionFrameMap.contains(pairKey)) {
					collisionFrameMapContains = true;
				}
			}

			/// このフレームで衝突判定をしている場合はスキップする
			if(!collisionFrameMapContains) {
				continue;
			}

			/// 衝突計算をしたフレームを記録
			++collisionFrameMap[pairKey];


			/// 衝突計算の関数を取得
			CollisionPair pair(a->GetOwner(), b->GetOwner());
			auto collisionCheckItr = collisionCheckMap_.find(collisionType);
			if(collisionCheckItr == collisionCheckMap_.end()) {
				continue;
			}

			/// 衝突計算を行う
			CollisionInfo info;
			bool isCollided = collisionCheckItr->second(pair, &info);
			if(isCollided) {

				/// 押し戻しを行う (どちらかがTriggerならスキップ)
				if (!a->IsTrigger() && !b->IsTrigger()) {
					PushBack(
						a->GetOwner(), a->GetCollisionState(),
						b->GetOwner(), b->GetCollisionState(),
						info
					);
				}


				/// collidedPairs_にペアがすでに存在しているかチェック
				auto collisionPairItr = std::find_if(collidedPairs_.begin(), collidedPairs_.end(), [&pair](const CollisionPair& p) {
					return (p.first == pair.first && p.second == pair.second)
						|| (p.first == pair.second && p.second == pair.first);
				});

				if(collisionPairItr != collidedPairs_.end()) {
					/// すでにペアが存在している場合は stayPairs_ に追加
					stayPairs_.emplace_back(pair);
				} else {
					/// 新たにペアが追加された場合は enterPairs_ に追加
					enterPairs_.emplace_back(pair);
					/// 新たに衝突した場合はペアを記録
					collidedPairs_.emplace_back(pair);
				}

			} else {

				/// collisionPairs_からペアを削除
				auto collisionPairItr = std::find_if(collidedPairs_.begin(), collidedPairs_.end(), [&pair](const CollisionPair& p) {
					return (p.first == pair.first && p.second == pair.second)
						|| (p.first == pair.second && p.second == pair.first);
				});

				/// 削除するペアがあった場合は exitPairs_ に追加
				if(collisionPairItr != collidedPairs_.end()) {
					exitPairs_.emplace_back(pair);
					collidedPairs_.erase(collisionPairItr);
				}
			}

		}
	}


	/// 各コールバック関数の実行
	const std::string& ecsGroupName = ecs->GetGroupName();
	CallEnterFunc(ecsGroupName);
	CallStayFunc(ecsGroupName);
	CallExitFunc(ecsGroupName);

	CPUTimeStamp::GetInstance().EndTimeStamp(CPUTimeStampID::PhysicsUpdate);
}

void CollisionSystem::CallEnterFunc(const std::string& ecsGroupName) {
	MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();
	ECSGroup* group = gECS->GetECSGroup(ecsGroupName);
	if (!group) {
		return;
	}

	std::vector<std::pair<int32_t, int32_t>> enterIds;
	enterIds.reserve(enterPairs_.size());
	for (auto& pair : enterPairs_) {
		if (pair.first && pair.second) {
			enterIds.push_back({ pair.first->GetId(), pair.second->GetId() });
		}
	}

	for(const auto& idPair : enterIds) {
		int32_t idA = idPair.first;
		int32_t idB = idPair.second;

		GameEntity* entityA = group->GetEntity(idA);
		GameEntity* entityB = group->GetEntity(idB);

		if(!entityA || !entityB) {
			continue;
		}

		std::array<int32_t, 2> entityIds = { idA, idB };

		for(size_t i = 0; i < 2; i++) {
			GameEntity* currentA = group->GetEntity(entityIds[0]);
			GameEntity* currentB = group->GetEntity(entityIds[1]);
			if (!currentA || !currentB) {
				break;
			}

			GameEntity* selfEntity = (i == 0) ? currentA : currentB;
			GameEntity* otherEntity = (i == 0) ? currentB : currentA;

			Script* scriptComponent = selfEntity->GetComponent<Script>();
			if(!scriptComponent) {
				continue;
			}

			auto& data = scriptComponent->GetScriptDataList();
			for(auto& script : data) {
				GameEntity* checkSelf = group->GetEntity(entityIds[i]);
				GameEntity* checkOther = group->GetEntity(entityIds[(i + 1) % 2]);
				if (!checkSelf || !checkOther) {
					break;
				}

				MonoObject* exc = nullptr;

				/// 引数の準備
				void* params[1];
				params[0] = monoEngine.GetEntityFromCS(ecsGroupName, checkOther->GetId());

				MonoObject* monoBehavior = monoEngine.GetMonoBehaviorFromCS(ecsGroupName, checkSelf->GetId(), script.scriptName);
				if(!script.collisionEventMethods[0]) {
					script.collisionEventMethods[0] = monoEngine.GetMethodFromCS("", script.scriptName, "OnCollisionEnter", 1);
				}

				mono_runtime_invoke(script.collisionEventMethods[0], monoBehavior, params, &exc);


				Console::Log("Collision Enter Event Invoked");

				/// 例外が発生した場合の処理
				if(exc) {
					MonoString* monoStr = mono_object_to_string(exc, nullptr);
					if(monoStr) {
						char* message = mono_string_to_utf8(monoStr);
						Console::Log(std::string("Mono Exception: ") + message);
						mono_free(message);
					} else {
						Console::Log("Mono Exception occurred, but message is null.");
					}
				}

			}
		}

	}
}

void CollisionSystem::CallStayFunc(const std::string& ecsGroupName) {
	MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();
	ECSGroup* group = gECS->GetECSGroup(ecsGroupName);
	if (!group) {
		return;
	}

	std::vector<std::pair<int32_t, int32_t>> stayIds;
	stayIds.reserve(stayPairs_.size());
	for (auto& pair : stayPairs_) {
		if (pair.first && pair.second) {
			stayIds.push_back({ pair.first->GetId(), pair.second->GetId() });
		}
	}

	for(const auto& idPair : stayIds) {
		int32_t idA = idPair.first;
		int32_t idB = idPair.second;

		GameEntity* entityA = group->GetEntity(idA);
		GameEntity* entityB = group->GetEntity(idB);

		if(!entityA || !entityB) {
			continue;
		}

		std::array<int32_t, 2> entityIds = { idA, idB };

		for(size_t i = 0; i < 2; i++) {
			GameEntity* currentA = group->GetEntity(entityIds[0]);
			GameEntity* currentB = group->GetEntity(entityIds[1]);
			if (!currentA || !currentB) {
				break;
			}

			GameEntity* selfEntity = (i == 0) ? currentA : currentB;
			GameEntity* otherEntity = (i == 0) ? currentB : currentA;

			Script* scriptComponent = selfEntity->GetComponent<Script>();
			if(!scriptComponent) {
				continue;
			}

			auto& data = scriptComponent->GetScriptDataList();
			for(auto& script : data) {
				GameEntity* checkSelf = group->GetEntity(entityIds[i]);
				GameEntity* checkOther = group->GetEntity(entityIds[(i + 1) % 2]);
				if (!checkSelf || !checkOther) {
					break;
				}

				MonoObject* exc = nullptr;

				/// 引数の準備
				void* params[1];
				params[0] = monoEngine.GetEntityFromCS(ecsGroupName, checkOther->GetId());

				MonoObject* monoBehavior = monoEngine.GetMonoBehaviorFromCS(ecsGroupName, checkSelf->GetId(), script.scriptName);
				if(!script.collisionEventMethods[1]) {
					script.collisionEventMethods[1] = monoEngine.GetMethodFromCS("", script.scriptName, "OnCollisionStay", 1);
				}

				mono_runtime_invoke(script.collisionEventMethods[1], monoBehavior, params, &exc);

				// Console::Log("Collision Stay Event Invoked");

				/// 例外が発生した場合の処理
				if(exc) {
					MonoString* monoStr = mono_object_to_string(exc, nullptr);
					if(monoStr) {
						char* message = mono_string_to_utf8(monoStr);
						Console::Log(std::string("Mono Exception: ") + message);
						mono_free(message);
					} else {
						Console::Log("Mono Exception occurred, but message is null.");
					}
				}

			}
		}

	}
}

void CollisionSystem::CallExitFunc(const std::string& ecsGroupName) {
	MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();
	ECSGroup* group = gECS->GetECSGroup(ecsGroupName);
	if (!group) {
		return;
	}

	std::vector<std::pair<int32_t, int32_t>> exitIds;
	exitIds.reserve(exitPairs_.size());
	for (auto& pair : exitPairs_) {
		if (pair.first && pair.second) {
			exitIds.push_back({ pair.first->GetId(), pair.second->GetId() });
		}
	}

	for(const auto& idPair : exitIds) {
		int32_t idA = idPair.first;
		int32_t idB = idPair.second;

		GameEntity* entityA = group->GetEntity(idA);
		GameEntity* entityB = group->GetEntity(idB);

		if(!entityA || !entityB) {
			continue;
		}

		std::array<int32_t, 2> entityIds = { idA, idB };

		for(size_t i = 0; i < 2; i++) {
			GameEntity* currentA = group->GetEntity(entityIds[0]);
			GameEntity* currentB = group->GetEntity(entityIds[1]);
			if (!currentA || !currentB) {
				break;
			}

			GameEntity* selfEntity = (i == 0) ? currentA : currentB;
			GameEntity* otherEntity = (i == 0) ? currentB : currentA;

			Script* scriptComponent = selfEntity->GetComponent<Script>();
			if(!scriptComponent) {
				continue;
			}

			auto& data = scriptComponent->GetScriptDataList();
			for(auto& script : data) {
				GameEntity* checkSelf = group->GetEntity(entityIds[i]);
				GameEntity* checkOther = group->GetEntity(entityIds[(i + 1) % 2]);
				if (!checkSelf || !checkOther) {
					break;
				}

				MonoObject* exc = nullptr;

				/// 引数の準備
				void* params[1];
				params[0] = monoEngine.GetEntityFromCS(ecsGroupName, checkOther->GetId());


				MonoObject* monoBehavior = monoEngine.GetMonoBehaviorFromCS(ecsGroupName, checkSelf->GetId(), script.scriptName);
				if(!script.collisionEventMethods[2]) {
					script.collisionEventMethods[2] = monoEngine.GetMethodFromCS("", script.scriptName, "OnCollisionExit", 1);
				}

				mono_runtime_invoke(script.collisionEventMethods[2], monoBehavior, params, &exc);


				Console::Log("Collision Exit Event Invoked");

				/// 例外が発生した場合の処理
				if(exc) {
					MonoString* monoStr = mono_object_to_string(exc, nullptr);
					if(monoStr) {
						char* message = mono_string_to_utf8(monoStr);
						Console::Log(std::string("Mono Exception: ") + message);
						mono_free(message);
					} else {
						Console::Log("Mono Exception occurred, but message is null.");
					}
				}

			}
		}

	}
}

void CollisionSystem::PushBack(GameEntity* a, CollisionState aState, GameEntity* b, CollisionState bState, const CollisionInfo& info) {
	if(!a || !b) {
		return;
	}

	// Colliderを取得
	ICollider* aCol = a->GetComponent<SphereCollider>();
	if (!aCol) aCol = a->GetComponent<BoxCollider>();
	ICollider* bCol = b->GetComponent<SphereCollider>();
	if (!bCol) bCol = b->GetComponent<BoxCollider>();

	// Dynamic / Static フラグ
	bool aDynamic = aState == CollisionState::Dynamic;
	bool bDynamic = bState == CollisionState::Dynamic;

	// 押し戻しベクトル
	Vector3 correction = info.normal * info.penetration;

	if(aDynamic && !bDynamic) {
		// aだけ押し戻す
		Vector3 pos = a->GetPosition() - correction;
		if (aCol && aCol->IsFreezeY()) pos.y = a->GetPosition().y;
		a->SetPosition(pos);
	} else if(!aDynamic && bDynamic) {
		// bだけ押し戻す
		Vector3 pos = b->GetPosition() + correction;
		if (bCol && bCol->IsFreezeY()) pos.y = b->GetPosition().y;
		b->SetPosition(pos);
	} else if(aDynamic && bDynamic) {
		// 両方Dynamicなら重量比で押し戻す
		float aMass = aCol ? aCol->GetMass() : 1.0f;
		float bMass = bCol ? bCol->GetMass() : 1.0f;
		float totalMass = aMass + bMass;

		// 重い方ほど動かない（逆比を掛ける）
		float aRatio = bMass / totalMass;
		float bRatio = aMass / totalMass;

		Vector3 posA = a->GetPosition() - correction * aRatio;
		if (aCol && aCol->IsFreezeY()) posA.y = a->GetPosition().y;
		a->SetPosition(posA);

		Vector3 posB = b->GetPosition() + correction * bRatio;
		if (bCol && bCol->IsFreezeY()) posB.y = b->GetPosition().y;
		b->SetPosition(posB);
	}
}


bool CheckMethod::CollisionCheckSphereVsSphere(SphereCollider* s1, SphereCollider* s2, CollisionInfo* info) {
	if(!s1 || !s2) {
		return false; // 型が一致しない場合は衝突なし
	}

	GameEntity* e1 = s1->GetOwner();
	GameEntity* e2 = s2->GetOwner();

	float r1 = s1->GetRadius();
	if (s1->IsUseOwnerScale()) {
		Transform* t1 = e1->GetTransform();
		r1 *= (std::max)({ t1->scale.x, t1->scale.y, t1->scale.z });
	}

	float r2 = s2->GetRadius();
	if (s2->IsUseOwnerScale()) {
		Transform* t2 = e2->GetTransform();
		r2 *= (std::max)({ t2->scale.x, t2->scale.y, t2->scale.z });
	}

	float distance = (e1->GetPosition() - e2->GetPosition()).Length();

	/// 衝突情報の設定
	if(info) {
		/// 法線はe1からe2への方向
		info->normal = Vector3::Normalize(e2->GetPosition() - e1->GetPosition());
		info->penetration = (r1 + r2) - distance;
	}


	return distance <= (r1 + r2);
}

bool CheckMethod::CollisionCheckSphereVsBox(SphereCollider* s, BoxCollider* b, CollisionInfo* info) {
	if(!s || !b) {
		return false; // 型が一致しない場合は衝突なし
	}

	return CollisionCheckBoxVsSphere(b, s, info);
}

bool CheckMethod::CollisionCheckBoxVsSphere(BoxCollider* b, SphereCollider* s, CollisionInfo* info) {
	if(!b || !s) {
		return false;
	}

	GameEntity* boxEntity = b->GetOwner();
	GameEntity* sphereEntity = s->GetOwner();

	// 球のワールド座標
	Vector3 spherePos = sphereEntity->GetPosition();

	Transform* bTrans = boxEntity->GetTransform();

	// ★修正1: 変換行列には Scale や Size を含めない！（距離のスケールをワールドと一致させるため）
	// エンジンの乗算順序（行優先か列優先か）に合わせて Rotate と Translate を掛けてください
	Matrix4x4 matOBBTransform = Matrix4x4::MakeRotate(bTrans->GetRotate()) * Matrix4x4::MakeTranslate(bTrans->position);
	Matrix4x4 matOBBTransformInverse = matOBBTransform.Inverse();
	
	// 球の座標をスケール無しのローカル空間へ
	Vector3 localSpherePos = Matrix4x4::Transform(spherePos, matOBBTransformInverse);

	// ★修正2: 箱のサイズとTransformのスケールは、ここで計算する
	Vector3 boxWorldSize = b->GetSize();
	if (b->IsUseOwnerScale()) {
		boxWorldSize.x *= bTrans->scale.x;
		boxWorldSize.y *= bTrans->scale.y;
		boxWorldSize.z *= bTrans->scale.z;
	}
	Vector3 halfExtents = boxWorldSize * 0.5f;
	
	Vector3 localMin = -halfExtents;
	Vector3 localMax = halfExtents;

	// 3. ローカル空間での最近接点 (Closest Point) をクランプで求める
	Vector3 localClosestPoint(
		std::clamp(localSpherePos.x, localMin.x, localMax.x),
		std::clamp(localSpherePos.y, localMin.y, localMax.y),
		std::clamp(localSpherePos.z, localMin.z, localMax.z)
	);

	// 4. 最近接点と球の中心の距離をチェック
	Vector3 localDelta = localSpherePos - localClosestPoint;
	float distanceSquared = localDelta.LengthSquared();

	float radius = s->GetRadius();
	if (s->IsUseOwnerScale()) {
		Transform* sTrans = sphereEntity->GetTransform();
		radius *= (std::max)({ sTrans->scale.x, sTrans->scale.y, sTrans->scale.z });
	}

	if(distanceSquared > radius * radius) {
		return false; // 衝突していない
	}

	// === ここから衝突時の情報 (CollisionInfo) の計算 ===
	if(info) {
		float distance = std::sqrt(distanceSquared);

		// 球の中心が箱の「外」にあり、浅く接触している場合
		if(distance > 0.0001f) {
			info->penetration = radius - distance;

			// 最近接点をワールド空間に戻す（ここは座標なので通常のTransformでOK）
			Vector3 worldClosestPoint = Matrix4x4::Transform(localClosestPoint, matOBBTransform);

			info->normal = -Vector3::Normalize(spherePos - worldClosestPoint);
			info->contactPoint = worldClosestPoint;
		}
		// 球の中心が箱の「完全に中」に入ってしまっている場合 (distance == 0)
		else {
			float distX = halfExtents.x - std::abs(localSpherePos.x);
			float distY = halfExtents.y - std::abs(localSpherePos.y);
			float distZ = halfExtents.z - std::abs(localSpherePos.z);

			float minDist = (std::min)({ distX, distY, distZ });
			Vector3 localNormal = Vector3::Zero;

			if(minDist == distX) {
				localNormal = localSpherePos.x > 0 ? Vector3::Right : Vector3::Left;
			} else if(minDist == distY) {
				localNormal = localSpherePos.y > 0 ? Vector3::Up : Vector3::Down;
			} else {
				localNormal = localSpherePos.z > 0 ? Vector3::Forward : Vector3::Back;
			}

			info->penetration = radius + minDist;

			// ★修正3: 法線には平行移動を適用してはいけない！
			// もし Matrix4x4::TransformNormal がまだエンジン内に無い場合は、
			// 回転行列のみを取り出して適用します。
			Matrix4x4 rotationOnlyMat = Matrix4x4::MakeRotate(bTrans->GetRotate());
			info->normal = Vector3::Normalize(Matrix4x4::Transform(localNormal, rotationOnlyMat));
			
			info->contactPoint = spherePos;
		}
	}

	return true;
}

bool CheckMethod::CollisionCheckBoxVsBox(BoxCollider* b1, BoxCollider* b2, CollisionInfo* info) {
	if(!b1 || !b2) {
		return false;
	}
	GameEntity* e1 = b1->GetOwner();
	GameEntity* e2 = b2->GetOwner();

	Transform* t1 = e1->GetTransform();
	Transform* t2 = e2->GetTransform();

	// OBBパラメータの準備
	Vector3 center1 = e1->GetPosition();
	Vector3 size1 = b1->GetSize();
	if (b1->IsUseOwnerScale()) {
		size1.x *= t1->scale.x;
		size1.y *= t1->scale.y;
		size1.z *= t1->scale.z;
	}
	Vector3 half1 = size1 * 0.5f;
	Matrix4x4 rot1 = Matrix4x4::MakeRotate(t1->GetRotate());
	Vector3 axis1[3] = {
		Vector3::Normalize(Matrix4x4::Transform(Vector3::Right, rot1)),
		Vector3::Normalize(Matrix4x4::Transform(Vector3::Up, rot1)),
		Vector3::Normalize(Matrix4x4::Transform(Vector3::Forward, rot1))
	};

	Vector3 center2 = e2->GetPosition();
	Vector3 size2 = b2->GetSize();
	if (b2->IsUseOwnerScale()) {
		size2.x *= t2->scale.x;
		size2.y *= t2->scale.y;
		size2.z *= t2->scale.z;
	}
	Vector3 half2 = size2 * 0.5f;
	Matrix4x4 rot2 = Matrix4x4::MakeRotate(t2->GetRotate());
	Vector3 axis2[3] = {
		Vector3::Normalize(Matrix4x4::Transform(Vector3::Right, rot2)),
		Vector3::Normalize(Matrix4x4::Transform(Vector3::Up, rot2)),
		Vector3::Normalize(Matrix4x4::Transform(Vector3::Forward, rot2))
	};

	// 分離軸定理 (SAT) による判定
	Vector3 L = center2 - center1;

	// 各軸への投影半径の和と中心距離の比較
	float minPenetration = FLT_MAX;
	Vector3 bestNormal;

	auto checkAxis = [&](const Vector3& axis) -> bool {
		if (axis.LengthSquared() < 0.0001f) return true; // 軸が潰れている場合はスキップ
		Vector3 unitAxis = Vector3::Normalize(axis);
		
		float r1 = std::abs(Vector3::Dot(axis1[0] * half1.x, unitAxis)) +
		           std::abs(Vector3::Dot(axis1[1] * half1.y, unitAxis)) +
		           std::abs(Vector3::Dot(axis1[2] * half1.z, unitAxis));
		
		float r2 = std::abs(Vector3::Dot(axis2[0] * half2.x, unitAxis)) +
		           std::abs(Vector3::Dot(axis2[1] * half2.y, unitAxis)) +
		           std::abs(Vector3::Dot(axis2[2] * half2.z, unitAxis));
		
		float distance = std::abs(Vector3::Dot(L, unitAxis));
		float penetration = (r1 + r2) - distance;

		if (penetration <= 0.0f) return false; // 分離軸発見、衝突していない

		if (penetration < minPenetration) {
			minPenetration = penetration;
			bestNormal = (Vector3::Dot(L, unitAxis) > 0) ? unitAxis : -unitAxis;
		}
		return true;
	};

	// 15本の候補軸をチェック
	for (int i = 0; i < 3; i++) if (!checkAxis(axis1[i])) return false;
	for (int i = 0; i < 3; i++) if (!checkAxis(axis2[i])) return false;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (!checkAxis(Vector3::Cross(axis1[i], axis2[j]))) return false;
		}
	}

	if (info) {
		info->normal = bestNormal;
		info->penetration = minPenetration;
		info->contactPoint = center1 + (bestNormal * (minPenetration * 0.5f)); // 簡易的な接触点
	}

	return true;
}

