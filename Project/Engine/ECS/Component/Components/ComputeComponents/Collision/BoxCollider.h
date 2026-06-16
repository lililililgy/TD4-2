#pragma once

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "ICollider.h"
#include "Engine/Core/Utility/Math/Vector3.h"


namespace ONEngine {

class BoxCollider;

namespace ComponentDebug {
/// @brief BoxColliderのデバッグ表示
/// @param boxCollider 
void BoxColliderDebug(BoxCollider* boxCollider);

}	/// namespace ComponentDebug

/// //////////////////////////////////////
/// BoxCollider
/// //////////////////////////////////////
class BoxCollider : public ICollider {
	/// --------------- friend function --------------- ///
	friend void ComponentDebug::BoxColliderDebug(BoxCollider* boxCollider);
	friend void from_json(const nlohmann::json& j, BoxCollider& b);
	friend void to_json(nlohmann::json& j, const BoxCollider& b);
public:
	/// ====================================================
	/// public : methods
	/// ====================================================

	BoxCollider();
	~BoxCollider() override = default;

private:
	/// =====================================================
	/// private : objects
	/// =====================================================

	Vector3 size_;

public:
	/// =====================================================
	/// public : accessors
	/// =====================================================

	void SetSize(const Vector3& size);
	const Vector3& GetSize() const;

};

/// @brief Mono Internal Calls
Vector3 InternalGetSize(uint64_t nativeHandle);
void InternalSetSize(uint64_t nativeHandle, Vector3 size);
bool InternalIsTriggerBox(uint64_t nativeHandle);
void InternalSetTriggerBox(uint64_t nativeHandle, bool trigger);
float InternalGetMassBox(uint64_t nativeHandle);
void InternalSetMassBox(uint64_t nativeHandle, float mass);

} /// ONEngine
