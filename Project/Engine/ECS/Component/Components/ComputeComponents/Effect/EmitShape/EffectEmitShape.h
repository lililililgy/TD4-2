#pragma once

/// engine
#include "Engine/Core/Utility/Utility.h"

/// /////////////////////////////////////////////////////////////////
/// エフェクトの発生形状を指定するクラス
/// /////////////////////////////////////////////////////////////////
namespace ONEngine {

class EffectEmitShape final {
public:
	/// =================================================
	/// public : sub class
	/// =================================================

	/// @brief エミッターの形状
	enum class ShapeType {
		Sphere,
		Cube,
		Cone,
	};

public:
	/// =================================================
	/// public : methods
	/// =================================================

	EffectEmitShape();
	EffectEmitShape(const EffectEmitShape& shape);
	~EffectEmitShape() = default;

	/// @brief エミッターの形状の代入演算子
	/// @param shape エミッターの形状
	/// @return 形状の参照
	EffectEmitShape& operator= (const EffectEmitShape& shape);

	/// @brief エミッターの座標
	Vector3 GetEmitPosition();

	/// @brief エミット後の方向ベクトルを取得する
	/// @param emitedPosition 出力後の座標
	/// @return エミット後の方向ベクトル
	Vector3 GetEmitDirection(const Vector3& emitedPosition);

private:
	/// =================================================
	/// private : objects
	/// =================================================
	ShapeType shapeType_ = ShapeType::Cone;
	union {
		Sphere sphere_;
		Cube cube_;
		Cone cone_;
	};


public:
	///	===========================================
	/// public : accessors
	///	===========================================

	void SetShapeType(ShapeType type);

	void SetSphere(const Vector3& center, float radius);
	void SetSphere(const Sphere& sphere);

	void SetCube(const Vector3& center, const Vector3& size);
	void SetCube(const Cube& cube);

	void SetCone(const Vector3& center, float angle, float radius, float height);
	void SetCone(const Cone& cone);

	Vector3 GetCenter() const;

	ShapeType GetType() const;

	const Sphere& GetSphere() const;
	const Cube& GetCube() const;
	const Cone& GetCone() const;

};


inline EffectEmitShape& EffectEmitShape::operator=(const EffectEmitShape& shape) {
	shapeType_ = shape.shapeType_;
	switch (shapeType_) {
	case ShapeType::Sphere: sphere_ = shape.sphere_; break;
	case ShapeType::Cube: cube_ = shape.cube_; break;
	case ShapeType::Cone: cone_ = shape.cone_; break;
	}

	return *this;
}

} /// ONEngine
