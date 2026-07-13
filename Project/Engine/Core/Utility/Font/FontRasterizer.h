#pragma once
#include <string>
#include "Engine/Core/Utility/Math/Vector4.h"

namespace ONEngine {

enum class HorizontalAlignment;

class FontRasterizer final {
public:
	/// @brief フォントとテキストから動的テクスチャアセットを生成・更新する
	static bool GenerateTexture(
		const std::string& text, 
		const std::string& fontAssetPath, 
		int fontSize, 
		const std::string& texturePath,
		HorizontalAlignment alignment = static_cast<HorizontalAlignment>(0),
		const Vector4& textColor = { 1.0f, 1.0f, 1.0f, 1.0f },
		const Vector4& outlineColor = { 0.0f, 0.0f, 0.0f, 1.0f },
		int outlineWidth = 0
	);
};

} // namespace ONEngine
