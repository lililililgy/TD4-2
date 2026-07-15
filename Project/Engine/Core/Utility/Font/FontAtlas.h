#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Engine/Core/Utility/Math/Vector2.h"
#include "Engine/Core/Utility/Math/Vector4.h"
#include "Engine/Asset/Assets/Texture/Texture.h"
#include "Externals/imgui/imstb_truetype.h"

namespace ONEngine {

struct GlyphData {
	uint32_t codepoint;
	Vector2 uvMin;
	Vector2 uvMax;
	Vector2 size;      // ピクセルサイズ
	Vector2 bearing;   // ベースラインおよび左ベアリングからのオフセット
	float advance;     // 送り幅
};

class FontAtlas {
private:
	std::string fontPath_;
	int fontSize_;
	int outlineWidth_;

	// アトラス用テクスチャ
	std::string texturePath_;
	Asset::Texture* texture_ = nullptr;

	// アトラスのピクセルバッファ (R8G8形式: 1ピクセルあたり2バイト)
	// R: 文字本体アルファ, G: フチアルファ
	std::vector<uint8_t> pixelBuffer_;
	int atlasWidth_ = 1024;
	int atlasHeight_ = 1024;

	// パッキング用状態 (Shelf packing)
	int currentX_ = 0;
	int currentY_ = 0;
	int maxRowHeight_ = 0;
	int padding_ = 4; // グリフ間のパディング

	stbtt_fontinfo fontInfo_;
	std::vector<uint8_t> fontData_; // TTFファイルのバイナリデータを維持

	std::unordered_map<uint32_t, GlyphData> glyphs_;

	bool isDirty_ = false;

public:
	FontAtlas(const std::string& fontPath, int fontSize, int outlineWidth);
	~FontAtlas();

	bool Initialize();

	// グリフ情報の取得。アトラスに無ければオンデマンドで追加
	const GlyphData* GetGlyph(uint32_t codepoint);

	// 必要に応じてGPUテクスチャを更新
	void UpdateGpuTexture();

	Asset::Texture* GetTexture() const { return texture_; }
	const std::string& GetTexturePath() const { return texturePath_; }

	float GetScale() const;
	void GetFontVMetrics(int* ascent, int* descent, int* lineGap) const;

private:
	bool AddGlyphToAtlas(uint32_t codepoint, GlyphData& outGlyph);
};

} // namespace ONEngine
