#pragma once

/// engine
#include "Engine/Asset/Assets/Texture/Texture.h"
#include "Engine/Core/DirectX12/Resource/DxResource.h"
#include "Engine/Core/Utility/Math/Vector4.h"

namespace ONEngine {
class DxManager;
class DxDepthStencil;
class DxCommand;
class DxDSVHeap;
class RenderTexture;
}

namespace ONEngine::Asset {
class AssetCollection;
}



/// ///////////////////////////////////////////////////
/// render texture
/// ///////////////////////////////////////////////////
namespace ONEngine {

class RenderTexture {
private:
	/// ===================================================
	/// private : sub class
	/// ===================================================

	struct Handle {
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	};

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	RenderTexture();
	~RenderTexture();

	void Initialize(DXGI_FORMAT format, const Vector4& clearColor, const Vector2& textureSize, const std::string& name, DxManager* dxm, DxDepthStencil* dxDepthStencil, Asset::AssetCollection* assetCollection);

	/// @brief render targetとして設定
	/// @param dxCommand DxCommandのインスタンスへのポインタ
	void SetRenderTarget(DxCommand* dxCommand, DxDSVHeap* dxDSVHeap, bool clear = true);

	/// @brief 複数のrender targetとして設定
	/// @param dxCommand DxCommandのインスタンスへのポインタ
	/// @param dxDSVHeap DxDSVHeapのインスタンスへのポインタ
	/// @param other 他のrender textureのvector
	void SetRenderTarget(DxCommand* dxCommand, DxDSVHeap* dxDSVHeap, const std::vector<std::unique_ptr<RenderTexture>>& other, bool clear = true);

	/// @brief render textureとして設定
	/// @param dxCommand DxCommandのインスタンスへのポインタ
	void CreateBarrierRenderTarget(DxCommand* dxCommand);

	/// @brief srvとして設定
	/// @param dxCommand DxCommandのインスタンスへのポインタ
	void CreateBarrierPixelShaderResource(DxCommand* dxCommand);

	/// @brief RenderTextureの名前を取得
	/// @return std::string RenderTextureの名前
	const std::string& GetName() const;

	DxResource& GetDxResource();

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	DxDepthStencil* pDxDepthStencil_ = nullptr;

	std::string name_;
	Vector4 clearColor_;

	Handle rtvHandle_;
	Asset::Texture* texture_ = nullptr;


};



/// ///////////////////////////////////////////////////
/// UAVTexture
/// ///////////////////////////////////////////////////
class UAVTexture {
private:
	/// ===================================================
	/// private : sub class
	/// ===================================================

	struct Handle {
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	};

public:

	/// ===================================================
	/// public : methods
	/// ===================================================

	UAVTexture();
	~UAVTexture();

	/// @brief uav textureの初期化
	/// @param textureName textureの名前
	/// @param dxm DxManagerへのポインタ
	/// @param assetCollection AssetCollectionへのポインタ
	void Initialize(const std::string& textureName, DxManager* dxm, class Asset::AssetCollection* assetCollection);


private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	Asset::Texture* texture_ = nullptr;

};

} /// ONEngine
