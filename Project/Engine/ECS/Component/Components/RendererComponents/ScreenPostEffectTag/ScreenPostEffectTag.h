#pragma once

/// std
#include <array>

/// external
#include <nlohmann/json.hpp>

/// engine
#include "../../Interface/IComponent.h"

#include "Engine/Core/Utility/Math/Vector2.h"
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
	PostEffectType_Pixelate,   ///< ピクセレート (解像度落とし)
	PostEffectType_Count	   ///< 要素数
};

/// ///////////////////////////////////////////////////
/// スクリーンにかけるポストエフェクトのフラグを持つコンポーネント
/// ///////////////////////////////////////////////////
namespace ONEngine {

class ECSGroup;
class EntityComponentSystem;

struct ScreenPostEffectFlags {
	std::array<bool, PostEffectType_Count> flags = {};
	float fisheyeStrength = 0.15f;
	float fisheyeScale = 0.9f;

	// Water Distortion
	float distortionStrength = 0.015f;
	float distortionSpeed = 1.0f;
	float distortionFrequency = 10.0f;

	// Depth Fog & Vignette
	Vector3 fogColor = Vector3(0.0f, 0.3f, 0.6f);
	float fogDensity = 0.05f;
	float fogWaterSurfaceY = 0.0f;
	float vignetteStrength = 1.5f;

	// Color Grading & Absorption
	Vector3 absorptionCoefficients = Vector3(1.0f, 0.3f, 0.0f);
	float contrast = 1.1f;
	float saturation = 0.9f;
	Vector3 colorFilter = Vector3(0.4f, 0.8f, 1.0f);

	// Caustics & Light Shafts
	float causticsScale = 0.5f;
	float causticsSpeed = 1.0f;
	float causticsIntensity = 1.5f;
	float lightShaftsIntensity = 1.5f;
	Vector3 lightDirection = Vector3(0.2f, -0.9f, 0.3f);

	// Pixelate
	float pixelSizeX = 8.0f;
	float pixelSizeY = 8.0f;

	// Size limitation (<=0 means full screen)
	int32_t postEffectWidth = -1;
	int32_t postEffectHeight = -1;

	// Start offset (0,0 by default)
	int32_t postEffectStartX = 0;
	int32_t postEffectStartY = 0;
	// Pivot (0: Top-Left, 1: Center)
	int32_t postEffectPivot = 0;
};

class ScreenPostEffectTag : public IComponent {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	ScreenPostEffectTag() = default;
	~ScreenPostEffectTag() override = default;

	/// @brief ポストエフェクトの有効/無効を設定する
	/// @param type 対象 of ポストエフェクトの種類
	/// @param isEnable true: 有効 false: 無効
	void SetPostEffectEnable(PostEffectType type, bool isEnable);

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

	/// Pixelate
	void SetPixelSizeX(float size);
	float GetPixelSizeX() const;
	void SetPixelSizeY(float size);
	float GetPixelSizeY() const;

	/// Size limitation
	void SetPostEffectWidth(int32_t width);
	int32_t GetPostEffectWidth() const;
	void SetPostEffectHeight(int32_t height);
	int32_t GetPostEffectHeight() const;

	/// Offset & Pivot
	void SetPostEffectStartX(int32_t x);
	int32_t GetPostEffectStartX() const;
	void SetPostEffectStartY(int32_t y);
	int32_t GetPostEffectStartY() const;
	void SetPostEffectPivot(int32_t pivot);
	int32_t GetPostEffectPivot() const;

	static Vector2 GetDispatchSize(ECSGroup* ecsGroup, EntityComponentSystem* entityComponentSystem);
	static Vector2 GetDispatchStartOffset(ECSGroup* ecsGroup, EntityComponentSystem* entityComponentSystem);

	ScreenPostEffectFlags flags_;

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

bool InternalGetScreenPostEffectEnabled(uint64_t nativeHandle, int32_t type);
void InternalSetScreenPostEffectEnabled(uint64_t nativeHandle, int32_t type, bool enabled);

} /// ONEngine
