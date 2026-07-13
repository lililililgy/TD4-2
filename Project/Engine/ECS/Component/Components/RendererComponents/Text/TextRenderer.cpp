#include "TextRenderer.h"
#include <imgui.h>
#include <format>

/// engine
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Core/Utility/Font/FontRasterizer.h"
#include "Engine/Core/Utility/Font/FontAtlasManager.h"
#include "Engine/Asset/Guid/Guid.h"

// Monoのヘッダ
#include <mono/jit/jit.h>
#include <mono/metadata/object.h>
#include <mono/metadata/appdomain.h>
#include "Engine/Script/MonoScriptEngine.h"

using namespace ONEngine;

void ComponentDebug::TextDebug(TextRenderer* tr, Asset::AssetCollection* assetCollection) {
	if (!tr) return;

	float indentValue = 1.8f;
	ImGui::Indent(indentValue);

	// 簡単なデバッグUI
	char textBuf[256];
	strncpy(textBuf, tr->text_.c_str(), sizeof(textBuf));
	if (ImGui::InputText("Text", textBuf, sizeof(textBuf))) {
		tr->SetText(textBuf);
	}

	char fontPathBuf[256];
	strncpy(fontPathBuf, tr->fontPath_.c_str(), sizeof(fontPathBuf));
	if (ImGui::InputText("Font Path", fontPathBuf, sizeof(fontPathBuf))) {
		tr->SetFontPath(fontPathBuf);
	}

	int size = tr->fontSize_;
	if (ImGui::InputInt("Font Size", &size)) {
		tr->SetFontSize(size);
	}

	Vector4 col = tr->GetColor();
	if (ImGui::ColorEdit4("Color", &col.x)) {
		tr->SetColor(col);
	}

	const char* hAlignments[] = { "Left", "Center", "Right" };
	int hAlign = static_cast<int>(tr->GetHorizontalAlignment());
	if (ImGui::Combo("Horizontal Align", &hAlign, hAlignments, IM_ARRAYSIZE(hAlignments))) {
		tr->SetHorizontalAlignment(static_cast<HorizontalAlignment>(hAlign));
	}

	const char* vAlignments[] = { "Top", "Middle", "Bottom" };
	int vAlign = static_cast<int>(tr->GetVerticalAlignment());
	if (ImGui::Combo("Vertical Align", &vAlign, vAlignments, IM_ARRAYSIZE(vAlignments))) {
		tr->SetVerticalAlignment(static_cast<VerticalAlignment>(vAlign));
	}

	Vector4 oCol = tr->GetOutlineColor();
	if (ImGui::ColorEdit4("Outline Color", &oCol.x)) {
		tr->SetOutlineColor(oCol);
	}
	int oWidth = tr->GetOutlineWidth();
	if (ImGui::InputInt("Outline Width", &oWidth)) {
		tr->SetOutlineWidth(oWidth);
	}

	Vector4 sCol = tr->GetShadowColor();
	if (ImGui::ColorEdit4("Shadow Color", &sCol.x)) {
		tr->SetShadowColor(sCol);
	}
	Vector2 sOffset = tr->GetShadowOffset();
	if (ImGui::DragFloat2("Shadow Offset", &sOffset.x, 0.1f)) {
		tr->SetShadowOffset(sOffset);
	}

	int charSpacing = tr->GetCharacterSpacing();
	if (ImGui::InputInt("Character Spacing", &charSpacing)) {
		tr->SetCharacterSpacing(charSpacing);
	}

	float lineSpacing = tr->GetLineSpacing();
	if (ImGui::DragFloat("Line Spacing", &lineSpacing, 0.05f, 0.1f, 10.0f)) {
		tr->SetLineSpacing(lineSpacing);
	}

	ImGui::Unindent(indentValue);
}

void ONEngine::to_json(nlohmann::json& j, const TextRenderer& tr) {
	j = nlohmann::json{
		{ "type", "TextRenderer" },
		{ "enable", tr.enable },
		{ "text", tr.text_ },
		{ "fontPath", tr.fontPath_ },
		{ "fontSize", tr.fontSize_ },
		{ "color", tr.material_.baseColor },
		{ "horizontalAlignment", static_cast<int>(tr.horizontalAlignment_) },
		{ "verticalAlignment", static_cast<int>(tr.verticalAlignment_) },
		{ "outlineColor", tr.outlineColor_ },
		{ "outlineWidth", tr.outlineWidth_ },
		{ "shadowColor", tr.shadowColor_ },
		{ "shadowOffset", tr.shadowOffset_ },
		{ "characterSpacing", tr.characterSpacing_ },
		{ "lineSpacing", tr.lineSpacing_ }
	};
}

