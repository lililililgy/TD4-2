#pragma once

/// std
#include <string>

/// external
#include <nlohmann/json.hpp>

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Graphics/Buffer/Data/GPUMaterial.h"
#include "Engine/Asset/Guid/Guid.h"
#include "Engine/Asset/Assets/Material/Material.h"
#include "Engine/Core/Utility/Math/Vector2.h"
#include "Engine/Graphics/Buffer/Data/UVTransform.h"

// Monoの型前方宣言
extern "C" {
typedef struct _MonoString MonoString;
}

namespace ONEngine {
class TextRenderer;
}

namespace ONEngine::Asset {
class AssetCollection;
}

namespace ONEngine {

namespace ComponentDebug {
void TextDebug(TextRenderer* tr, Asset::AssetCollection* assetCollection);
}

/// json serialize
void to_json(nlohmann::json& j, const TextRenderer& tr);
void from_json(const nlohmann::json& j, TextRenderer& tr);

/// ///////////////////////////////////////////////////
/// Text描画クラス
/// ///////////////////////////////////////////////////
class TextRenderer final : public IComponent {
	friend class TextRenderingPipeline;

	friend void ComponentDebug::TextDebug(TextRenderer* tr, Asset::AssetCollection* assetCollection);
	friend void to_json(nlohmann::json& j, const TextRenderer& tr);
	friend void from_json(const nlohmann::json& j, TextRenderer& tr);
public:
	TextRenderer();
	~TextRenderer();

	/// @brief 描画用データのセットアップ
	void RenderingSetup(Asset::AssetCollection* assetCollection);

	/// @brief テクスチャ更新要求フラグを立てる
	void MarkDirty() { isDirty_ = true; }

private:
	/// @brief 内部テクスチャの更新処理
	void UpdateTextTexture();

private:
	GPUMaterial gpuMaterial_;
	Asset::Material material_;

	std::string text_ = "";
	std::string fontPath_ = "./Assets/Fonts/MPLUSRounded1c-Black.ttf";
	int fontSize_ = 32;

	std::string dynamicTexturePath_ = "";
	bool isDirty_ = true;

public:
	/// ----- setters ----- ///
	void SetText(const std::string& text);
	void SetFontPath(const std::string& fontPath);
	void SetFontSize(int size);
	void SetColor(const Vector4& color);
	void SetUVTransform(const UVTransform& uvTransform);

	/// ----- getters ----- ///
	const std::string& GetText() const { return text_; }
	const std::string& GetFontPath() const { return fontPath_; }
	int GetFontSize() const { return fontSize_; }
	const Vector4& GetColor() const;
	const GPUMaterial& GetGpuMaterial() const { return gpuMaterial_; }
	const UVTransform& GetUVTransform() const { return material_.uvTransform; }
	Vector2 GetTextureSize(Asset::AssetCollection* assetCollection) const;
	Asset::Material& GetMaterialForAnimation() { return material_; }
};

/// C#用
namespace MonoInternalMethods {
	Vector4 InternalGetTextColor(uint64_t nativeHandle);
	void InternalSetTextColor(uint64_t nativeHandle, Vector4 color);

	MonoString* InternalGetTextText(uint64_t nativeHandle);
	void InternalSetTextText(uint64_t nativeHandle, MonoString* text);

	MonoString* InternalGetTextFontPath(uint64_t nativeHandle);
	void InternalSetTextFontPath(uint64_t nativeHandle, MonoString* fontPath);

	int InternalGetTextFontSize(uint64_t nativeHandle);
	void InternalSetTextFontSize(uint64_t nativeHandle, int fontSize);
}

} // namespace ONEngine
