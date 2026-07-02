#pragma once

/// external
#include <nlohmann/json.hpp>

/// engine
#include "../../Interface/IComponent.h"

#include "Engine/Core/Utility/Math/Vector3.h"

/// @brief ポストエフェクトの種類
enum PostEffectType {
	PostEffectType_Grayscale,  ///< グレースケール
	PostEffectType_RadialBlur, ///< ラジアルブラー
	PostEffectType_Fisheye,    ///< 魚眼レンズ
	PostEffectType_WaterDistortion,     ///< スクリーンスペース歪み
	PostEffectType_WaterDepthFogVignette,///< 深度フォグ & ビネット
	PostEffectType_WaterColorGrading,   ///< カラーグレーディング & 吸収
	PostEffectType_WaterCausticsLightShafts, ///< コースティクス & ライトシャフト
	PostEffectType_Count	   ///< 要素数
};

/// ///////////////////////////////////////////////////
/// スクリーンにかけるポストエフェクトのフラグを持つコンポーネント
/// ///////////////////////////////////////////////////
namespace ONEngine {

class ScreenPostEffectTag : public IComponent {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	ScreenPostEffectTag() = default;
	~ScreenPostEffectTag() override = default;

	/// @brief ポストエフェクトの有効/無効を設定する
	/// @param type 対象 of ポストエフェクトの種類
	/// @param enable true: 有効 false: 無効
	void SetPostEffectEnable(PostEffectType type, bool enable);

	/// @brief 指定した種類のポストエフェクトが有効かどうかを返す
	/// @param type 確認するポストエフェクトの種類
	/// @return true: 有効 false: 無効
	bool GetPostEffectEnable(PostEffectType type) const;

	/// @brief 魚眼レンズの歪み強度を設定する
	void SetFisheyeStrength(float strength);

	/// @brief 魚眼レンズ of 歪み強度を取得する
	float GetFisheyeStrength() const;

	/// @brief 魚眼レンズ of 表示スケールを設定する
	void SetFisheyeScale(float scale);

	/// @brief 魚眼レンズ of 表示スケールを取得する
	float GetFisheyeScale() const;

	/// Wave Distortion
	void SetWaterDistortionStrength(float strength);
	float GetWaterDistortionStrength() const;
	void SetWaterDistortionSpeed(float speed);
	float GetWaterDistortionSpeed() const;
	void SetWaterDistortionFrequency(float freq);
	float GetWaterDistortionFrequency() const;

	/// Depth Fog & Vignette
	void SetWaterFogColor(const Vector3& color);
	Vector3 GetWaterFogColor() const;
	void SetWaterFogDensity(float density);
	float GetWaterFogDensity() const;
	void SetWaterFogWaterSurfaceY(float y);
	float GetWaterFogWaterSurfaceY() const;
	void SetWaterVignetteStrength(float strength);
	float GetWaterVignetteStrength() const;

	/// Color Grading & Absorption
	void SetWaterAbsorptionCoefficients(const Vector3& coeffs);
	Vector3 GetWaterAbsorptionCoefficients() const;
	void SetWaterContrast(float contrast);
	float GetWaterContrast() const;
	void SetWaterSaturation(float sat);
	float GetWaterSaturation() const;
	void SetWaterColorFilter(const Vector3& filter);
	Vector3 GetWaterColorFilter() const;

	/// Caustics & Light Shafts
	void SetWaterCausticsScale(float scale);
	float GetWaterCausticsScale() const;
	void SetWaterCausticsSpeed(float speed);
	float GetWaterCausticsSpeed() const;
	void SetWaterCausticsIntensity(float intensity);
	float GetWaterCausticsIntensity() const;
	void SetWaterLightShaftsIntensity(float intensity);
	float GetWaterLightShaftsIntensity() const;
	void SetWaterLightDirection(const Vector3& dir);
	Vector3 GetWaterLightDirection() const;

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

};


namespace ComponentDebug {
	void ScreenPostEffectTagDebug(ScreenPostEffectTag* component);
}


/// Json変換
void from_json(const nlohmann::json& j, ScreenPostEffectTag& c);
void to_json(nlohmann::json& j, const ScreenPostEffectTag& c);

} /// ONEngine
