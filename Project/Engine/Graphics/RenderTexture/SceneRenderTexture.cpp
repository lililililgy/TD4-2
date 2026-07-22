#include "SceneRenderTexture.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Graphics/Shader/GraphicsPipeline.h"

SceneRenderTexture::SceneRenderTexture() {
	name_ = "";
	clearColor_ = Vector4(0.1f, 0.25f, 0.5f, 1.0f);
}


void SceneRenderTexture::Initialize( 
	const std::string& name, const Vector4& clearColor, const Vector2& textureSize,
	DxManager* dxm, Asset::AssetCollection* assetCollection) {

	/// パラメータの設定
	name_ = name;
	clearColor_ = clearColor;

	/// 新規のDepthStencilを作成
	pDxDepthStencil_ = dxm->AddDepthStencil(name);


	/// texture init

	{	/// game render textures
		renderTextures_.resize(4);
		for (auto& renderTexture : renderTextures_) {
			renderTexture = std::make_unique<RenderTexture>();
		}

		renderTextures_[0]->Initialize(static_cast<DXGI_FORMAT>(RTVFormat::Color),         clearColor_, textureSize, name_ + "Scene",         dxm, pDxDepthStencil_, assetCollection);
		renderTextures_[1]->Initialize(static_cast<DXGI_FORMAT>(RTVFormat::WorldPosition), clearColor_, textureSize, name_ + "WorldPosition", dxm, pDxDepthStencil_, assetCollection);
		renderTextures_[2]->Initialize(static_cast<DXGI_FORMAT>(RTVFormat::Normal),        clearColor_, textureSize, name_ + "Normal",        dxm, pDxDepthStencil_, assetCollection);
		renderTextures_[3]->Initialize(static_cast<DXGI_FORMAT>(RTVFormat::Flags),         {},          textureSize, name_ + "Flags",         dxm, pDxDepthStencil_, assetCollection);
	}

}

void SceneRenderTexture::SetClearColor(const Vector4& clearColor) {
	clearColor_ = clearColor;
	for (size_t i = 0; i < renderTextures_.size(); ++i) {
		if (i != 3 && renderTextures_[i]) {
			renderTextures_[i]->SetClearColor(clearColor);
		}
	}
}

void SceneRenderTexture::SetRenderTarget(DxCommand* dxCommand, DxDSVHeap* dxDSVHeap, bool clear) {
	renderTextures_[0]->SetRenderTarget(
		dxCommand, dxDSVHeap,
		renderTextures_,
		clear
	);
}

void SceneRenderTexture::SetRenderTargetColorOnly(DxCommand* dxCommand, DxDSVHeap* dxDSVHeap, bool clear) {
	renderTextures_[0]->SetRenderTarget(
		dxCommand, dxDSVHeap,
		clear
	);
}

void SceneRenderTexture::CreateBarrierRenderTarget(DxCommand* dxCommand) {
	for (auto& renderTexture : renderTextures_) {
		renderTexture->CreateBarrierRenderTarget(dxCommand);
	}
}

void SceneRenderTexture::CreateBarrierPixelShaderResource(DxCommand* dxCommand) {
	for (auto& renderTexture : renderTextures_) {
		renderTexture->CreateBarrierPixelShaderResource(dxCommand);
	}
}

const std::string& SceneRenderTexture::GetName(size_t index) const {
	if (index < renderTextures_.size()) {
		return renderTextures_[index]->GetName();
	}

	static const std::string emptyString = "";
	return emptyString;
}

const std::string& SceneRenderTexture::GetName() const {
	return name_;
}

DxResource& SceneRenderTexture::GetDxResource(size_t index) {
	return renderTextures_[index]->GetDxResource();
}

