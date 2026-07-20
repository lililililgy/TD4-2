#include "Rigidbody2D.h"
#include <nlohmann/json.hpp>

/// externals
#include <imgui.h>

/// engine
#include "Engine/Editor/EditorUtils.h"

namespace ONEngine {

Rigidbody2D::Rigidbody2D() {
	Reset();
}

void Rigidbody2D::Reset() {
	velocity_ = Vector2::Zero;
	mass_ = 1.0f;
	restitution_ = 0.5f;
	useGravity_ = false;
	gravityScale_ = 1.0f;
	freezeX_ = false;
	freezeY_ = false;
}

void Rigidbody2D::SetVelocity(const Vector2& velocity) {
	velocity_ = velocity;
}

const Vector2& Rigidbody2D::GetVelocity() const {
	return velocity_;
}

void Rigidbody2D::SetMass(float mass) {
	mass_ = (std::max)(0.0001f, mass);
}

float Rigidbody2D::GetMass() const {
	return mass_;
}

void Rigidbody2D::SetRestitution(float restitution) {
	restitution_ = (std::max)(0.0f, (std::min)(1.0f, restitution));
}

float Rigidbody2D::GetRestitution() const {
	return restitution_;
}

void Rigidbody2D::SetUseGravity(bool use) {
	useGravity_ = use;
}

bool Rigidbody2D::GetUseGravity() const {
	return useGravity_;
}

void Rigidbody2D::SetGravityScale(float scale) {
	gravityScale_ = scale;
}

float Rigidbody2D::GetGravityScale() const {
	return gravityScale_;
}

void Rigidbody2D::SetFreezeX(bool freeze) {
	freezeX_ = freeze;
}

bool Rigidbody2D::IsFreezeX() const {
	return freezeX_;
}

void Rigidbody2D::SetFreezeY(bool freeze) {
	freezeY_ = freeze;
}

bool Rigidbody2D::IsFreezeY() const {
	return freezeY_;
}

void ComponentDebug::Rigidbody2DDebug(Rigidbody2D* rb) {
	if (!rb) return;

	ImGui::SeparatorText("Rigidbody2D Parameters");

	ImGui::DragFloat2("Velocity", &rb->velocity_.x, 0.1f);
	ImGui::DragFloat("Mass", &rb->mass_, 0.1f, 0.001f, 10000.0f);
	ImGui::DragFloat("Restitution", &rb->restitution_, 0.01f, 0.0f, 1.0f);

	ImGui::Checkbox("Use Gravity", &rb->useGravity_);
	if (rb->useGravity_) {
		ImGui::DragFloat("Gravity Scale", &rb->gravityScale_, 0.1f);
	}

	ImGui::Checkbox("Freeze X", &rb->freezeX_);
	ImGui::Checkbox("Freeze Y", &rb->freezeY_);
}

void from_json(const nlohmann::json& j, Rigidbody2D& r) {
	r.enable = j.value("enable", 1);
	r.velocity_ = j.value("velocity", Vector2::Zero);
	r.mass_ = j.value("mass", 1.0f);
	r.restitution_ = j.value("restitution", 0.5f);
	r.useGravity_ = j.value("useGravity", false);
	r.gravityScale_ = j.value("gravityScale", 1.0f);
	r.freezeX_ = j.value("freezeX", false);
	r.freezeY_ = j.value("freezeY", false);
}

void to_json(nlohmann::json& j, const Rigidbody2D& r) {
	j = nlohmann::json{
		{ "type", "Rigidbody2D" },
		{ "enable", r.enable },
		{ "velocity", r.velocity_ },
		{ "mass", r.mass_ },
		{ "restitution", r.restitution_ },
		{ "useGravity", r.useGravity_ },
		{ "gravityScale", r.gravityScale_ },
		{ "freezeX", r.freezeX_ },
		{ "freezeY", r.freezeY_ }
	};
}

/// Mono Internal Calls Implementation
void InternalGetVelocity2D(uint64_t nativeHandle, float* x, float* y) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	if (rb && x && y) {
		*x = rb->GetVelocity().x;
		*y = rb->GetVelocity().y;
	}
}

void InternalSetVelocity2D(uint64_t nativeHandle, float x, float y) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	if (rb) {
		rb->SetVelocity(Vector2(x, y));
	}
}

float InternalGetRigidbody2DMass(uint64_t nativeHandle) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	return rb ? rb->GetMass() : 1.0f;
}

void InternalSetRigidbody2DMass(uint64_t nativeHandle, float mass) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	if (rb) {
		rb->SetMass(mass);
	}
}

float InternalGetRigidbody2DRestitution(uint64_t nativeHandle) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	return rb ? rb->GetRestitution() : 0.5f;
}

void InternalSetRigidbody2DRestitution(uint64_t nativeHandle, float restitution) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	if (rb) {
		rb->SetRestitution(restitution);
	}
}

bool InternalGetRigidbody2DUseGravity(uint64_t nativeHandle) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	return rb ? rb->GetUseGravity() : false;
}

void InternalSetRigidbody2DUseGravity(uint64_t nativeHandle, bool use) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	if (rb) {
		rb->SetUseGravity(use);
	}
}

float InternalGetRigidbody2DGravityScale(uint64_t nativeHandle) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	return rb ? rb->GetGravityScale() : 1.0f;
}

void InternalSetRigidbody2DGravityScale(uint64_t nativeHandle, float scale) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	if (rb) {
		rb->SetGravityScale(scale);
	}
}

bool InternalGetRigidbody2DFreezeX(uint64_t nativeHandle) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	return rb ? rb->IsFreezeX() : false;
}

void InternalSetRigidbody2DFreezeX(uint64_t nativeHandle, bool freeze) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	if (rb) {
		rb->SetFreezeX(freeze);
	}
}

bool InternalGetRigidbody2DFreezeY(uint64_t nativeHandle) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	return rb ? rb->IsFreezeY() : false;
}

void InternalSetRigidbody2DFreezeY(uint64_t nativeHandle, bool freeze) {
	Rigidbody2D* rb = reinterpret_cast<Rigidbody2D*>(nativeHandle);
	if (rb) {
		rb->SetFreezeY(freeze);
	}
}

} /// ONEngine