void ONEngine::from_json(const nlohmann::json& j, TextRenderer& tr) {
	tr.enable = j.value("enable", static_cast<int>(true));
	tr.text_ = j.value("text", "");
	tr.fontPath_ = j.value("fontPath", "./Assets/Fonts/MPLUSRounded1c-Black.ttf");
	tr.fontSize_ = j.value("fontSize", 32);
	tr.material_.baseColor = j.value("color", Vector4::White);
	tr.horizontalAlignment_ = static_cast<HorizontalAlignment>(j.value("horizontalAlignment", 0));
	tr.verticalAlignment_ = static_cast<VerticalAlignment>(j.value("verticalAlignment", 0));
	tr.outlineColor_ = j.value("outlineColor", Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	tr.outlineWidth_ = j.value("outlineWidth", 0);
	tr.shadowColor_ = j.value("shadowColor", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
	tr.shadowOffset_ = j.value("shadowOffset", Vector2(2.0f, -2.0f));
	tr.characterSpacing_ = j.value("characterSpacing", 0);
	tr.lineSpacing_ = j.value("lineSpacing", 1.0f);
	tr.MarkDirty();
}

TextRenderer::TextRenderer() {
	gpuMaterial_.baseColor = Vector4::White;
	gpuMaterial_.entityId = 0;
	gpuMaterial_.baseTextureId = 0;
	gpuMaterial_.uvTransform = UVTransform();
	gpuMaterial_.postEffectFlags = 0;

	material_.baseColor = Vector4::White;
	material_.postEffectFlags = 0;

	// インスタンス専用のユニークなアセット名を設定
	dynamicTexturePath_ = std::format("dynamic://TextRenderer_{}", GenerateGuid().ToString());
}

TextRenderer::~TextRenderer() {
	// アセットのクリーンアップ
}

TextRenderer::TextRenderer(const TextRenderer& other)
	: IComponent(other),
	  gpuMaterial_(other.gpuMaterial_),
	  material_(other.material_),
	  text_(other.text_),
	  fontPath_(other.fontPath_),
	  fontSize_(other.fontSize_),
	  horizontalAlignment_(other.horizontalAlignment_),
	  verticalAlignment_(other.verticalAlignment_),
	  outlineColor_(other.outlineColor_),
	  outlineWidth_(other.outlineWidth_),
	  shadowColor_(other.shadowColor_),
	  shadowOffset_(other.shadowOffset_),
	  characterSpacing_(other.characterSpacing_),
	  lineSpacing_(other.lineSpacing_),
	  isDirty_(true)
{
	// コピー先でユニークなテクスチャパスを新しく生成する
	dynamicTexturePath_ = std::format("dynamic://TextRenderer_{}", GenerateGuid().ToString());
}

TextRenderer& TextRenderer::operator=(const TextRenderer& other) {
	if (this != &other) {
		IComponent::operator=(other);
		gpuMaterial_ = other.gpuMaterial_;
		material_ = other.material_;
		text_ = other.text_;
		fontPath_ = other.fontPath_;
		fontSize_ = other.fontSize_;
		horizontalAlignment_ = other.horizontalAlignment_;
		verticalAlignment_ = other.verticalAlignment_;
		outlineColor_ = other.outlineColor_;
		outlineWidth_ = other.outlineWidth_;
		shadowColor_ = other.shadowColor_;
		shadowOffset_ = other.shadowOffset_;
		characterSpacing_ = other.characterSpacing_;
		lineSpacing_ = other.lineSpacing_;
		isDirty_ = true;
		// パスは自分自身のユニークなものを維持する
	}
	return *this;
}

namespace {
	// UTF-8 を UTF-32 にデコードするヘルパー
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
			else { i += 1; continue; }
			utf32.push_back(cp);
		}
		return utf32;
	}
}

void TextRenderer::UpdateTextTexture() {
	if (!isDirty_) return;

	if (text_.empty()) {
		vertices_.clear();
		textBounds_ = Vector2(0.0f, 0.0f);
		isDirty_ = false;
		return;
	}

	FontAtlas* atlas = FontAtlasManager::GetInstance().GetOrCreateAtlas(fontPath_, fontSize_, outlineWidth_);
	if (atlas) {
		atlas->UpdateGpuTexture();
		auto* assetCollection = Asset::AssetCollection::GetInstance();
		auto* texture = assetCollection->GetTexture(atlas->GetTexturePath());
		if (texture) {
			material_.SetBaseTextureGuid(texture->guid);
		}
	}

	AssembleVertices();
	isDirty_ = false;
}

