#pragma once

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Vector4.h"

/// ////////////////////////////////////////////////////////////
/// DirectionalLight
/// ////////////////////////////////////////////////////////////
namespace ONEngine {

class DirectionalLight : public IComponent {
	friend class AnimationPlayer;
public:

	/// ===================================================
	/// public : methods
	/// ===================================================

	DirectionalLight();
	~DirectionalLight() {}

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	float intensity_;
	Vector3 direction_;
	Vector4 color_;

public:
	/// ===================================================
	/// public : accessor
	/// ===================================================
	
	/// @brief 光の強度の設定
	/// @param intensity 光の強度
	void SetIntensity(float intensity) { intensity_ = intensity; }
	
	/// @brief 光の方向の設定
	/// @param direction 光の方向
	void SetDirection(const Vector3& direction) { direction_ = direction; }
	
	/// @brief 光の色の設定
	/// @param color 光の色
	void SetColor(const Vector4& color) { color_ = color; }


	/// @brief 光の強度の取得
	/// @return 光の強度
	float GetIntensity() const { return intensity_; }
	
	/// @brief 光の方向の取得
	/// @return 光の方向
	const Vector3& GetDirection() const { return direction_; }

	/// @brief 光の色の取得
	/// @return 光の色
	const Vector4& GetColor() const { return color_; }

	/// @brief アニメーション制御用強度への参照取得
	float& GetIntensityForAnimation() { return intensity_; }

	/// @brief アニメーション制御用色への参照取得
	Vector4& GetColorForAnimation() { return color_; }
};


/// ////////////////////////////////////////////////////////////
/// PointLight
/// ////////////////////////////////////////////////////////////
class PointLight : public IComponent {
public:
	PointLight();
	~PointLight() {}

	void SetIntensity(float intensity) { intensity_ = intensity; }
	void SetColor(const Vector4& color) { color_ = color; }
	void SetRadius(float radius) { radius_ = radius; }

	float GetIntensity() const { return intensity_; }
	const Vector4& GetColor() const { return color_; }
	float GetRadius() const { return radius_; }

private:
	float intensity_;
	float radius_;
	Vector4 color_;
};


/// ////////////////////////////////////////////////////////////
/// SpotLight
/// ////////////////////////////////////////////////////////////
class SpotLight : public IComponent {
public:
	SpotLight();
	~SpotLight() {}

	void SetIntensity(float intensity) { intensity_ = intensity; }
	void SetColor(const Vector4& color) { color_ = color; }
	void SetDirection(const Vector3& direction) { direction_ = direction; }
	void SetRadius(float radius) { radius_ = radius; }
	void SetInnerAngle(float angle) { innerAngle_ = angle; }
	void SetOuterAngle(float angle) { outerAngle_ = angle; }

	float GetIntensity() const { return intensity_; }
	const Vector4& GetColor() const { return color_; }
	const Vector3& GetDirection() const { return direction_; }
	float GetRadius() const { return radius_; }
	float GetInnerAngle() const { return innerAngle_; }
	float GetOuterAngle() const { return outerAngle_; }

private:
	float intensity_;
	Vector3 direction_;
	Vector4 color_;

	float radius_;
	float innerAngle_;
	float outerAngle_;
};


/// //////////////////////////////////////////////
/// componentのデバッグ表示関数
/// //////////////////////////////////////////////
void DirectionalLightDebug(DirectionalLight* light);
void PointLightDebug(PointLight* light);
void SpotLightDebug(SpotLight* light);


} /// ONEngine
