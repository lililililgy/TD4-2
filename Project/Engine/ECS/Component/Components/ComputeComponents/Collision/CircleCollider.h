#pragma once

/// engine
#include "ICollider.h"

namespace ONEngine {

class CircleCollider;

namespace ComponentDebug {
/// @brief CircleColliderのデバッグ表示
/// @param collider CircleColliderのポインタ
void CircleColliderDebug(CircleCollider* collider);
}

/// //////////////////////////////////////
/// CircleCollider
/// //////////////////////////////////////
class CircleCollider : public ICollider {
	friend void ComponentDebug::CircleColliderDebug(CircleCollider* collider);
	friend void from_json(const nlohmann::json& j, CircleCollider& c);
	friend void to_json(nlohmann::json& j, const CircleCollider& c);
public:
	/// ====================================================
	/// public : methods
	/// ====================================================

	CircleCollider();
	~CircleCollider() override = default;

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
float InternalGetRadiusCircle(uint64_t nativeHandle);
void InternalSetRadiusCircle(uint64_t nativeHandle, float radius);
bool InternalIsTriggerCircle(uint64_t nativeHandle);
void InternalSetTriggerCircle(uint64_t nativeHandle, bool trigger);
float InternalGetMassCircle(uint64_t nativeHandle);
void InternalSetMassCircle(uint64_t nativeHandle, float mass);
bool InternalIsUseOwnerScaleCircle(uint64_t nativeHandle);
void InternalSetUseOwnerScaleCircle(uint64_t nativeHandle, bool use);

} /// ONEngine
