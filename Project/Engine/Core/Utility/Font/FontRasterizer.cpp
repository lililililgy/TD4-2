#include "FontRasterizer.h"

/// directX12 / engine
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Asset/Assets/Texture/Texture.h"
#include "Engine/Asset/Assets/Font/FontAsset.h"
#include "Engine/Core/Utility/Tools/Log.h"

#include <vector>
#include <algorithm>
#include <cmath>

// stb_truetype の実装マクロを定義してインクルード
#define STB_TRUETYPE_IMPLEMENTATION
#include "Externals/imgui/imstb_truetype.h"

namespace {
	// UTF-8 を UTF-32 (Unicodeコードポイント) にデコードする
	std::vector<uint32_t> Utf8ToUtf32(const std::string& str) {
		std::vector<uint32_t> utf32;
		for (size_t i = 0; i < str.size();) {
			uint32_t cp = 0;
			unsigned char c = str[i];
			if (c < 0x80) { cp = c; i += 1; }
			else if ((c & 0xE0) == 0xC0) {
				if (i + 1 >= str.size()) break;
				cp = ((c & 0x1F) << 6) | (str[i+1] & 0x3F); i += 2;
			}
			else if ((c & 0xF0) == 0xE0) {
				if (i + 2 >= str.size()) break;
				cp = ((c & 0x0F) << 12) | ((str[i+1] & 0x3F) << 6) | (str[i+2] & 0x3F); i += 3;
			}
			else if ((c & 0xF8) == 0xF0) {
				if (i + 3 >= str.size()) break;
				cp = ((c & 0x07) << 18) | ((str[i+1] & 0x3F) << 12) | ((str[i+2] & 0x3F) << 6) | (str[i+3] & 0x3F); i += 4;
			}
			else { i += 1; continue; } // 無効なバイトはスキップ
			utf32.push_back(cp);
		}
		return utf32;
	}
}