void TextRenderer::AssembleVertices() {
	vertices_.clear();

	FontAtlas* atlas = FontAtlasManager::GetInstance().GetOrCreateAtlas(fontPath_, fontSize_, outlineWidth_);
	if (!atlas) {
		textBounds_ = Vector2(0.0f, 0.0f);
		return;
	}

	std::vector<uint32_t> utf32Text = Utf8ToUtf32(text_);
	if (utf32Text.empty()) {
		textBounds_ = Vector2(0.0f, 0.0f);
		return;
	}

	// 1. 各行の幅とグリフのレイアウトを事前計算
	int ascent, descent, lineGap;
	atlas->GetFontVMetrics(&ascent, &descent, &lineGap);
	float scale = atlas->GetScale();
	float rowHeight = (ascent - descent + lineGap) * scale * lineSpacing_;

	struct LineInfo {
		int startGlyphIdx;
		int glyphCount;
		float width;
	};
	std::vector<LineInfo> lines;
	
	std::vector<const GlyphData*> glyphs;
	glyphs.reserve(utf32Text.size());

	float currentX = 0.0f;
	int lineStartIdx = 0;

	for (size_t i = 0; i < utf32Text.size(); ++i) {
		uint32_t cp = utf32Text[i];
		if (cp == '\n') {
			LineInfo li;
			li.startGlyphIdx = lineStartIdx;
			li.glyphCount = static_cast<int>(glyphs.size()) - lineStartIdx;
			li.width = currentX;
			lines.push_back(li);

			currentX = 0.0f;
			lineStartIdx = static_cast<int>(glyphs.size());
			continue;
		}

		const GlyphData* glyph = atlas->GetGlyph(cp);
		if (glyph) {
			glyphs.push_back(glyph);
			currentX += glyph->advance;
			
			// 次の文字があり、それが改行でなければ文字間隔を加える
			if (i + 1 < utf32Text.size() && utf32Text[i + 1] != '\n') {
				currentX += characterSpacing_;
			}
		}
	}
	// 最終行の追加
	LineInfo li;
	li.startGlyphIdx = lineStartIdx;
	li.glyphCount = static_cast<int>(glyphs.size()) - lineStartIdx;
	li.width = currentX;
	lines.push_back(li);

	// 全体の幅と高さ（ピクセル単位）を計算してキャッシュ
	float maxLineWidth = 0.0f;
	for (const auto& line : lines) {
		maxLineWidth = (std::max)(maxLineWidth, line.width);
	}
	float totalHeight = lines.size() * rowHeight;
	textBounds_ = Vector2(maxLineWidth, totalHeight);

	// 2. 頂点データのアセンブル (1ピクセル = 0.002 ワールド単位)
	float unitScale = 0.002f;

	for (size_t lineIdx = 0; lineIdx < lines.size(); ++lineIdx) {
		const auto& line = lines[lineIdx];
		if (line.glyphCount == 0) continue;

		// アライメントに応じたXの開始オフセット
		float startX = 0.0f;
		if (horizontalAlignment_ == HorizontalAlignment::Center) {
			startX = -line.width * 0.5f;
		} else if (horizontalAlignment_ == HorizontalAlignment::Right) {
			startX = -line.width;
		}

		// アライメントに応じたYの開始オフセット
		float startY = 0.0f;
		if (verticalAlignment_ == VerticalAlignment::Top) {
			startY = 0.0f;
		} else if (verticalAlignment_ == VerticalAlignment::Middle) {
			startY = totalHeight * 0.5f;
		} else if (verticalAlignment_ == VerticalAlignment::Bottom) {
			startY = totalHeight;
		}

		// 行のY座標 (上方向がプラスなので、行を下に進むほどマイナス)
		float currentLineY = startY - (lineIdx * rowHeight);

		float cursorX = startX;

		for (int gIdx = 0; gIdx < line.glyphCount; ++gIdx) {
			const GlyphData* glyph = glyphs[line.startGlyphIdx + gIdx];

			// 文字の Quad 座標 (ワールド空間へのスケーリング適用前)
			float x0 = cursorX + glyph->bearing.x;
			float x1 = x0 + glyph->size.x;
			float y0 = currentLineY - glyph->bearing.y;
			float y1 = y0 - glyph->size.y; // 下方向へ延びる

			// 1ピクセル＝0.002f としてワールド空間にスケール変換
			x0 *= unitScale;
			x1 *= unitScale;
			y0 *= unitScale;
			y1 *= unitScale;

			// UV 座標
			Vector2 uvLT = glyph->uvMin;
			Vector2 uvRB = glyph->uvMax;
			Vector2 uvRT = Vector2(uvRB.x, uvLT.y);
			Vector2 uvLB = Vector2(uvLT.x, uvRB.y);

			// 6つの頂点を時計回り(CullMode: Back)でトライアングルリストとして格納
			// Tri 1: LT (0), RT (1), LB (2)
			TextVertex v0{ Vector3(x0, y0, 0.0f), uvLT };
			TextVertex v1{ Vector3(x1, y0, 0.0f), uvRT };
			TextVertex v2{ Vector3(x0, y1, 0.0f), uvLB };

			// Tri 2: RT (1), RB (3), LB (2)
			TextVertex v3{ Vector3(x1, y1, 0.0f), uvRB };

			vertices_.push_back(v0);
			vertices_.push_back(v1);
			vertices_.push_back(v2);

			vertices_.push_back(v1);
			vertices_.push_back(v3);
			vertices_.push_back(v2);

			// カーソルを次の文字へ
			cursorX += glyph->advance;
			if (gIdx + 1 < line.glyphCount) {
				cursorX += characterSpacing_;
			}
		}
	}
}

