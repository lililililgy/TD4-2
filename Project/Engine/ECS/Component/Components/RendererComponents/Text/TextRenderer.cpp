#include "TextRenderer.h"
#include <imgui.h>
#include <format>

/// engine
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Core/Utility/Font/FontRasterizer.h"

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
		{ "shadowOffset", tr.shadowOffset_ }
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
	dynamicTexturePath_ = std::format("dynamic://TextRenderer_{}", reinterpret_cast<uintptr_t>(this));
}

TextRenderer::~TextRenderer() {
	// アセットのクリーンアップ
}

void TextRenderer::UpdateTextTexture() {
	if (!isDirty_) return;

	if (text_.empty()) {
		isDirty_ = false;
		return;
	}

	// フォント画像を作成
	if (FontRasterizer::GenerateTexture(text_, fontPath_, fontSize_, dynamicTexturePath_, horizontalAlignment_, material_.baseColor, outlineColor_, outlineWidth_)) {
		auto* assetCollection = Asset::AssetCollection::GetInstance();
		auto* texture = assetCollection->GetTexture(dynamicTexturePath_);
		if (texture) {
			material_.SetBaseTextureGuid(texture->guid);
		}
	}
	isDirty_ = false;
}

void TextRenderer::RenderingSetup(Asset::AssetCollection* assetCollection) {
	gpuMaterial_.baseColor = material_.baseColor;
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
	material_.baseColor = color;
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

const Vector4& TextRenderer::GetColor() const {
	return material_.baseColor;
}

Vector2 TextRenderer::GetTextureSize(Asset::AssetCollection* assetCollection) const {
	if (material_.HasBaseTexture()) {
		Asset::Texture* texture = assetCollection->GetTextureFromGuid(material_.GetBaseTextureGuid());
		if (texture) {
			return texture->GetTextureSize();
		}
	}
	return Vector2(0.0f, 0.0f);
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
