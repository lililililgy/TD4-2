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

	ImGui::Unindent(indentValue);
}

void ONEngine::to_json(nlohmann::json& j, const TextRenderer& tr) {
	j = nlohmann::json{
		{ "type", "TextRenderer" },
		{ "enable", tr.enable },
		{ "text", tr.text_ },
		{ "fontPath", tr.fontPath_ },
		{ "fontSize", tr.fontSize_ },
		{ "color", tr.material_.baseColor }
	};
}

void ONEngine::from_json(const nlohmann::json& j, TextRenderer& tr) {
	tr.enable = j.value("enable", static_cast<int>(true));
	tr.text_ = j.value("text", "");
	tr.fontPath_ = j.value("fontPath", "./Assets/Fonts/MPLUSRounded1c-Black.ttf");
	tr.fontSize_ = j.value("fontSize", 32);
	tr.material_.baseColor = j.value("color", Vector4::White);
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

	// フォント画像を作成
	if (FontRasterizer::GenerateTexture(text_, fontPath_, fontSize_, dynamicTexturePath_)) {
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
