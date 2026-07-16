#pragma once

/// std
#include <memory>

/// engine
#include "Engine/Core/Utility/Utility.h"

namespace ONEngine {

class IEmitShape; // 前方宣言

/// /////////////////////////////////////////////////////////////////
/// エフェクトの発生形状を指定するクラス
/// /////////////////////////////////////////////////////////////////
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
	std::unique_ptr<IEmitShape> impl_;

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


/// /////////////////////////////////////////////////////////////////
/// 発生形状のインターフェース
/// /////////////////////////////////////////////////////////////////
class IEmitShape {
public:
	virtual ~IEmitShape() = default;
	virtual Vector3 GetEmitPosition() = 0;
	virtual Vector3 GetEmitDirection(const Vector3& emittedPosition) = 0;
	virtual Vector3 GetCenter() const = 0;
	virtual EffectEmitShape::ShapeType GetType() const = 0;
	virtual std::unique_ptr<IEmitShape> Clone() const = 0;

	virtual const Sphere& GetSphere() const {
		static Sphere dummy{ Vector3::Zero, 0.0f };
		return dummy;
	}
	virtual const Cube& GetCube() const {
		static Cube dummy{ Vector3::Zero, Vector3::Zero };
		return dummy;
	}
	virtual const Cone& GetCone() const {
		static Cone dummy{ Vector3::Zero, 0.0f, 0.0f, 0.0f };
		return dummy;
	}
};


/// /////////////////////////////////////////////////////////////////
/// 球体発生形状
/// /////////////////////////////////////////////////////////////////
class SphereEmitShape : public IEmitShape {
public:
	SphereEmitShape(const Sphere& sphere) : sphere_(sphere) {}
	Vector3 GetEmitPosition() override;
	Vector3 GetEmitDirection(const Vector3& emittedPosition) override;
	Vector3 GetCenter() const override { return sphere_.center; }
	EffectEmitShape::ShapeType GetType() const override { return EffectEmitShape::ShapeType::Sphere; }
	std::unique_ptr<IEmitShape> Clone() const override { return std::make_unique<SphereEmitShape>(*this); }

	const Sphere& GetSphere() const override { return sphere_; }
	void SetSphere(const Sphere& sphere) { sphere_ = sphere; }
private:
	Sphere sphere_;
};


/// /////////////////////////////////////////////////////////////////
/// 立方体発生形状
/// /////////////////////////////////////////////////////////////////
class CubeEmitShape : public IEmitShape {
public:
	CubeEmitShape(const Cube& cube) : cube_(cube) {}
	Vector3 GetEmitPosition() override;
	Vector3 GetEmitDirection(const Vector3& emittedPosition) override;
	Vector3 GetCenter() const override { return cube_.center; }
	EffectEmitShape::ShapeType GetType() const override { return EffectEmitShape::ShapeType::Cube; }
	std::unique_ptr<IEmitShape> Clone() const override { return std::make_unique<CubeEmitShape>(*this); }

	const Cube& GetCube() const override { return cube_; }
	void SetCube(const Cube& cube) { cube_ = cube; }
private:
	Cube cube_;
};


/// /////////////////////////////////////////////////////////////////
/// 円錐発生形状
/// /////////////////////////////////////////////////////////////////
class ConeEmitShape : public IEmitShape {
public:
	ConeEmitShape(const Cone& cone) : cone_(cone) {}
	Vector3 GetEmitPosition() override;
	Vector3 GetEmitDirection(const Vector3& emittedPosition) override;
	Vector3 GetCenter() const override { return cone_.center; }
	EffectEmitShape::ShapeType GetType() const override { return EffectEmitShape::ShapeType::Cone; }
	std::unique_ptr<IEmitShape> Clone() const override { return std::make_unique<ConeEmitShape>(*this); }

	const Cone& GetCone() const override { return cone_; }
	void SetCone(const Cone& cone) { cone_ = cone; }
private:
	Cone cone_;
};

} /// ONEngine
