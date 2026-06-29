#include "Collision2DSystem.h"

using namespace ONEngine;

/// std
#include <unordered_map>
#include <utility>
#include <algorithm>

/// engine
#include "Engine/Core/Utility/Utility.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/Script/MonoScriptEngine.h"
#include "Engine/Core/Utility/Time/CPUTimeStamp.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider2D.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/CircleCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/CollisionCheck/CollisionCheck.h"
#include "Engine/ECS/System/Collision/CollisionSystem.h" // 衝突情報を流用するため

namespace std {
template <>
struct hash<std::pair<int, int>> {
	std::size_t operator()(const std::pair<int, int>& p) const {
		return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
	}
};
}

Collision2DSystem::Collision2DSystem() {
	std::string circleCompName = typeid(CircleCollider).name();
	std::string box2DCompName = typeid(BoxCollider2D).name();

	/// 関数の登録をする
	collisionCheckMap_[circleCompName + "Vs" + circleCompName] = [](const CollisionPair& pair, CollisionInfo* info) -> bool {
		return CheckMethod2D::CollisionCheckCircleVsCircle(
			pair.first->GetComponent<CircleCollider>(),
			pair.second->GetComponent<CircleCollider>(),
			info
		);
	};

	collisionCheckMap_[circleCompName + "Vs" + box2DCompName] = [](const CollisionPair& pair, CollisionInfo* info) -> bool {
		return CheckMethod2D::CollisionCheckCircleVsBox2D(
			pair.first->GetComponent<CircleCollider>(),
			pair.second->GetComponent<BoxCollider2D>(),
			info
		);
	};

	collisionCheckMap_[box2DCompName + "Vs" + circleCompName] = [](const CollisionPair& pair, CollisionInfo* info) -> bool {
		return CheckMethod2D::CollisionCheckBox2DVsCircle(
			pair.first->GetComponent<BoxCollider2D>(),
			pair.second->GetComponent<CircleCollider>(),
			info
		);
	};

	collisionCheckMap_[box2DCompName + "Vs" + box2DCompName] = [](const CollisionPair& pair, CollisionInfo* info) -> bool {
		return CheckMethod2D::CollisionCheckBox2DVsBox2D(
			pair.first->GetComponent<BoxCollider2D>(),
			pair.second->GetComponent<BoxCollider2D>(),
			info
		);
	};
}