void TextRenderer::RenderingSetup(Asset::AssetCollection* assetCollection) {
	gpuMaterial_.baseColor = material_.baseColor;
	gpuMaterial_.outlineColor = outlineColor_;
	gpuMaterial_.postEffectFlags = material_.postEffectFlags;

	if (material_.HasBaseTexture() && material_.GetBaseTextureGuid().CheckValid()) {
		int32_t textureIndex = assetCollection->GetTextureIndexFromGuid(material_.GetBaseTextureGuid());
		if (textureIndex != -1) {
			gpuMaterial_.baseTextureId = textureIndex;
		} else {
			gpuMaterial_.baseTextureId = 0;
		}
	} else {
		gpuMaterial_.baseTextureId = 0;
	}

	gpuMaterial_.uvTransform = material_.uvTransform;
	gpuMaterial_.entityId = owner_ ? static_cast<int32_t>(owner_->GetId()) : 0;
}

void TextRenderer::SetText(const std::string& text) {
	if (text_ != text) {
		text_ = text;
		MarkDirty();
	}
}

void TextRenderer::SetFontPath(const std::string& fontPath) {
	if (fontPath_ != fontPath) {
		fontPath_ = fontPath;
		MarkDirty();
	}
}

void TextRenderer::SetFontSize(int size) {
	if (fontSize_ != size) {
		fontSize_ = size;
		MarkDirty();
	}
}

void TextRenderer::SetColor(const Vector4& color) {
	if (material_.baseColor.x != color.x || material_.baseColor.y != color.y ||
		material_.baseColor.z != color.z || material_.baseColor.w != color.w) {
		material_.baseColor = color;
		MarkDirty();
	}
}

void TextRenderer::SetUVTransform(const UVTransform& uvTransform) {
	material_.uvTransform = uvTransform;
}

void TextRenderer::SetHorizontalAlignment(HorizontalAlignment alignment) {
	if (horizontalAlignment_ != alignment) {
		horizontalAlignment_ = alignment;
		MarkDirty();
	}
}

void TextRenderer::SetVerticalAlignment(VerticalAlignment alignment) {
	if (verticalAlignment_ != alignment) {
		verticalAlignment_ = alignment;
	}
}

void TextRenderer::SetOutlineColor(const Vector4& color) {
	outlineColor_ = color;
	MarkDirty();
}

void TextRenderer::SetOutlineWidth(int width) {
	if (outlineWidth_ != width) {
		outlineWidth_ = width;
		MarkDirty();
	}
}

void TextRenderer::SetShadowColor(const Vector4& color) {
	shadowColor_ = color;
}

void TextRenderer::SetShadowOffset(const Vector2& offset) {
	shadowOffset_ = offset;
}

void TextRenderer::SetCharacterSpacing(int spacing) {
	if (characterSpacing_ != spacing) {
		characterSpacing_ = spacing;
		MarkDirty();
	}
}

void TextRenderer::SetLineSpacing(float spacing) {
	if (lineSpacing_ != spacing) {
		lineSpacing_ = spacing;
		MarkDirty();
	}
}

const Vector4& TextRenderer::GetColor() const {
	return material_.baseColor;
}