namespace ONEngine {

bool FontRasterizer::GenerateTexture(const std::string& text, const std::string& fontAssetPath, int fontSize, const std::string& texturePath) {
	if (text.empty()) {
		// 空文字列の場合は、最低限の1x1の透明テクスチャを割り当てる（クラッシュ防止）
		auto* assetCollection = Asset::AssetCollection::GetInstance();
		DxManager* dxm = DxManager::GetInstance();
		if (!dxm) return false;

		uint8_t transparentPixel[4] = { 0, 0, 0, 0 };
		if (assetCollection->HasAsset(texturePath)) {
			Asset::Texture* tex = assetCollection->GetTexture(texturePath);
			if (tex) {
				tex->RecreateFromPixels(transparentPixel, 1, 1, dxm->GetDxDevice(), dxm->GetDxSRVHeap(), dxm->GetDxCommand());
				return true;
			}
		} else {
			Asset::Texture newTex;
			newTex.RecreateFromPixels(transparentPixel, 1, 1, dxm->GetDxDevice(), dxm->GetDxSRVHeap(), dxm->GetDxCommand());
			assetCollection->AddAsset<Asset::Texture>(texturePath, std::move(newTex));
			return true;
		}
		return false;
	}

	auto* assetCollection = Asset::AssetCollection::GetInstance();
	const Asset::FontAsset* fontAsset = assetCollection->GetFont(fontAssetPath);
	if (!fontAsset) {
		// ロードされていなければロードを試みる
		assetCollection->Load(fontAssetPath, Asset::AssetType::Font);
		fontAsset = assetCollection->GetFont(fontAssetPath);
		if (!fontAsset) {
			Console::LogError("[FontRasterizer] Font asset not found: \"" + fontAssetPath + "\"");
			return false;
		}
	}

	// fontAsset のバイナリデータを使って stb_truetype を初期化
	stbtt_fontinfo font;
	if (!stbtt_InitFont(&font, fontAsset->fontData.data(), 0)) {
		Console::LogError("[FontRasterizer] Failed to initialize stb_truetype with font asset: \"" + fontAssetPath + "\"");
		return false;
	}

	// 1. スケール及びレイアウトの計算
	float scale = stbtt_ScaleForPixelHeight(&font, static_cast<float>(fontSize));

	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
	int baseline = static_cast<int>(ascent * scale);
	int rowHeight = static_cast<int>((ascent - descent + lineGap) * scale);

	std::vector<uint32_t> utf32Text = Utf8ToUtf32(text);

	struct GlyphInfo {
		uint32_t codepoint;
		int x0, y0, x1, y1;
		int advanceWidth;
		int leftSideBearing;
		int lineIndex;
		int xOffset;
	};

	std::vector<GlyphInfo> glyphs;
	int currentLine = 0;
	int currentX = 0;
	int maxLineWidth = 0;

	for (uint32_t cp : utf32Text) {
		if (cp == '\n') {
			maxLineWidth = (std::max)(maxLineWidth, currentX);
			currentX = 0;
			currentLine++;
			continue;
		}

		GlyphInfo info = {};
		info.codepoint = cp;
		info.lineIndex = currentLine;

		int advance, lsb;
		stbtt_GetCodepointHMetrics(&font, cp, &advance, &lsb);
		info.advanceWidth = static_cast<int>(advance * scale);
		info.leftSideBearing = static_cast<int>(lsb * scale);

		stbtt_GetCodepointBitmapBox(&font, cp, scale, scale, &info.x0, &info.y0, &info.x1, &info.y1);

		info.xOffset = currentX;
		glyphs.push_back(info);

		currentX += info.advanceWidth;
	}
	maxLineWidth = (std::max)(maxLineWidth, currentX);
	int lineCount = currentLine + 1;

	int totalWidth = maxLineWidth;
	int totalHeight = lineCount * rowHeight;

	if (totalWidth <= 0 || totalHeight <= 0) {
		return false;
	}

	// 2. RGBA ピクセルバッファの確保とラスタライズ
	std::vector<uint8_t> rgba(totalWidth * totalHeight * 4, 0);

	for (const auto& glyph : glyphs) {
		if (glyph.codepoint == ' ' || glyph.codepoint == '\t') {
			continue;
		}

		int w = glyph.x1 - glyph.x0;
		int h = glyph.y1 - glyph.y0;
		if (w <= 0 || h <= 0) continue;

		std::vector<uint8_t> mono(w * h, 0);
		stbtt_MakeCodepointBitmap(&font, mono.data(), w, h, w, scale, scale, glyph.codepoint);

		// 行の位置とベースラインを加味して配置先のX, Y開始位置を決定する
		int startX = glyph.xOffset + glyph.leftSideBearing;
		int startY = glyph.lineIndex * rowHeight + baseline + glyph.y0;

		for (int y = 0; y < h; ++y) {
			int destY = startY + y;
			if (destY < 0 || destY >= totalHeight) continue;

			for (int x = 0; x < w; ++x) {
				int destX = startX + x;
				if (destX < 0 || destX >= totalWidth) continue;

				uint8_t alpha = mono[y * w + x];
				if (alpha == 0) continue;

				size_t destIdx = (destY * totalWidth + destX) * 4;
				rgba[destIdx + 0] = 255; // Red
				rgba[destIdx + 1] = 255; // Green
				rgba[destIdx + 2] = 255; // Blue
				rgba[destIdx + 3] = (std::max)(rgba[destIdx + 3], alpha);
			}
		}
	}

	// 3. テクスチャアセットの作成または再利用
	DxManager* dxm = DxManager::GetInstance();
	if (!dxm) {
		Console::LogError("[FontRasterizer] DxManager instance is null");
		return false;
	}

	if (assetCollection->HasAsset(texturePath)) {
		Asset::Texture* existingTexture = assetCollection->GetTexture(texturePath);
		if (existingTexture) {
			existingTexture->RecreateFromPixels(rgba.data(), totalWidth, totalHeight, dxm->GetDxDevice(), dxm->GetDxSRVHeap(), dxm->GetDxCommand());
			return true;
		}
	} else {
		Asset::Texture newTexture;
		newTexture.guid = GenerateGuid();
		newTexture.RecreateFromPixels(rgba.data(), totalWidth, totalHeight, dxm->GetDxDevice(), dxm->GetDxSRVHeap(), dxm->GetDxCommand());
		assetCollection->AddAsset<Asset::Texture>(texturePath, std::move(newTexture));
		return true;
	}

	return false;
}

} // namespace ONEngine
