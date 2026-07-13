#pragma once
#include <string>

namespace ONEngine {

class FontRasterizer final {
public:
	/// @brief フォントとテキストから動的テクスチャアセットを生成・更新する
	/// @param text レンダリングする文字列
	/// @param fontAssetPath 使用するフォントアセットへのパス (.ttf)
	/// @param fontSize フォントのサイズ
	/// @param texturePath 出力・更新対象の動的テクスチャアセットパス (例: "dynamic://xxx")
	/// @return 成功した場合は true
	static bool GenerateTexture(const std::string& text, const std::string& fontAssetPath, int fontSize, const std::string& texturePath);
};

} // namespace ONEngine
