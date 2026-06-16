#pragma once

/// externals
#include <mono/jit/jit.h>
#include <nlohmann/json_fwd.hpp>

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Asset/Assets/Material/Material.h"
#include "Engine/Graphics/Pipelines/Render/Mesh/MeshRenderingPipeline.h"
#include "Engine/Graphics/Buffer/Data/GPUMaterial.h"
#include "Engine/Graphics/Buffer/Data/UVTransform.h"


namespace ONEngine {
class EntityComponentSystem;
class MeshRenderer;
}

namespace ONEngine::Asset {
class AssetCollection;
}



namespace ONEngine {

namespace ComponentDebug {
/// @brief MeshRendererのデバッグ表示
void MeshRendererDebug(MeshRenderer* mr, Asset::AssetCollection* assetCollection);
}

/// Json変換
void from_json(const nlohmann::json& j, MeshRenderer& mr);
void to_json(nlohmann::json& j, const MeshRenderer& mr);


/// @brief 描画の優先順位
enum class RenderQueue : uint32_t {
	Background = 0,
	Telegraph  = 1,
	Default    = 2,
};

/// ///////////////////////////////////////////////////
/// mesh描画クラス
/// ///////////////////////////////////////////////////
class MeshRenderer : public IRenderComponent {
	friend class AnimationPlayer;
	/// friend methods
	friend void ComponentDebug::MeshRendererDebug(MeshRenderer* mr, Asset::AssetCollection* assetCollection);
	friend void from_json(const nlohmann::json& j, MeshRenderer& mr);
	friend void to_json(nlohmann::json& j, const MeshRenderer& mr);

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	MeshRenderer();
	~MeshRenderer();

	/// @brief 描画のために必要なデータを設定する
	void SetupRenderData(Asset::AssetCollection* assetCollection);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::string meshPath_;

	GPUMaterial gpuMaterial_;
	Asset::Material material_;

	RenderQueue renderQueue_ = RenderQueue::Default;

public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	/// @brief 描画するレイヤーの設定
	void SetRenderQueue(RenderQueue queue);

	/// @brief 描画レイヤーの取得
	RenderQueue GetRenderQueue() const;

	/// @brief 描画するmeshの file pathを設定
	/// @param path .slnからの相対パス
	void SetMeshPath(const std::string& path);

	/// @brief 描画する色の設定
	/// @param color RGBA 0.0f ~ 1.0f
	void SetColor(const Vector4& color);

	/// @brief ポストエフェクトのフラグを設定
	/// @param flags ポストエフェクトのフラグ
	void SetPostEffectFlags(uint32_t flags);

	/// @brief UV変形のセット
	void SetUVTransform(const UVTransform& uvTransform);

	/// @brief 描画するmeshの file pathを取得
	/// @return .slnからの相対パス
	const std::string& GetMeshPath() const;

	/// @brief 色の取得
	/// @return RGBA 0.0f ~ 1.0f
	const Vector4& GetColor() const;

	/// @brief GPUで使用するMaterialデータの取得
	const GPUMaterial& GetGpuMaterial() const;

	/// @brief ポストエフェクトのフラグを取得
	/// @return ポストエフェクトのフラグ
	uint32_t GetPostEffectFlags() const;

	/// @brief UV変形の取得
	const UVTransform& GetUVTransform() const;

	/// @brief テクスチャのGuidを返す
	const Guid& GetTextureGuid() const;

	/// @brief アニメーション制御用マテリアルへの参照取得
	Asset::Material& GetMaterialForAnimation() { return material_; }

};



/// ===================================================
/// csで使用するための関数群
/// ===================================================

MonoString* InternalGetMeshName(uint64_t nativeHandle);
void InternalSetMeshName(uint64_t nativeHandle, MonoString* meshName);
Vector4 InternalGetMeshColor(uint64_t nativeHandle);
void InternalSetMeshColor(uint64_t nativeHandle, Vector4 color);
uint32_t InternalGetPostEffectFlags(uint64_t nativeHandle);
void InternalSetPostEffectFlags(uint64_t nativeHandle, uint32_t flags);
uint32_t InternalGetRenderQueue(uint64_t nativeHandle);
void InternalSetRenderQueue(uint64_t nativeHandle, uint32_t queue);

} /// ONEngine
