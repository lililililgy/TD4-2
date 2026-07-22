#pragma once

/// std
#include <vector>
#include <memory>
#include <string>

/// engine
#include "RenderTexture.h"

namespace ONEngine {
class DxManager;
class DxCommand;
class DxDSVHeap;
class DxDepthStencil;
}

namespace ONEngine::Asset {
class AssetCollection;
}



/// @brief シーンのレンダリングテクスチャの種類
enum SCENE_RTV {
	SCENE_RTV_COLOR,
	SCENE_RTV_WORLD_POSITION,
	SCENE_RTV_NORMAL,
	SCENE_RTV_FLAGS,
};

/// ///////////////////////////////////////////////////
/// シーンのレンダリングテクスチャ
/// ///////////////////////////////////////////////////
namespace ONEngine {

class SceneRenderTexture final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	SceneRenderTexture();
	~SceneRenderTexture() = default;

	void Initialize(
		const std::string& name, const Vector4& clearColor, const Vector2& textureSize,
		DxManager* dxm, Asset::AssetCollection* assetCollection
	);


	void SetRenderTarget(DxCommand* dxCommand, DxDSVHeap* dxDSVHeap, bool clear = true);
	void SetRenderTargetColorOnly(DxCommand* dxCommand, DxDSVHeap* dxDSVHeap, bool clear = true);
	RenderTexture* GetRenderTexture(size_t index) const { return renderTextures_[index].get(); }

	void SetClearColor(const Vector4& clearColor);

	void CreateBarrierRenderTarget(DxCommand* dxCommand);
	void CreateBarrierPixelShaderResource(DxCommand* dxCommand);

	const std::string& GetName(size_t index) const;
	const std::string& GetName() const;

	DxResource& GetDxResource(size_t index);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::string name_;
	Vector4 clearColor_;

	std::vector<std::unique_ptr<RenderTexture>> renderTextures_;
	DxDepthStencil* pDxDepthStencil_ = nullptr;

};


} /// ONEngine