void Collision2DSystem::RuntimeUpdate(ECSGroup* ecs) {
	CPUTimeStamp::GetInstance().BeginTimeStamp(CPUTimeStampID::PhysicsUpdate);

	enterPairs_.clear();
	stayPairs_.clear();
	exitPairs_.clear();

	/// 全てのコライダーを取得
	ComponentArray<CircleCollider>* circleColliderArray = ecs->GetComponentArray<CircleCollider>();
	ComponentArray<BoxCollider2D>* boxCollider2DArray = ecs->GetComponentArray<BoxCollider2D>();

	/// コライダーの配列
	std::vector<ICollider*> colliders;

	if(circleColliderArray) {
		for(auto& circleCollider : circleColliderArray->GetUsedComponents()) {
			if(circleCollider && circleCollider->enable) {
				colliders.push_back(circleCollider);
			}
		}
	}

	if(boxCollider2DArray) {
		for(auto& boxCollider2D : boxCollider2DArray->GetUsedComponents()) {
			if(boxCollider2D && boxCollider2D->enable) {
				colliders.push_back(boxCollider2D);
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

			/// このフレーム内で衝突計算をしているかチェック
			collisionType = typeid(*a).name() + std::string("Vs") + typeid(*b).name();
			EntityIdPair pairKey = std::make_pair(a->GetOwner()->GetId(), b->GetOwner()->GetId());

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

void Collision2DSystem::CallEnterFunc(const std::string& ecsGroupName) {
	MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();

	for(auto& pair : enterPairs_) {
		GameEntity* entityA = pair.first;
		GameEntity* entityB = pair.second;

		if(!entityA || !entityB) {
			continue;
		}

		std::array<GameEntity*, 2> entities = { entityA, entityB };
		std::array<Script*, 2>     scripts = { entityA->GetComponent<Script>(), entityB->GetComponent<Script>() };

		for(size_t i = 0; i < 2; i++) {
			if(!scripts[i]) {
				continue;
			}

			auto& data = scripts[i]->GetScriptDataList();
			for(auto& script : data) {
				MonoObject* exc = nullptr;

				if(!script.collisionEventMethods2D[0]) {
					script.collisionEventMethods2D[0] = monoEngine.GetMethodFromCS("", script.scriptName, "OnCollisionEnter", 1);
				}

				if(!script.collisionEventMethods2D[0]) {
					continue; // メソッドが定義されていない場合はスキップ
				}

				void* params[1];
				params[0] = monoEngine.GetEntityFromCS(ecsGroupName, entities[(i + 1) % 2]->GetId());

				MonoObject* monoBehavior = monoEngine.GetMonoBehaviorFromCS(ecsGroupName, scripts[i]->GetOwner()->GetId(), script.scriptName);
				mono_runtime_invoke(script.collisionEventMethods2D[0], monoBehavior, params, &exc);

				Console::Log("Collision Enter 2D Event Invoked");

				if(exc) {
					MonoString* monoStr = mono_object_to_string(exc, nullptr);
					if(monoStr) {
						char* message = mono_string_to_utf8(monoStr);
						Console::Log(std::string("Mono Exception: ") + message);
						mono_free(message);
					}
				}
			}
		}
	}
}

void Collision2DSystem::CallStayFunc(const std::string& ecsGroupName) {
	MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();

	for(auto& pair : stayPairs_) {
		GameEntity* entityA = pair.first;
		GameEntity* entityB = pair.second;

		if(!entityA || !entityB) {
			continue;
		}

		std::array<GameEntity*, 2> entities = { entityA, entityB };
		std::array<Script*, 2>     scripts = { entityA->GetComponent<Script>(), entityB->GetComponent<Script>() };

		for(size_t i = 0; i < 2; i++) {
			if(!scripts[i]) {
				continue;
			}

			auto& data = scripts[i]->GetScriptDataList();
			for(auto& script : data) {
				MonoObject* exc = nullptr;

				if(!script.collisionEventMethods2D[1]) {
					script.collisionEventMethods2D[1] = monoEngine.GetMethodFromCS("", script.scriptName, "OnCollisionStay", 1);
				}

				if(!script.collisionEventMethods2D[1]) {
					continue;
				}

				void* params[1];
				params[0] = monoEngine.GetEntityFromCS(ecsGroupName, entities[(i + 1) % 2]->GetId());

				MonoObject* monoBehavior = monoEngine.GetMonoBehaviorFromCS(ecsGroupName, scripts[i]->GetOwner()->GetId(), script.scriptName);
				mono_runtime_invoke(script.collisionEventMethods2D[1], monoBehavior, params, &exc);

				if(exc) {
					MonoString* monoStr = mono_object_to_string(exc, nullptr);
					if(monoStr) {
						char* message = mono_string_to_utf8(monoStr);
						Console::Log(std::string("Mono Exception: ") + message);
						mono_free(message);
					}
				}
			}
		}
	}
}

void Collision2DSystem::CallExitFunc(const std::string& ecsGroupName) {
	MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();

	for(auto& pair : exitPairs_) {
		GameEntity* entityA = pair.first;
		GameEntity* entityB = pair.second;

		if(!entityA || !entityB) {
			continue;
		}

		std::array<GameEntity*, 2> entities = { entityA, entityB };
		std::array<Script*, 2>     scripts = { entityA->GetComponent<Script>(), entityB->GetComponent<Script>() };

		for(size_t i = 0; i < 2; i++) {
			if(!scripts[i]) {
				continue;
			}

			auto& data = scripts[i]->GetScriptDataList();
			for(auto& script : data) {
				MonoObject* exc = nullptr;

				if(!script.collisionEventMethods2D[2]) {
					script.collisionEventMethods2D[2] = monoEngine.GetMethodFromCS("", script.scriptName, "OnCollisionExit", 1);
				}

				if(!script.collisionEventMethods2D[2]) {
					continue;
				}

				void* params[1];
				params[0] = monoEngine.GetEntityFromCS(ecsGroupName, entities[(i + 1) % 2]->GetId());

				MonoObject* monoBehavior = monoEngine.GetMonoBehaviorFromCS(ecsGroupName, scripts[i]->GetOwner()->GetId(), script.scriptName);
				mono_runtime_invoke(script.collisionEventMethods2D[2], monoBehavior, params, &exc);

				Console::Log("Collision Exit 2D Event Invoked");

				if(exc) {
					MonoString* monoStr = mono_object_to_string(exc, nullptr);
					if(monoStr) {
						char* message = mono_string_to_utf8(monoStr);
						Console::Log(std::string("Mono Exception: ") + message);
						mono_free(message);
					}
				}
			}
		}
	}
}

void Collision2DSystem::PushBack(GameEntity* a, CollisionState aState, GameEntity* b, CollisionState bState, const CollisionInfo& info) {
	if(!a || !b) {
		return;
	}

	ICollider* aCol = a->GetComponent<CircleCollider>();
	if (!aCol) aCol = a->GetComponent<BoxCollider2D>();
	ICollider* bCol = b->GetComponent<CircleCollider>();
	if (!bCol) bCol = b->GetComponent<BoxCollider2D>();

	bool aDynamic = aState == CollisionState::Dynamic;
	bool bDynamic = bState == CollisionState::Dynamic;

	Vector3 correction = info.normal * info.penetration;

	if(aDynamic && !bDynamic) {
		Vector3 pos = a->GetPosition() - correction;
		if (aCol && aCol->IsFreezeY()) pos.y = a->GetPosition().y;
		a->SetPosition(pos);
	} else if(!aDynamic && bDynamic) {
		Vector3 pos = b->GetPosition() + correction;
		if (bCol && bCol->IsFreezeY()) pos.y = b->GetPosition().y;
		b->SetPosition(pos);
	} else if(aDynamic && bDynamic) {
		float aMass = aCol ? aCol->GetMass() : 1.0f;
		float bMass = bCol ? bCol->GetMass() : 1.0f;
		float totalMass = aMass + bMass;

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


bool CheckMethod2D::CollisionCheckCircleVsCircle(CircleCollider* c1, CircleCollider* c2, CollisionInfo* info) {
	if(!c1 || !c2) {
		return false;
	}

	GameEntity* e1 = c1->GetOwner();
	GameEntity* e2 = c2->GetOwner();

	Vector2 p1(e1->GetPosition().x, e1->GetPosition().y);
	Vector2 p2(e2->GetPosition().x, e2->GetPosition().y);

	float r1 = c1->GetRadius();
	if(c1->IsUseOwnerScale()) {
		Vector3 scale1 = e1->GetScale();
		r1 *= (std::max)(scale1.x, scale1.y);
	}

	float r2 = c2->GetRadius();
	if(c2->IsUseOwnerScale()) {
		Vector3 scale2 = e2->GetScale();
		r2 *= (std::max)(scale2.x, scale2.y);
	}

	float distance = (p1 - p2).Length();
	float sumRadius = r1 + r2;

	if(distance > sumRadius) {
		return false;
	}

	if(info) {
		Vector2 dir = p2 - p1;
		if(dir.LengthSquared() > 0.0001f) {
			dir = Vector2::Normalize(dir);
		} else {
			dir = Vector2(1.0f, 0.0f);
		}
		info->normal = Vector3(dir.x, dir.y, 0.0f);
		info->penetration = sumRadius - distance;
		info->contactPoint = Vector3(p1.x + dir.x * r1, p1.y + dir.y * r1, 0.0f);
	}

	return true;
}

bool CheckMethod2D::CollisionCheckCircleVsBox2D(CircleCollider* c, BoxCollider2D* b, CollisionInfo* info) {
	if(!c || !b) {
		return false;
	}
	bool collided = CollisionCheckBox2DVsCircle(b, c, info);
	if(collided && info) {
		info->normal = -info->normal;
	}
	return collided;
}

bool CheckMethod2D::CollisionCheckBox2DVsCircle(BoxCollider2D* b, CircleCollider* s, CollisionInfo* info) {
	if(!b || !s) {
		return false;
	}

	GameEntity* boxEntity = b->GetOwner();
	GameEntity* circleEntity = s->GetOwner();

	Vector2 boxPos(boxEntity->GetPosition().x, boxEntity->GetPosition().y);
	Vector2 circlePos(circleEntity->GetPosition().x, circleEntity->GetPosition().y);

	float radius = s->GetRadius();
	if(s->IsUseOwnerScale()) {
		Vector3 circleScale = circleEntity->GetScale();
		radius *= (std::max)(circleScale.x, circleScale.y);
	}

	Matrix4x4 rot = Matrix4x4::MakeRotate(boxEntity->GetRotateQuaternion());
	Vector3 axisX3D = Vector3::Normalize(Matrix4x4::Transform(Vector3::Right, rot));
	Vector3 axisY3D = Vector3::Normalize(Matrix4x4::Transform(Vector3::Up, rot));
	Vector2 axisX(axisX3D.x, axisX3D.y);
	Vector2 axisY(axisY3D.x, axisY3D.y);

	Vector2 d = circlePos - boxPos;
	Vector2 localCirclePos(Vector2::Dot(d, axisX), Vector2::Dot(d, axisY));

	Vector2 size = b->GetSize();
	if(b->IsUseOwnerScale()) {
		Vector3 boxScale = boxEntity->GetScale();
		size.x *= boxScale.x;
		size.y *= boxScale.y;
	}
	Vector2 half = size * 0.5f;

	Vector2 localClosestPoint(
		(std::clamp)(localCirclePos.x, -half.x, half.x),
		(std::clamp)(localCirclePos.y, -half.y, half.y)
	);

	Vector2 localDelta = localCirclePos - localClosestPoint;
	float distSq = localDelta.LengthSquared();

	if(distSq > radius * radius) {
		return false;
	}

	if(info) {
		float distance = std::sqrt(distSq);
		if(distance > 0.0001f) {
			info->penetration = radius - distance;
			Vector2 worldClosestPoint = boxPos + axisX * localClosestPoint.x + axisY * localClosestPoint.y;
			Vector2 normal = -Vector2::Normalize(circlePos - worldClosestPoint);
			info->normal = Vector3(normal.x, normal.y, 0.0f);
			info->contactPoint = Vector3(worldClosestPoint.x, worldClosestPoint.y, 0.0f);
		} else {
			float distX = half.x - std::abs(localCirclePos.x);
			float distY = half.y - std::abs(localCirclePos.y);

			Vector2 localNormal;
			float minDist;
			if(distX < distY) {
				minDist = distX;
				localNormal = localCirclePos.x > 0 ? Vector2(1, 0) : Vector2(-1, 0);
			} else {
				minDist = distY;
				localNormal = localCirclePos.y > 0 ? Vector2(0, 1) : Vector2(0, -1);
			}
			info->penetration = radius + minDist;
			Vector2 normal = axisX * localNormal.x + axisY * localNormal.y;
			info->normal = Vector3(normal.x, normal.y, 0.0f);
			info->contactPoint = Vector3(circlePos.x, circlePos.y, 0.0f);
		}
	}

	return true;
}

bool CheckMethod2D::CollisionCheckBox2DVsBox2D(BoxCollider2D* b1, BoxCollider2D* b2, CollisionInfo* info) {
	if(!b1 || !b2) {
		return false;
	}
	GameEntity* e1 = b1->GetOwner();
	GameEntity* e2 = b2->GetOwner();

	Vector2 center1(e1->GetPosition().x, e1->GetPosition().y);
	Vector2 center2(e2->GetPosition().x, e2->GetPosition().y);

	Vector2 size1 = b1->GetSize();
	if(b1->IsUseOwnerScale()) {
		Vector3 scale1 = e1->GetScale();
		size1.x *= scale1.x;
		size1.y *= scale1.y;
	}
	Vector2 half1 = size1 * 0.5f;

	Vector2 size2 = b2->GetSize();
	if(b2->IsUseOwnerScale()) {
		Vector3 scale2 = e2->GetScale();
		size2.x *= scale2.x;
		size2.y *= scale2.y;
	}
	Vector2 half2 = size2 * 0.5f;

	Matrix4x4 rot1 = Matrix4x4::MakeRotate(Quaternion::Normalize(e1->GetRotateQuaternion()));
	Vector3 axis1X_3D = Vector3::Normalize(Matrix4x4::Transform(Vector3::Right, rot1));
	Vector3 axis1Y_3D = Vector3::Normalize(Matrix4x4::Transform(Vector3::Up, rot1));
	Vector2 axis1[2] = { Vector2(axis1X_3D.x, axis1X_3D.y), Vector2(axis1Y_3D.x, axis1Y_3D.y) };

	Matrix4x4 rot2 = Matrix4x4::MakeRotate(Quaternion::Normalize(e2->GetRotateQuaternion()));
	Vector3 axis2X_3D = Vector3::Normalize(Matrix4x4::Transform(Vector3::Right, rot2));
	Vector3 axis2Y_3D = Vector3::Normalize(Matrix4x4::Transform(Vector3::Up, rot2));
	Vector2 axis2[2] = { Vector2(axis2X_3D.x, axis2X_3D.y), Vector2(axis2Y_3D.x, axis2Y_3D.y) };

	Vector2 L = center2 - center1;

	float minPenetration = FLT_MAX;
	Vector2 bestNormal;

	auto checkAxis = [&](const Vector2& axis) -> bool {
		if(axis.LengthSquared() < 0.0001f) return true;
		Vector2 unitAxis = Vector2::Normalize(axis);

		float r1 = std::abs(Vector2::Dot(axis1[0] * half1.x, unitAxis)) +
				   std::abs(Vector2::Dot(axis1[1] * half1.y, unitAxis));

		float r2 = std::abs(Vector2::Dot(axis2[0] * half2.x, unitAxis)) +
				   std::abs(Vector2::Dot(axis2[1] * half2.y, unitAxis));

		float distance = std::abs(Vector2::Dot(L, unitAxis));
		float penetration = (r1 + r2) - distance;

		if (penetration <= 0.0f) return false;

		if (penetration < minPenetration) {
			minPenetration = penetration;
			bestNormal = (Vector2::Dot(L, unitAxis) > 0) ? unitAxis : -unitAxis;
		}
		return true;
	};

	if (!checkAxis(axis1[0])) return false;
	if (!checkAxis(axis1[1])) return false;
	if (!checkAxis(axis2[0])) return false;
	if (!checkAxis(axis2[1])) return false;

	if (info) {
		info->normal = Vector3(bestNormal.x, bestNormal.y, 0.0f);
		info->penetration = minPenetration;
		Vector2 contact = center1 + bestNormal * (minPenetration * 0.5f);
		info->contactPoint = Vector3(contact.x, contact.y, 0.0f);
	}

	return true;
}
