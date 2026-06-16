#pragma once

/// engine
#include "Engine/Core/Utility/Math/Vector2.h"

/// /////////////////////////////////////////////////
/// @brief UV変形用データ構造体
/// /////////////////////////////////////////////////
namespace ONEngine {

struct UVTransform {
	Vector2 position = Vector2T<float>::Zero; /// オフセット
	Vector2 scale = Vector2::One; /// スケール
	float   rotate = 0.0f;   /// 回転
	float   pad1[3];
};


/// uv transform
void to_json(nlohmann::json& j, const UVTransform& uvTransform);
void from_json(const nlohmann::json& j, UVTransform& uvTransform);

} /// ONEngine
