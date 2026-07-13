#include "RenderTexture.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Asset/Guid/Guid.h"

RenderTexture::RenderTexture() = default;
RenderTexture::~RenderTexture() = default;

void RenderTexture::Initialize(DXGI_FORMAT format, const Vector4& clearColor, const Vector2& textureSize, const std::string& name, DxManager* dxm, DxDepthStencil* dxDepthStencil, Asset::AssetCollection* assetCollection) {
	clearColor_ = clearColor;
	name_ = name;
	pDxDepthStencil_ = dxDepthStencil;

	{	/// textureの作成
		Asset::Texture rtvTexture(textureSize);
		rtvTexture.guid = GenerateGuid();
		assetCollection->AddAsset<Asset::Texture>(name, std::move(rtvTexture)); /// textureの管理を AssetCollection に任せる
		texture_ = assetCollection->GetTexture(name);
	}

	/// 必要なオブジェクトの取得
	DxDevice* dxDevice = dxm->GetDxDevice();
	DxSRVHeap* dxSRVHeap = dxm->GetDxSRVHeap();
	DxRTVHeap* dxRTVHeap = dxm->GetDxRTVHeap();
	DxResource& renderTextureResource = texture_->GetDxResource();

	/// render texture resourceの作成
	renderTextureResource.CreateRenderTextureResource(
		dxDevice, textureSize, format, clearColor
	);

	uint32_t rtvHeapIndex = dxRTVHeap->Allocate();
	rtvHandle_.cpuHandle = dxRTVHeap->GetCPUDescriptorHandel(rtvHeapIndex);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	dxDevice->GetDevice()->CreateRenderTargetView(renderTextureResource.Get(), &rtvDesc, rtvHandle_.cpuHandle);


	/// shader resource viewの作成
	texture_->CreateEmptySRVHandle();
	texture_->SetSRVDescriptorIndex(dxSRVHeap->AllocateTexture());
	texture_->SetSRVCPUHandle(dxSRVHeap->GetCPUDescriptorHandel(texture_->GetSRVDescriptorIndex()));
	texture_->SetSRVGPUHandle(dxSRVHeap->GetGPUDescriptorHandel(texture_->GetSRVDescriptorIndex()));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	dxDevice->GetDevice()->CreateShaderResourceView(renderTextureResource.Get(), &srvDesc, texture_->GetSRVCPUHandle());


	renderTextureResource.CreateBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		dxm->GetDxCommand()
	);
}

void RenderTexture::SetRenderTarget(DxCommand* dxCommand, DxDSVHeap* dxDSVHeap, bool clear) {
	auto command = dxCommand->GetCommandList();
	uint32_t dsvIndex = pDxDepthStencil_->GetDepthDsvHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxDSVHeap->GetCPUDescriptorHandel(dsvIndex);

	command->OMSetRenderTargets(1, &rtvHandle_.cpuHandle, FALSE, &dsvHandle);
	
	if (clear) {
		command->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		command->ClearRenderTargetView(rtvHandle_.cpuHandle, &clearColor_.x, 0, nullptr);
	}
}

void RenderTexture::SetRenderTarget(DxCommand* dxCommand, DxDSVHeap* dxDSVHeap, const std::vector<std::unique_ptr<class RenderTexture>>& others, bool clear) {
	auto command = dxCommand->GetCommandList();
	uint32_t dsvIndex = pDxDepthStencil_->GetDepthDsvHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxDSVHeap->GetCPUDescriptorHandel(dsvIndex);

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
	for (auto& rt : others) {
		rtvHandles.push_back(rt->rtvHandle_.cpuHandle);
	}

	command->OMSetRenderTargets(static_cast<UINT>(rtvHandles.size()), rtvHandles.data(), FALSE, &dsvHandle);
	
	if (clear) {
		command->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		for (auto& rt : others) {
			command->ClearRenderTargetView(rt->rtvHandle_.cpuHandle, &rt->clearColor_.x, 0, nullptr);
		}
	}
}

void RenderTexture::CreateBarrierRenderTarget(DxCommand* dxCommand) {
	texture_->GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		dxCommand
	);
}

void RenderTexture::CreateBarrierPixelShaderResource(DxCommand* dxCommand) {
	texture_->GetDxResource().CreateBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		dxCommand
	);
}

const std::string& RenderTexture::GetName() const {
	return name_;
}

DxResource& RenderTexture::GetDxResource() {
	return texture_->GetDxResource();
}



/// ///////////////////////////////////////////////////
/// UAVTexture
/// ///////////////////////////////////////////////////

UAVTexture::UAVTexture() = default;
UAVTexture::~UAVTexture() = default;

void UAVTexture::Initialize(const std::string& textureName, DxManager* dxm, Asset::AssetCollection* assetCollection) {
	Asset::Texture uavTexture;
	uavTexture.guid = GenerateGuid();
	assetCollection->AddAsset<Asset::Texture>(textureName, std::move(uavTexture));
	texture_ = assetCollection->GetTexture(textureName);

	/// 必要なオブジェクトの取得
	DxDevice* dxDevice = dxm->GetDxDevice();
	DxSRVHeap* dxSRVHeap = dxm->GetDxSRVHeap();
	DxResource& uavTextureResource = texture_->GetDxResource();

	/// UAV texture resourceの作成
	uavTextureResource.CreateUAVTextureResource(
		dxDevice, EngineConfig::kWindowSize, DXGI_FORMAT_R8G8B8A8_UNORM
	);

	texture_->CreateEmptyUAVHandle();
	texture_->SetUAVDescriptorIndex(dxSRVHeap->AllocateUAVTexture());
	texture_->SetUAVCPUHandle(dxSRVHeap->GetCPUDescriptorHandel(texture_->GetUAVDescriptorIndex()));
	texture_->SetUAVGPUHandle(dxSRVHeap->GetGPUDescriptorHandel(texture_->GetUAVDescriptorIndex()));

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	dxDevice->GetDevice()->CreateUnorderedAccessView(uavTextureResource.Get(), nullptr, &uavDesc, texture_->GetUAVCPUHandle());

	/// shader resource viewの作成
	texture_->CreateEmptySRVHandle();
	texture_->SetSRVDescriptorIndex(dxSRVHeap->AllocateTexture());
	texture_->SetSRVCPUHandle(dxSRVHeap->GetCPUDescriptorHandel(texture_->GetSRVDescriptorIndex()));
	texture_->SetSRVGPUHandle(dxSRVHeap->GetGPUDescriptorHandel(texture_->GetSRVDescriptorIndex()));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	dxDevice->GetDevice()->CreateShaderResourceView(uavTextureResource.Get(), &srvDesc, texture_->GetSRVCPUHandle());
}
