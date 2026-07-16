#include "FontRasterizer.h"

/// directX12 / engine
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Asset/Assets/Texture/Texture.h"
#include "Engine/Asset/Assets/Font/FontAsset.h"
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/ECS/Component/Components/RendererComponents/Text/TextRenderer.h"

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

bool FontRasterizer::GenerateTexture(
	const std::string& text, 
	const std::string& fontAssetPath, 
	int fontSize, 
	const std::string& texturePath,
	HorizontalAlignment alignment,
	const Vector4& textColor,
	const Vector4& outlineColor,
	int outlineWidth,
	int characterSpacing,
	float lineSpacing
) {
	if (text.empty()) {
		// 空文字列の場合は、最低限の1x1の透明テクスチャを割り当てる（クラッシュ防止）
		auto* assetCollection = Asset::AssetCollection::GetInstance();
		DxManager* dxm = DxManager::GetInstance();
		if (!dxm) return false;

		uint8_t transparentPixel[4] = { 0, 0, 0, 0 };
		Asset::Texture* tex = assetCollection->GetTexture(texturePath);
		if (tex) {
			tex->RecreateFromPixels(transparentPixel, 1, 1, dxm->GetDxDevice(), dxm->GetDxSRVHeap(), dxm->GetDxCommand());
			return true;
		} else {
			Asset::Texture newTex;
			newTex.guid = GenerateGuid();
			newTex.RecreateFromPixels(transparentPixel, 1, 1, dxm->GetDxDevice(), dxm->GetDxSRVHeap(), dxm->GetDxCommand());
			assetCollection->AddAsset<Asset::Texture>(texturePath, std::move(newTex));
			return true;
		}
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
	int rowHeight = static_cast<int>((ascent - descent + lineGap) * scale * lineSpacing);

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
	std::vector<int> lineWidths;
	int currentLine = 0;
	int currentX = 0;
	int maxLineWidth = 0;

	// フチ取りのための余白を定義
	int drawOutlineWidth = (outlineWidth > 0) ? outlineWidth : 0;

	for (size_t i = 0; i < utf32Text.size(); ++i) {
		uint32_t cp = utf32Text[i];
		if (cp == '\n') {
			lineWidths.push_back(currentX);
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

		// 次の文字があり、それが改行でなければ文字間隔を加算
		if (i + 1 < utf32Text.size() && utf32Text[i + 1] != '\n') {
			currentX += characterSpacing;
		}
	}
	lineWidths.push_back(currentX);
	maxLineWidth = (std::max)(maxLineWidth, currentX);
	int lineCount = currentLine + 1;

	// フチ取りがある場合、テクスチャ全体も左右・上下に余白を確保する
	int totalWidth = maxLineWidth + drawOutlineWidth * 2;
	int totalHeight = lineCount * rowHeight + drawOutlineWidth * 2;

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

		// フチ取り分の文字ごとの拡張サイズ
		int outW = w;
		int outH = h;
		if (drawOutlineWidth > 0) {
			outW += drawOutlineWidth * 2;
			outH += drawOutlineWidth * 2;
		}

		std::vector<uint8_t> mono(w * h, 0);
		stbtt_MakeCodepointBitmap(&font, mono.data(), w, h, w, scale, scale, glyph.codepoint);

		std::vector<uint8_t> finalAlpha(outW * outH, 0);
		std::vector<uint8_t> finalColorType(outW * outH, 0); // 0: 透明, 1: フチ, 2: 本体

		if (drawOutlineWidth > 0) {
			// 1. 周囲を太らせてフチを作成
			for (int y = 0; y < h; ++y) {
				for (int x = 0; x < w; ++x) {
					uint8_t alpha = mono[y * w + x];
					if (alpha > 0) {
						for (int dy = -drawOutlineWidth; dy <= drawOutlineWidth; ++dy) {
							for (int dx = -drawOutlineWidth; dx <= drawOutlineWidth; ++dx) {
								if (dx * dx + dy * dy <= drawOutlineWidth * drawOutlineWidth) {
									int nx = x + drawOutlineWidth + dx;
									int ny = y + drawOutlineWidth + dy;
									if (nx >= 0 && nx < outW && ny >= 0 && ny < outH) {
										size_t idx = ny * outW + nx;
										finalAlpha[idx] = (std::max)(finalAlpha[idx], alpha);
										finalColorType[idx] = (std::max)(finalColorType[idx], (uint8_t)1); // フチ
									}
								}
							}
						}
					}
				}
			}
			// 2. 中央に本体を上書き
			for (int y = 0; y < h; ++y) {
				for (int x = 0; x < w; ++x) {
					uint8_t alpha = mono[y * w + x];
					if (alpha > 0) {
						int nx = x + drawOutlineWidth;
						int ny = y + drawOutlineWidth;
						size_t idx = ny * outW + nx;
						finalAlpha[idx] = (std::max)(finalAlpha[idx], alpha);
						finalColorType[idx] = 2; // 本体
					}
				}
			}
		} else {
			// フチなし時はそのままコピー
			for (int i = 0; i < w * h; ++i) {
				finalAlpha[i] = mono[i];
				if (mono[i] > 0) {
					finalColorType[i] = 2; // 本体
				}
			}
		}

		// アライメント（配置）計算
		int lineXOffset = 0;
		if (alignment == HorizontalAlignment::Center) {
			lineXOffset = (maxLineWidth - lineWidths[glyph.lineIndex]) / 2;
		} else if (alignment == HorizontalAlignment::Right) {
			lineXOffset = maxLineWidth - lineWidths[glyph.lineIndex];
		}

		// 行の位置とベースライン、アライメント、全体パディングを加味して配置先のX, Y開始位置を決定する
		int startX = drawOutlineWidth + lineXOffset + glyph.xOffset + glyph.leftSideBearing;
		int startY = drawOutlineWidth + glyph.lineIndex * rowHeight + baseline + glyph.y0;

		// フチ取りがある場合はさらにアウトラインパディング分ずらす
		if (drawOutlineWidth > 0) {
			startX -= drawOutlineWidth;
			startY -= drawOutlineWidth;
		}

		for (int y = 0; y < outH; ++y) {
			int destY = startY + y;
			if (destY < 0 || destY >= totalHeight) continue;

			for (int x = 0; x < outW; ++x) {
				int destX = startX + x;
				if (destX < 0 || destX >= totalWidth) continue;

				size_t srcIdx = y * outW + x;
				uint8_t type = finalColorType[srcIdx];
				if (type == 0) continue;

				uint8_t alpha = finalAlpha[srcIdx];
				size_t destIdx = (destY * totalWidth + destX) * 4;

				Vector4 color = (type == 2) ? textColor : outlineColor;
				
				// アルファブレンド（重なる部分のブレンド）
				float srcA = (alpha / 255.0f) * color.w;
				float destA = rgba[destIdx + 3] / 255.0f;
				
				// 簡易アルファ合成
				float outA = srcA + destA * (1.0f - srcA);
				if (outA > 0.0f) {
					rgba[destIdx + 0] = static_cast<uint8_t>((color.x * 255.0f * srcA + rgba[destIdx + 0] * destA * (1.0f - srcA)) / outA);
					rgba[destIdx + 1] = static_cast<uint8_t>((color.y * 255.0f * srcA + rgba[destIdx + 1] * destA * (1.0f - srcA)) / outA);
					rgba[destIdx + 2] = static_cast<uint8_t>((color.z * 255.0f * srcA + rgba[destIdx + 2] * destA * (1.0f - srcA)) / outA);
					rgba[destIdx + 3] = static_cast<uint8_t>(outA * 255.0f);
				}
			}
		}
	}

	// 3. テクスチャアセットの作成または再利用
	DxManager* dxm = DxManager::GetInstance();
	if (!dxm) {
		Console::LogError("[FontRasterizer] DxManager instance is null");
		return false;
	}

	Asset::Texture* existingTexture = assetCollection->GetTexture(texturePath);
	if (existingTexture) {
		existingTexture->RecreateFromPixels(rgba.data(), totalWidth, totalHeight, dxm->GetDxDevice(), dxm->GetDxSRVHeap(), dxm->GetDxCommand());
		return true;
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
