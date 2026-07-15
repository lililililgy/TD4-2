#pragma once

/// std
#include <cstdint>

/// external
#include <nlohmann/json.hpp>

/// engine
#include "Engine/Core/Utility/Math/Vector4.h"
#include "Engine/Asset/Guid/Guid.h"
#include "UVTransform.h"

/// /////////////////////////////////////////////////
/// @brief ポストエフェクトの適用
/// /////////////////////////////////////////////////
enum PostEffectFlags_ {
	PostEffectFlags_None = 0,      ///< なし
	PostEffectFlags_Lighting = 1 << 0, ///< ライティング
	PostEffectFlags_Grayscale = 1 << 1, ///< グレースケール
	PostEffectFlags_EnvironmentReflection = 1 << 2, ///< 天球に合わせて環境反射を行う
	PostEffectFlags_Shadow = 1 << 3, ///< 影
	PostEffectFlags_Bloom = 1 << 4, ///< ブルーム
	PostEffectFlags_Outline = 1 << 5, ///< アウトライン
	PostEffectFlags_RadialBlur = 1 << 6, ///< ラジアルブラー
};

/// /////////////////////////////////////////////////
/// @brief GPUで利用するマテリアルデータ構造体
/// /////////////////////////////////////////////////
namespace ONEngine {

struct GPUMaterial {
	UVTransform uvTransform;     /// UV変形
	Vector4     baseColor;       /// 色
	Vector4     outlineColor;    /// フチ色
	uint32_t    postEffectFlags; /// ポストエフェクトのフラグ
	int32_t     entityId;        /// エンティティID
	int32_t     baseTextureId;   /// ベーステクスチャID
	int32_t     normalTextureId; /// 法線テクスチャID
	float       bloomIntensity;  /// ブルームの強度
	float       bloomThreshold;  /// ブルームのしきい値
	float       bloomRadius;     /// ブルームのぼかし半径
};


/// material
void to_json(nlohmann::json& j, const GPUMaterial& material);
void from_json(const nlohmann::json& j, GPUMaterial& material);

} /// ONEngine
