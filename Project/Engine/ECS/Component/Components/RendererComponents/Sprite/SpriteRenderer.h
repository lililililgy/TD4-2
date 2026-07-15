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


namespace ONEngine {
class SpriteRenderer;
}

namespace ONEngine::Asset {
class AssetCollection;
}


namespace ONEngine {


namespace ComponentDebug {
void SpriteDebug(SpriteRenderer* sr, Asset::AssetCollection* assetCollection);
}

/// json serialize
void to_json(nlohmann::json& j, const SpriteRenderer& sr);
void from_json(const nlohmann::json& j, SpriteRenderer& sr);

/// ///////////////////////////////////////////////////
/// sprite描画クラス
/// ///////////////////////////////////////////////////
class SpriteRenderer final : public IComponent {
	friend class SpriteUpdateSystem;
	friend class AnimationPlayer;

	friend void ComponentDebug::SpriteDebug(SpriteRenderer* sr, Asset::AssetCollection* assetCollection);
	friend void to_json(nlohmann::json& j, const SpriteRenderer& sr);
	friend void from_json(const nlohmann::json& j, SpriteRenderer& sr);
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	SpriteRenderer();
	~SpriteRenderer();

	/// @brief 描画用データのセットアップ
	void RenderingSetup(Asset::AssetCollection* assetCollection);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	GPUMaterial gpuMaterial_;
	Asset::Material material_;

public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	/// ----- setter ----- ///
	void SetColor(const Vector4& color);
	void SetUVTransform(const UVTransform& uvTransform);
	void SetPixelPerfect(bool enable);
	void SetPostEffectFlags(uint32_t flags);

	/// ----- getter ----- ///
	const Vector4& GetColor() const;

	const GPUMaterial& GetGpuMaterial() const;

	const UVTransform& GetUVTransform() const;

	Vector2 GetTextureSize(Asset::AssetCollection* assetCollection) const;

	bool IsPixelPerfect() const;

	uint32_t GetPostEffectFlags() const;

	/// @brief アニメーション制御用マテリアルへの参照取得
	Asset::Material& GetMaterialForAnimation() { return material_; }

private:
	bool isPixelPerfect_ = false;

};


/// ===================================================
/// csで使用するための関数群
/// ===================================================

namespace MonoInternalMethods {
	/// ここでコメントアウトしているのは今後実装する
	//MonoString* InternalGetTexturePath(uint64_t nativeHandle);
	//void InternalSetTexturePath(uint64_t nativeHandle, MonoString* path);

	Vector4 InternalGetColor(uint64_t nativeHandle);
	void InternalSetColor(uint64_t nativeHandle, Vector4 color);

	Vector2 InternalGetTextureSize(uint64_t nativeHandle);

	bool InternalGetPixelPerfect(uint64_t nativeHandle);
	void InternalSetPixelPerfect(uint64_t nativeHandle, bool enable);
}

} /// ONEngine