Vector2 TextRenderer::GetTextureSize(Asset::AssetCollection* /*assetCollection*/) const {
	return textBounds_;
}

/// ===================================================
/// csで使用するための関数群
/// ===================================================

Vector4 MonoInternalMethods::InternalGetTextColor(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		return tr->GetColor();
	}
	return Vector4();
}

void MonoInternalMethods::InternalSetTextColor(uint64_t nativeHandle, Vector4 color) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		tr->SetColor(color);
	}
}

MonoString* MonoInternalMethods::InternalGetTextText(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		return mono_string_new(MonoScriptEngine::GetInstance().Domain(), tr->GetText().c_str());
	}
	return mono_string_new(MonoScriptEngine::GetInstance().Domain(), "");
}

void MonoInternalMethods::InternalSetTextText(uint64_t nativeHandle, MonoString* text) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr && text) {
		char* utf8Text = mono_string_to_utf8(text);
		tr->SetText(utf8Text);
		mono_free(utf8Text);
	}
}

MonoString* MonoInternalMethods::InternalGetTextFontPath(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		return mono_string_new(MonoScriptEngine::GetInstance().Domain(), tr->GetFontPath().c_str());
	}
	return mono_string_new(MonoScriptEngine::GetInstance().Domain(), "");
}

void MonoInternalMethods::InternalSetTextFontPath(uint64_t nativeHandle, MonoString* fontPath) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr && fontPath) {
		char* utf8Path = mono_string_to_utf8(fontPath);
		tr->SetFontPath(utf8Path);
		mono_free(utf8Path);
	}
}

int MonoInternalMethods::InternalGetTextFontSize(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		return tr->GetFontSize();
	}
	return 0;
}

void MonoInternalMethods::InternalSetTextFontSize(uint64_t nativeHandle, int fontSize) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		tr->SetFontSize(fontSize);
	}
}

int MonoInternalMethods::InternalGetHorizontalAlignment(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		return static_cast<int>(tr->GetHorizontalAlignment());
	}
	return 0;
}

void MonoInternalMethods::InternalSetHorizontalAlignment(uint64_t nativeHandle, int alignment) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		tr->SetHorizontalAlignment(static_cast<HorizontalAlignment>(alignment));
	}
}

int MonoInternalMethods::InternalGetVerticalAlignment(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		return static_cast<int>(tr->GetVerticalAlignment());
	}
	return 0;
}

void MonoInternalMethods::InternalSetVerticalAlignment(uint64_t nativeHandle, int alignment) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		tr->SetVerticalAlignment(static_cast<VerticalAlignment>(alignment));
	}
}

Vector4 MonoInternalMethods::InternalGetOutlineColor(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		return tr->GetOutlineColor();
	}
	return Vector4();
}

void MonoInternalMethods::InternalSetOutlineColor(uint64_t nativeHandle, Vector4 color) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		tr->SetOutlineColor(color);
	}
}

int MonoInternalMethods::InternalGetOutlineWidth(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		return tr->GetOutlineWidth();
	}
	return 0;
}

void MonoInternalMethods::InternalSetOutlineWidth(uint64_t nativeHandle, int width) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		tr->SetOutlineWidth(width);
	}
}

Vector4 MonoInternalMethods::InternalGetShadowColor(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		return tr->GetShadowColor();
	}
	return Vector4();
}

void MonoInternalMethods::InternalSetShadowColor(uint64_t nativeHandle, Vector4 color) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		tr->SetShadowColor(color);
	}
}

Vector2 MonoInternalMethods::InternalGetShadowOffset(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		return tr->GetShadowOffset();
	}
	return Vector2();
}

void MonoInternalMethods::InternalSetShadowOffset(uint64_t nativeHandle, Vector2 offset) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		tr->SetShadowOffset(offset);
	}
}

int MonoInternalMethods::InternalGetCharacterSpacing(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	return tr ? tr->GetCharacterSpacing() : 0;
}

void MonoInternalMethods::InternalSetCharacterSpacing(uint64_t nativeHandle, int spacing) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		tr->SetCharacterSpacing(spacing);
	}
}

float MonoInternalMethods::InternalGetLineSpacing(uint64_t nativeHandle) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	return tr ? tr->GetLineSpacing() : 1.0f;
}

void MonoInternalMethods::InternalSetLineSpacing(uint64_t nativeHandle, float spacing) {
	TextRenderer* tr = reinterpret_cast<TextRenderer*>(nativeHandle);
	if (tr) {
		tr->SetLineSpacing(spacing);
	}
}
