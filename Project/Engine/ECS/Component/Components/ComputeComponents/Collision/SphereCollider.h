#pragma once

/// engine
#include "ICollider.h"


namespace ONEngine {

class SphereCollider;

namespace ComponentDebug {
/// @brief SphereColliderのデバッグ表示
/// @param collider SphereColliderのポインタ
void SphereColliderDebug(SphereCollider* collider);
}

/// //////////////////////////////////////
/// SphereCollider
/// //////////////////////////////////////
class SphereCollider : public ICollider {
	friend void ComponentDebug::SphereColliderDebug(SphereCollider* collider);
	friend void from_json(const nlohmann::json& j, SphereCollider& c);
	friend void to_json(nlohmann::json& j, const SphereCollider& c);
public:
	/// ====================================================
	/// public : methods
	/// ====================================================

	SphereCollider();
	~SphereCollider() override = default;

private:
	/// =====================================================
	/// private : objects
	/// =====================================================

	float radius_;

public:
	/// =====================================================
	/// public : accessors
	/// =====================================================

	void SetRadius(float radius);
	float GetRadius() const;

};

/// @brief Mono Internal Calls
float InternalGetRadius(uint64_t nativeHandle);
void InternalSetRadius(uint64_t nativeHandle, float radius);
bool InternalIsTriggerSphere(uint64_t nativeHandle);
void InternalSetTriggerSphere(uint64_t nativeHandle, bool trigger);
float InternalGetMass(uint64_t nativeHandle);
void InternalSetMass(uint64_t nativeHandle, float mass);

} /// ONEngine
