#include "EffectEmitShape.h"

using namespace ONEngine;

/// std
#include <numbers>

/// engine
#include "Engine/Core/Utility/Utility.h"

// --- EffectEmitShape ---
EffectEmitShape::EffectEmitShape() {
	/// デフォルトで円錐形状を設定
	impl_ = std::make_unique<ConeEmitShape>(Cone{ Vector3::Zero, 30.0f, 1.0f, 1.0f });
}

EffectEmitShape::EffectEmitShape(const EffectEmitShape& shape) {
	impl_ = shape.impl_ ? shape.impl_->Clone() : nullptr;
}

EffectEmitShape& EffectEmitShape::operator=(const EffectEmitShape& shape) {
	if (this != &shape) {
		impl_ = shape.impl_ ? shape.impl_->Clone() : nullptr;
	}
	return *this;
}

Vector3 EffectEmitShape::GetEmitPosition() {
	return impl_ ? impl_->GetEmitPosition() : Vector3::Zero;
}

Vector3 EffectEmitShape::GetEmitDirection(const Vector3& emitedPosition) {
	return impl_ ? impl_->GetEmitDirection(emitedPosition) : Vector3::Zero;
}

void EffectEmitShape::SetShapeType(ShapeType type) {
	if (impl_ && impl_->GetType() == type) return;

	switch (type) {
	case ShapeType::Sphere:
		impl_ = std::make_unique<SphereEmitShape>(Sphere{ Vector3::Zero, 1.0f });
		break;
	case ShapeType::Cube:
		impl_ = std::make_unique<CubeEmitShape>(Cube{ Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f) });
		break;
	case ShapeType::Cone:
		impl_ = std::make_unique<ConeEmitShape>(Cone{ Vector3::Zero, 30.0f, 1.0f, 1.0f });
		break;
	}
}

void EffectEmitShape::SetSphere(const Vector3& center, float radius) {
	impl_ = std::make_unique<SphereEmitShape>(Sphere{ center, radius });
}

void EffectEmitShape::SetSphere(const Sphere& sphere) {
	impl_ = std::make_unique<SphereEmitShape>(sphere);
}

void EffectEmitShape::SetCube(const Vector3& center, const Vector3& size) {
	impl_ = std::make_unique<CubeEmitShape>(Cube{ center, size });
}

void EffectEmitShape::SetCube(const Cube& cube) {
	impl_ = std::make_unique<CubeEmitShape>(cube);
}

void EffectEmitShape::SetCone(const Vector3& center, float angle, float radius, float height) {
	impl_ = std::make_unique<ConeEmitShape>(Cone{ center, angle, radius, height });
}

void EffectEmitShape::SetCone(const Cone& cone) {
	impl_ = std::make_unique<ConeEmitShape>(cone);
}

Vector3 EffectEmitShape::GetCenter() const {
	return impl_ ? impl_->GetCenter() : Vector3::Zero;
}

EffectEmitShape::ShapeType EffectEmitShape::GetType() const {
	return impl_ ? impl_->GetType() : ShapeType::Cone;
}

const Sphere& EffectEmitShape::GetSphere() const {
	static Sphere dummy{ Vector3::Zero, 0.0f };
	return impl_ ? impl_->GetSphere() : dummy;
}

const Cube& EffectEmitShape::GetCube() const {
	static Cube dummy{ Vector3::Zero, Vector3::Zero };
	return impl_ ? impl_->GetCube() : dummy;
}

const Cone& EffectEmitShape::GetCone() const {
	static Cone dummy{ Vector3::Zero, 0.0f, 0.0f, 0.0f };
	return impl_ ? impl_->GetCone() : dummy;
}


// --- SphereEmitShape ---
Vector3 SphereEmitShape::GetEmitPosition() {
	float theta = Random::Float(0.0f, 2.0f * std::numbers::pi_v<float>);
	float phi = Random::Float(0.0f, std::numbers::pi_v<float>);
	float r = Random::Float(0.0f, sphere_.radius);
	return sphere_.center + Vector3(
		r * std::sin(phi) * std::cos(theta),
		r * std::cos(phi),
		r * std::sin(phi) * std::sin(theta)
	);
}

Vector3 SphereEmitShape::GetEmitDirection(const Vector3& emittedPosition) {
	Vector3 direction = emittedPosition - sphere_.center;
	return direction.Normalize();
}


// --- CubeEmitShape ---
Vector3 CubeEmitShape::GetEmitPosition() {
	return cube_.center + Vector3(
		Random::Float(-cube_.size.x, cube_.size.x),
		Random::Float(-cube_.size.y, cube_.size.y),
		Random::Float(-cube_.size.z, cube_.size.z)
	);
}

Vector3 CubeEmitShape::GetEmitDirection(const Vector3& emittedPosition) {
	Vector3 direction = emittedPosition - cube_.center;
	return direction.Normalize();
}


// --- ConeEmitShape ---
Vector3 ConeEmitShape::GetEmitPosition() {
	float theta = Random::Float(0.0f, 2.0f * std::numbers::pi_v<float>);
	float r = Random::Float(0.0f, cone_.radius);
	return cone_.center + Vector3(
		r * std::cos(theta),
		Random::Float(0.0f, cone_.height),
		r * std::sin(theta)
	);
}

Vector3 ConeEmitShape::GetEmitDirection(const Vector3& emittedPosition) {
	Vector3 direction = emittedPosition - cone_.center;
	return direction.Normalize();
}

