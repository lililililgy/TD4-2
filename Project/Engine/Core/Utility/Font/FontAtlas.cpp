#include "FontAtlas.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Core/Utility/Tools/Log.h"
#include <fstream>
#include <algorithm>
#include <cmath>

namespace ONEngine {

FontAtlas::FontAtlas(const std::string& fontPath, int fontSize, int outlineWidth)
	: fontPath_(fontPath), fontSize_(fontSize), outlineWidth_(outlineWidth)
{
	// アトラス用ユニークテクスチャパスを生成
	texturePath_ = std::format("dynamic://FontAtlas_{}_{}_{}", 
		fontSize_, outlineWidth_, GenerateGuid().ToString());
}

FontAtlas::~FontAtlas() {
	// アセットのクリーンアップはアセットコレクションに委ねる
}

bool FontAtlas::Initialize() {
	// 1. TTFファイルの読み込み
	std::ifstream file(fontPath_, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		Console::LogError("[FontAtlas] Failed to open font file: " + fontPath_);
		return false;
	}
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	fontData_.resize(size);
	if (!file.read(reinterpret_cast<char*>(fontData_.data()), size)) {
		Console::LogError("[FontAtlas] Failed to read font file data: " + fontPath_);
		return false;
	}

	// 2. stb_truetype の初期化
	if (!stbtt_InitFont(&fontInfo_, fontData_.data(), 0)) {
		Console::LogError("[FontAtlas] Failed to initialize stb_truetype for: " + fontPath_);
		return false;
	}

	// 3. ピクセルバッファの初期化 (RGBA: 4バイト/ピクセル)
	pixelBuffer_.resize(atlasWidth_ * atlasHeight_ * 4, 0);

	// 4. アセットコレクションへの空テクスチャの登録
	auto* assetCollection = Asset::AssetCollection::GetInstance();
	DxManager* dxm = DxManager::GetInstance();
	if (!dxm) return false;

	Asset::Texture newTex;
	newTex.guid = GenerateGuid();
	// 1x1 の透明ピクセルで初期作成し、アセットへ追加
	uint8_t dummy[4] = { 0, 0, 0, 0 };
	newTex.RecreateFromPixels(dummy, 1, 1, dxm->GetDxDevice(), dxm->GetDxSRVHeap(), dxm->GetDxCommand());
	
	assetCollection->AddAsset<Asset::Texture>(texturePath_, std::move(newTex));
	texture_ = assetCollection->GetTexture(texturePath_);

	isDirty_ = true;
	UpdateGpuTexture();

	return true;
}

const GlyphData* FontAtlas::GetGlyph(uint32_t codepoint) {
	auto it = glyphs_.find(codepoint);
	if (it != glyphs_.end()) {
		return &it->second;
	}

	// キャッシュに無い場合はオンデマンドで追加
	GlyphData newData;
	if (AddGlyphToAtlas(codepoint, newData)) {
		glyphs_[codepoint] = newData;
		isDirty_ = true;
		return &glyphs_[codepoint];
	}

	return nullptr;
}

void FontAtlas::UpdateGpuTexture() {
	if (!isDirty_ || !texture_) return;

	DxManager* dxm = DxManager::GetInstance();
	if (!dxm) return;

	// アトラス全体をGPUへ転送
	texture_->RecreateFromPixels(
		pixelBuffer_.data(), 
		atlasWidth_, 
		atlasHeight_, 
		dxm->GetDxDevice(), 
		dxm->GetDxSRVHeap(), 
		dxm->GetDxCommand()
	);

	isDirty_ = false;
}

float FontAtlas::GetScale() const {
	return stbtt_ScaleForPixelHeight(&fontInfo_, static_cast<float>(fontSize_));
}

void FontAtlas::GetFontVMetrics(int* ascent, int* descent, int* lineGap) const {
	stbtt_GetFontVMetrics(&fontInfo_, ascent, descent, lineGap);
}

bool FontAtlas::AddGlyphToAtlas(uint32_t codepoint, GlyphData& outGlyph) {
	float scale = GetScale();

	// 1. グリフ情報の取得
	int advance, lsb;
	stbtt_GetCodepointHMetrics(&fontInfo_, codepoint, &advance, &lsb);
	
	int x0, y0, x1, y1;
	stbtt_GetCodepointBitmapBox(&fontInfo_, codepoint, scale, scale, &x0, &y0, &x1, &y1);

	int bodyW = x1 - x0;
	int bodyH = y1 - y0;

	int pad = (outlineWidth_ > 0) ? outlineWidth_ : 0;
	int glyphW = bodyW + pad * 2;
	int glyphH = bodyH + pad * 2;

	// 2. 空き領域パッキング (Shelf packing)
	if (currentX_ + glyphW + padding_ > atlasWidth_) {
		currentX_ = 0;
		currentY_ += maxRowHeight_ + padding_;
		maxRowHeight_ = 0;
	}

	if (currentY_ + glyphH + padding_ > atlasHeight_) {
		Console::LogError(std::format("[FontAtlas] Atlas is full! Font: {}, Size: {}", fontPath_, fontSize_));
		return false;
	}

	// パッキング位置の決定
	int posX = currentX_;
	int posY = currentY_;

	currentX_ += glyphW + padding_;
	maxRowHeight_ = (std::max)(maxRowHeight_, glyphH);

	// 3. 文字本体のラスタライズ
	std::vector<uint8_t> bodyAlpha(bodyW * bodyH, 0);
	if (bodyW > 0 && bodyH > 0) {
		stbtt_MakeCodepointBitmap(&fontInfo_, bodyAlpha.data(), bodyW, bodyH, bodyW, scale, scale, codepoint);
	}

	// 4. フチ（アウトライン）アルファの生成 (Dilation 膨張アルゴリズム)
	std::vector<uint8_t> outlineAlpha(glyphW * glyphH, 0);
	if (pad > 0 && bodyW > 0 && bodyH > 0) {
		for (int y = 0; y < glyphH; ++y) {
			for (int x = 0; x < glyphW; ++x) {
				uint8_t maxAlpha = 0;
				for (int dy = -pad; dy <= pad; ++dy) {
					for (int dx = -pad; dx <= pad; ++dx) {
						if (dx * dx + dy * dy <= pad * pad) {
							int srcX = x - pad + dx;
							int srcY = y - pad + dy;
							if (srcX >= 0 && srcX < bodyW && srcY >= 0 && srcY < bodyH) {
								maxAlpha = (std::max)(maxAlpha, bodyAlpha[srcY * bodyW + srcX]);
							}
						}
					}
				}
				outlineAlpha[y * glyphW + x] = maxAlpha;
			}
		}
	}

	// 5. アトラスピクセルバッファへの書き込み (RGBA)
	for (int dy = 0; dy < glyphH; ++dy) {
		int destY = posY + dy;
		for (int dx = 0; dx < glyphW; ++dx) {
			int destX = posX + dx;

			// 本体アルファの位置
			int srcBodyX = dx - pad;
			int srcBodyY = dy - pad;
			uint8_t rVal = 0; // 本体アルファ
			if (srcBodyX >= 0 && srcBodyX < bodyW && srcBodyY >= 0 && srcBodyY < bodyH) {
				rVal = bodyAlpha[srcBodyY * bodyW + srcBodyX];
			}

			uint8_t gVal = outlineAlpha[dy * glyphW + dx]; // フチアルファ

			size_t destIdx = (destY * atlasWidth_ + destX) * 4;
			pixelBuffer_[destIdx + 0] = rVal;   // R: 本体
			pixelBuffer_[destIdx + 1] = gVal;   // G: フチ
			pixelBuffer_[destIdx + 2] = 0;      // B
			pixelBuffer_[destIdx + 3] = 255;    // A: 不透明度
		}
	}

	// 6. グリフデータの作成
	outGlyph.codepoint = codepoint;
	outGlyph.uvMin = Vector2(static_cast<float>(posX) / atlasWidth_, static_cast<float>(posY) / atlasHeight_);
	outGlyph.uvMax = Vector2(static_cast<float>(posX + glyphW) / atlasWidth_, static_cast<float>(posY + glyphH) / atlasHeight_);
	outGlyph.size = Vector2(static_cast<float>(glyphW), static_cast<float>(glyphH));
	outGlyph.bearing = Vector2(static_cast<float>(lsb * scale) - pad, static_cast<float>(y0) - pad);
	outGlyph.advance = static_cast<float>(advance * scale);

	return true;
}

} // namespace ONEngine
