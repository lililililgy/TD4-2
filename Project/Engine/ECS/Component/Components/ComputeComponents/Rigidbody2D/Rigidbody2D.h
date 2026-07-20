#pragma once

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "../../Interface/IComponent.h"
#include "Engine/Core/Utility/Math/Vector2.h"
#include <nlohmann/json_fwd.hpp>

namespace ONEngine {

class Rigidbody2D;

namespace ComponentDebug {
/// @brief Rigidbody2Dのデバッグ表示
void Rigidbody2DDebug(Rigidbody2D* rigidbody2D);
}

/// //////////////////////////////////////
/// Rigidbody2D Component
/// //////////////////////////////////////
class Rigidbody2D : public IComponent {
	friend void ComponentDebug::Rigidbody2DDebug(Rigidbody2D* rigidbody2D);
	friend void from_json(const nlohmann::json& j, Rigidbody2D& r);
	friend void to_json(nlohmann::json& j, const Rigidbody2D& r);

public:
	Rigidbody2D();
	~Rigidbody2D() override = default;

	void Reset() override;

public:
	/// =====================================================
	/// public : accessors
	/// =====================================================

	void SetVelocity(const Vector2& velocity);
	const Vector2& GetVelocity() const;

	void SetMass(float mass);
	float GetMass() const;

	void SetRestitution(float restitution);
	float GetRestitution() const;

	void SetUseGravity(bool use);
	bool GetUseGravity() const;

	void SetGravityScale(float scale);
	float GetGravityScale() const;

	void SetFreezeX(bool freeze);
	bool IsFreezeX() const;

	void SetFreezeY(bool freeze);
	bool IsFreezeY() const;

private:
	/// =====================================================
	/// private : objects
	/// =====================================================

	Vector2 velocity_ = Vector2::Zero;
	float mass_ = 1.0f;
	float restitution_ = 0.5f;
	bool useGravity_ = false;
	float gravityScale_ = 1.0f;
	bool freezeX_ = false;
	bool freezeY_ = false;
};

/// @brief Mono Internal Calls
void InternalGetVelocity2D(uint64_t nativeHandle, float* x, float* y);
void InternalSetVelocity2D(uint64_t nativeHandle, float x, float y);
float InternalGetRigidbody2DMass(uint64_t nativeHandle);
void InternalSetRigidbody2DMass(uint64_t nativeHandle, float mass);
float InternalGetRigidbody2DRestitution(uint64_t nativeHandle);
void InternalSetRigidbody2DRestitution(uint64_t nativeHandle, float restitution);
bool InternalGetRigidbody2DUseGravity(uint64_t nativeHandle);
void InternalSetRigidbody2DUseGravity(uint64_t nativeHandle, bool use);
float InternalGetRigidbody2DGravityScale(uint64_t nativeHandle);
void InternalSetRigidbody2DGravityScale(uint64_t nativeHandle, float scale);
bool InternalGetRigidbody2DFreezeX(uint64_t nativeHandle);
void InternalSetRigidbody2DFreezeX(uint64_t nativeHandle, bool freeze);
bool InternalGetRigidbody2DFreezeY(uint64_t nativeHandle);
void InternalSetRigidbody2DFreezeY(uint64_t nativeHandle, bool freeze);

} /// ONEngine
