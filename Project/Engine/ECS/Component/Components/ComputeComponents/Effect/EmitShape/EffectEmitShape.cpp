#include "EffectEmitShape.h"

using namespace ONEngine;

/// std
#include <numbers>

/// engine
#include "Engine/Core/Utility/Utility.h"

EffectEmitShape::EffectEmitShape() {
	/// 球体の初期化
	sphere_.center = Vector3::Zero;
	sphere_.radius = 1.0f;
	/// 立方体の初期化
	cube_.center = Vector3::Zero;
	cube_.size = Vector3(1.0f, 1.0f, 1.0f);
	/// 円錐の初期化
	cone_.center = Vector3::Zero;
	cone_.angle = 30.0f;
	cone_.radius = 1.0f;
	cone_.height = 1.0f;
}

EffectEmitShape::EffectEmitShape(const EffectEmitShape& shape) {
	shapeType_ = shape.shapeType_;
	switch (shapeType_) {
	case ShapeType::Sphere:   sphere_ = shape.sphere_;     break;
	case ShapeType::Cube:     cube_ = shape.cube_;         break;
	case ShapeType::Cone:     cone_ = shape.cone_;         break;
	}
}


Vector3 EffectEmitShape::GetEmitPosition() {
	/// 形状ごとに発生位置を取得する
	switch (shapeType_) {
	case ShapeType::Sphere:
	{
		float theta = Random::Float(0.0f, 2.0f * std::numbers::pi_v<float>);
		float phi = Random::Float(0.0f, std::numbers::pi_v<float>);
		float r = Random::Float(0.0f, sphere_.radius);
		return sphere_.center + Vector3(
			r * std::sin(phi) * std::cos(theta),
			r * std::cos(phi),
			r * std::sin(phi) * std::sin(theta)
		);
	}
	case ShapeType::Cube:
	{
		return cube_.center + Vector3(
			Random::Float(-cube_.size.x, cube_.size.x),
			Random::Float(-cube_.size.y, cube_.size.y),
			Random::Float(-cube_.size.z, cube_.size.z)
		);
	}
	case ShapeType::Cone:
	{
		float theta = Random::Float(0.0f, 2.0f * std::numbers::pi_v<float>);
		float r = Random::Float(0.0f, cone_.radius);
		return cone_.center + Vector3(
			r * std::cos(theta),
			Random::Float(0.0f, cone_.height),
			r * std::sin(theta)
		);
	}

	default:
		break;
	}

	return Vector3();
}

Vector3 EffectEmitShape::GetEmitDirection(const Vector3& emitedPosition) {
	Vector3 direction = Vector3::Zero;
	/// 形状ごとに発生方向を取得する
	switch (shapeType_) {
	case ShapeType::Sphere:
		direction = emitedPosition - sphere_.center;
		break;
	case ShapeType::Cube:
		direction = emitedPosition - cube_.center;
		break;
	case ShapeType::Cone:
		direction = emitedPosition - cone_.center;
		break;
	};

	return direction.Normalize();
}

void EffectEmitShape::SetShapeType(ShapeType type) {
	shapeType_ = type;
}

void EffectEmitShape::SetSphere(const Vector3& center, float radius) {
	shapeType_ = ShapeType::Sphere;
	sphere_.center = center;
	sphere_.radius = radius;
}

void EffectEmitShape::SetSphere(const Sphere& sphere) {
	shapeType_ = ShapeType::Sphere;
	sphere_ = sphere;
}

void EffectEmitShape::SetCube(const Vector3& center, const Vector3& size) {
	shapeType_ = ShapeType::Cube;
	cube_.center = center;
	cube_.size = size;
}

void EffectEmitShape::SetCube(const Cube& cube) {
	shapeType_ = ShapeType::Cube;
	cube_ = cube;
}

void EffectEmitShape::SetCone(const Vector3& center, float angle, float radius, float height) {
	shapeType_ = ShapeType::Cone;
	cone_.center = center;
	cone_.angle = angle;
	cone_.radius = radius;
	cone_.height = height;
}

void EffectEmitShape::SetCone(const Cone& cone) {
	shapeType_ = ShapeType::Cone;
	cone_ = cone;
}

Vector3 EffectEmitShape::GetCenter() const {
	switch (shapeType_) {
	case ShapeType::Cube: return cube_.center;
	case ShapeType::Cone: return cone_.center;
	default: return Vector3::Zero;
	}
}

EffectEmitShape::ShapeType EffectEmitShape::GetType() const {
	return shapeType_;
}

const Sphere& EffectEmitShape::GetSphere() const {
	return sphere_;
}

const Cube& EffectEmitShape::GetCube() const {
	return cube_;
}

const Cone& EffectEmitShape::GetCone() const {
	return cone_;
}

