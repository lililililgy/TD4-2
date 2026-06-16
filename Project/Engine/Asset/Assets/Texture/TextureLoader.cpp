#include "TextureLoader.h"

/// std
#include <fstream>
#include <mutex>
#include <format>

/// externals
#include <magic_enum/magic_enum.hpp>

/// engine
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Core/Utility/Utility.h"

#include "Engine/Core/Threading/ThreadPool.h"

namespace {
/// SRVの割り当てとログ出力が競合しないようにするためだけのミューテックス
std::mutex s_textureGpuMutex;

/// @brief DXGI_FORMATの文字列から列挙型を取得する
DXGI_FORMAT GetDxgiFormatFromString(const std::string& formatStr) {
	if(formatStr == "R8G8B8A8_UNORM") return DXGI_FORMAT_R8G8B8A8_UNORM;
	if(formatStr == "R8G8B8A8_UNORM_SRGB") return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	if(formatStr == "BC7_UNORM") return DXGI_FORMAT_BC7_UNORM;
	if(formatStr == "BC7_UNORM_SRGB") return DXGI_FORMAT_BC7_UNORM_SRGB;
	if(formatStr == "R32G32B32A32_FLOAT") return DXGI_FORMAT_R32G32B32A32_FLOAT;
	return DXGI_FORMAT_UNKNOWN;
}

}

namespace ONEngine::Asset {

AssetLoader<Texture>::AssetLoader(DxManager* dxm, AssetCollection* ac)
	: pDxManager_(dxm), pAssetCollection_(ac) {}

std::optional<Texture> AssetLoader<Texture>::Load(const std::string& filepath, Meta<Texture::MetaData> meta) {
	std::optional<Texture> res{};

	const std::string extension = FileSystem::FileExtension(filepath);
	if(extension == ".dds") {
		DirectX::ScratchImage scratch = LoadScratchImage3D(filepath);
		const auto& texMeta = scratch.GetMetadata();
		if(texMeta.dimension == DirectX::TEX_DIMENSION_TEXTURE3D) {
			res = Load3DTexture(filepath);
			if(res.has_value()) {
				res->guid = meta.base.guid;
				return res;
			}
		}
	}

	res = Load2DTexture(filepath);
	if(res.has_value()) {
		res->guid = meta.base.guid;
	}
	return res;
}

std::optional<Texture> AssetLoader<Texture>::Reload(const std::string& filepath, Texture* src, Meta<Texture::MetaData> meta) {
	std::optional<Texture> res{};

	const std::string extension = FileSystem::FileExtension(filepath);
	if(extension == ".dds") {
		DirectX::ScratchImage scratch = LoadScratchImage3D(filepath);
		const auto& texMeta = scratch.GetMetadata();
		if(texMeta.dimension == DirectX::TEX_DIMENSION_TEXTURE3D) {
			res = Reload3DTexture(filepath, src);
			if(res.has_value()) {
				res->guid = meta.base.guid;
				return res;
			}
		}
	}

	res = Reload2DTexture(filepath, src);
	if(res.has_value()) {
		res->guid = meta.base.guid;
	}
	return res;
}

Meta<Texture::MetaData> AssetLoader<Texture>::GetMetaData(const std::string& filepath) {
	Meta<Texture::MetaData> res{};

	const std::string metaPath = filepath + ".meta";
	res.base = LoadOrGenerateMetaBase(metaPath, filepath);

	nlohmann::json j;
	std::ifstream ifs(metaPath);
	if(!ifs.is_open()) {
		return {};
	}

	ifs >> j;
	Texture::MetaData data;
	data.format = j.value("format", TextureFormat::RGBA16_FLOAT);
	data.colorSpace = j.value("colorSpace", ColorSpace::Linear);

	res.data = data;

	return res;
}


std::optional<Texture> AssetLoader<Texture>::Load2DTexture(const std::string& filepath) {
	Texture texture;

	DirectX::ScratchImage       scratchImage = LoadScratchImage2D(filepath);
	const DirectX::TexMetadata& metadata = scratchImage.GetMetadata();

	texture.dxResource_ = std::move(CreateTextureResource2D(pDxManager_->GetDxDevice(), metadata));
	if(!texture.dxResource_.Get()) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Load Failed] [Texture] - Don't Create DxResource: \"" + filepath + "\"");
		return std::nullopt;
	}

	texture.dxResource_.Get()->SetName(ConvertString(filepath).c_str());
	texture.name_ = filepath;

	DxResource intermediateResource;

	/// =========================================================================
	/// VRAMへのアップロード（スレッドローカルコマンドを使用して無停止実行）
	/// =========================================================================
	auto* workerCtx = ThreadPool::GetWorkerContext();

	if(workerCtx && workerCtx->uploadCmd) {
		/// 【ワーカースレッド】共有Queueを取得して、自分専用のコマンドリストでアップロード
		ID3D12CommandQueue* cmdQueue = pDxManager_->GetDxCommand()->GetCommandQueue();

		workerCtx->uploadCmd->Begin(); // 記録開始
		intermediateResource = UploadTextureData(texture.dxResource_.Get(), scratchImage);
		workerCtx->uploadCmd->End();   // 記録終了

		// 共有キューに送信し、自身のスレッドのFenceで待機する
		workerCtx->uploadCmd->ExecuteAndWait(cmdQueue);
	} else {
		/// 【メインスレッド】共有コマンドリストを使うためロックが必要
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		intermediateResource = UploadTextureData(texture.dxResource_.Get(), scratchImage);
		pDxManager_->GetDxCommand()->CommandExecuteAndWait();
		pDxManager_->GetDxCommand()->CommandReset();
	}

	/// =========================================================================
	/// SRVの設定とログ出力（共有リソースに触るため一瞬だけ順番待ちする）
	/// =========================================================================
	{
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = metadata.format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if(metadata.IsCubemap()) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = UINT_MAX;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		} else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
		}

		DxSRVHeap* dxSRVHeap = pDxManager_->GetDxSRVHeap();
		texture.CreateEmptySRVHandle();
		texture.srvHandle_->descriptorIndex = dxSRVHeap->AllocateTexture();
		texture.srvHandle_->cpuHandle = dxSRVHeap->GetCPUDescriptorHandel(texture.srvHandle_->descriptorIndex);
		texture.srvHandle_->gpuHandle = dxSRVHeap->GetGPUDescriptorHandel(texture.srvHandle_->descriptorIndex);

		DxDevice* dxDevice = pDxManager_->GetDxDevice();
		dxDevice->GetDevice()->CreateShaderResourceView(texture.dxResource_.Get(), &srvDesc, texture.srvHandle_->cpuHandle);

		Vector2 textureSize = { static_cast<float>(metadata.width), static_cast<float>(metadata.height) };
		texture.textureSize_ = textureSize;
		texture.srvFormat_ = metadata.format;
		texture.isCubeMap_ = metadata.IsCubemap();
		texture.arraySize_ = static_cast<UINT>(metadata.arraySize);

		Console::Log("[Success Texture Info] Path: \"" + filepath + "\"");
		Console::Log(" - DescriptorIndex: " + std::to_string(texture.srvHandle_->descriptorIndex));
	}

	return std::move(texture);
}

std::optional<Texture> AssetLoader<Texture>::Load3DTexture(const std::string& filepath) {
	Texture texture;

	DirectX::ScratchImage scratchImage = LoadScratchImage3D(filepath);
	if(scratchImage.GetImageCount() == 0) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Load Failed] [Texture DDS] - Failed to load DDS file: \"" + filepath + "\"");
		return std::nullopt;
	}

	const DirectX::TexMetadata& metadata = scratchImage.GetMetadata();
	if(metadata.dimension != DirectX::TEX_DIMENSION_TEXTURE3D) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Load Failed] [Texture DDS] - Not a 3D texture: \"" + filepath + "\"");
		return std::nullopt;
	}

	texture.dxResource_ = std::move(CreateTextureResource3D(pDxManager_->GetDxDevice(), metadata));
	if(!texture.dxResource_.Get()) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Load Failed] [Texture DDS] - Don't Create DxResource: \"" + filepath + "\"");
		return std::nullopt;
	}

	texture.dxResource_.Get()->SetName(ConvertString(filepath).c_str());
	texture.dxResource_.SetCurrentState(D3D12_RESOURCE_STATE_GENERIC_READ);
	texture.name_ = filepath;

	DxResource intermediateResource;

	auto* workerCtx = ThreadPool::GetWorkerContext();
	if(workerCtx && workerCtx->uploadCmd) {
		ID3D12CommandQueue* cmdQueue = pDxManager_->GetDxCommand()->GetCommandQueue();
		workerCtx->uploadCmd->Begin();
		intermediateResource = UploadTextureData(texture.dxResource_.Get(), scratchImage);
		workerCtx->uploadCmd->End();
		workerCtx->uploadCmd->ExecuteAndWait(cmdQueue);
	} else {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		intermediateResource = UploadTextureData(texture.dxResource_.Get(), scratchImage);
		pDxManager_->GetDxCommand()->CommandExecuteAndWait();
		pDxManager_->GetDxCommand()->CommandReset();
	}

	{
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = metadata.format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture3D.MipLevels = static_cast<UINT16>(metadata.mipLevels);

		DxSRVHeap* dxSRVHeap = pDxManager_->GetDxSRVHeap();
		texture.CreateEmptySRVHandle();
		texture.srvHandle_->descriptorIndex = dxSRVHeap->AllocateTexture();
		texture.srvHandle_->cpuHandle = dxSRVHeap->GetCPUDescriptorHandel(texture.srvHandle_->descriptorIndex);
		texture.srvHandle_->gpuHandle = dxSRVHeap->GetGPUDescriptorHandel(texture.srvHandle_->descriptorIndex);

		DxDevice* dxDevice = pDxManager_->GetDxDevice();
		dxDevice->GetDevice()->CreateShaderResourceView(texture.dxResource_.Get(), &srvDesc, texture.srvHandle_->cpuHandle);

		Vector2 textureSize = { static_cast<float>(metadata.width), static_cast<float>(metadata.height) };
		texture.textureSize_ = textureSize;
		texture.depth_ = static_cast<uint32_t>(metadata.depth);
		texture.srvFormat_ = metadata.format;

		Console::Log("[Texture DDS Info] Path: \"" + filepath + "\"");
	}

	return std::move(texture);
}

std::optional<Texture> AssetLoader<Texture>::Reload2DTexture(const std::string& filepath, Texture* src) {
	Texture texture = *src;

	DirectX::ScratchImage       scratchImage = LoadScratchImage2D(filepath);
	const DirectX::TexMetadata& metadata = scratchImage.GetMetadata();

	if(metadata.width == 0 || metadata.height == 0) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Reload Failed] [Texture] - Invalid dimensions: \"" + filepath + "\"");
		return std::nullopt;
	}

	DxResource newResource = CreateTextureResource2D(pDxManager_->GetDxDevice(), metadata);
	if(!newResource.Get()) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Reload Failed] [Texture] - Don't Create DxResource: \"" + filepath + "\"");
		return std::nullopt;
	}

	newResource.Get()->SetName(ConvertString(filepath).c_str());
	texture.dxResource_ = std::move(newResource);
	DxResource intermediateResource;

	auto* workerCtx = ThreadPool::GetWorkerContext();
	if(workerCtx && workerCtx->uploadCmd) {
		ID3D12CommandQueue* cmdQueue = pDxManager_->GetDxCommand()->GetCommandQueue();
		workerCtx->uploadCmd->Begin();
		intermediateResource = UploadTextureData(texture.dxResource_.Get(), scratchImage);
		workerCtx->uploadCmd->End();
		workerCtx->uploadCmd->ExecuteAndWait(cmdQueue);
	} else {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		intermediateResource = UploadTextureData(texture.dxResource_.Get(), scratchImage);
		pDxManager_->GetDxCommand()->CommandExecuteAndWait();
		pDxManager_->GetDxCommand()->CommandReset();
	}

	{
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = metadata.format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if(metadata.IsCubemap()) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = UINT_MAX;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		} else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
		}

		DxDevice* dxDevice = pDxManager_->GetDxDevice();
		dxDevice->GetDevice()->CreateShaderResourceView(texture.dxResource_.Get(), &srvDesc, texture.srvHandle_->cpuHandle);

		Vector2 textureSize = { static_cast<float>(metadata.width), static_cast<float>(metadata.height) };
		texture.textureSize_ = textureSize;
		texture.srvFormat_ = metadata.format;
		texture.isCubeMap_ = metadata.IsCubemap();
		texture.arraySize_ = static_cast<UINT>(metadata.arraySize);

		Console::Log("[Success Reload Texture Info] Path: \"" + filepath + "\"");
	}

	return std::move(texture);
}

std::optional<Texture> AssetLoader<Texture>::Reload3DTexture(const std::string& filepath, Texture* src) {
	Texture texture = *src;

	DirectX::ScratchImage scratchImage = LoadScratchImage3D(filepath);
	if(scratchImage.GetImageCount() == 0) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Reload Failed] [Texture DDS] - Failed to load DDS file: \"" + filepath + "\"");
		return std::nullopt;
	}

	const DirectX::TexMetadata& metadata = scratchImage.GetMetadata();
	if(metadata.dimension != DirectX::TEX_DIMENSION_TEXTURE3D) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Reload Failed] [Texture DDS] - Not a 3D texture: \"" + filepath + "\"");
		return std::nullopt;
	}

	DxResource newResource = CreateTextureResource3D(pDxManager_->GetDxDevice(), metadata);
	if(!newResource.Get()) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Reload Failed] [Texture DDS] - Don't Create DxResource: \"" + filepath + "\"");
		return std::nullopt;
	}

	newResource.Get()->SetName(ConvertString(filepath).c_str());
	DxResource oldResourceKeeper = std::move(texture.dxResource_);
	texture.dxResource_ = std::move(newResource);
	DxResource intermediateResource;

	auto* workerCtx = ThreadPool::GetWorkerContext();
	if(workerCtx && workerCtx->uploadCmd) {
		ID3D12CommandQueue* cmdQueue = pDxManager_->GetDxCommand()->GetCommandQueue();
		workerCtx->uploadCmd->Begin();
		intermediateResource = UploadTextureData(texture.dxResource_.Get(), scratchImage);
		workerCtx->uploadCmd->End();
		workerCtx->uploadCmd->ExecuteAndWait(cmdQueue);
	} else {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		intermediateResource = UploadTextureData(texture.dxResource_.Get(), scratchImage);
		pDxManager_->GetDxCommand()->CommandExecuteAndWait();
		pDxManager_->GetDxCommand()->CommandReset();
	}

	{
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = metadata.format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture3D.MipLevels = static_cast<UINT16>(metadata.mipLevels);
		srvDesc.Texture3D.MostDetailedMip = 0;
		srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;

		DxDevice* dxDevice = pDxManager_->GetDxDevice();
		dxDevice->GetDevice()->CreateShaderResourceView(texture.dxResource_.Get(), &srvDesc, texture.srvHandle_->cpuHandle);

		Vector2 textureSize = { static_cast<float>(metadata.width), static_cast<float>(metadata.height) };
		texture.textureSize_ = textureSize;
		texture.depth_ = static_cast<uint32_t>(metadata.depth);
		texture.srvFormat_ = metadata.format;

		Console::Log("[Success Reload Texture3D] Path: \"" + filepath + "\"");
	}

	return std::move(texture);
}

DirectX::ScratchImage AssetLoader<Texture>::LoadScratchImage2D(const std::string& filepath) {
	DirectX::ScratchImage image{};
	std::wstring          filePathW = ConvertString(filepath);
	HRESULT hr = S_OK;

	if(filepath.ends_with(".dds")) {
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		if (FAILED(hr)) {
			// sRGB強制で失敗した場合は、フラグなしでリトライ（グレースケールや特殊フォーマット用）
			hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
		}
	}

	if (FAILED(hr)) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError(std::format("[Load Failed] [Texture WIC/DDS] - HRESULT: 0x{:08X}, Path: \"{}\"", (uint32_t)hr, filepath));
		return DirectX::ScratchImage{};
	}

	DirectX::ScratchImage mipImages{};

	if(DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	} else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		if (FAILED(hr)) {
			std::lock_guard<std::mutex> lock(s_textureGpuMutex);
			Console::LogWarning(std::format("[MipMap Failed] [Texture] - HRESULT: 0x{:08X}, Path: \"{}\"", (uint32_t)hr, filepath));
			mipImages = std::move(image);
		}
	}
	return mipImages;
}

DirectX::ScratchImage AssetLoader<Texture>::LoadScratchImage3D(const std::string& filepath) {
	if(!filepath.ends_with(".dds")) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("LoadScratchImage3D: Only DDS files are supported for Texture3D.");
		return DirectX::ScratchImage{};
	}

	std::wstring filePathW = ConvertString(filepath);
	DirectX::ScratchImage image;

	HRESULT hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	if(FAILED(hr)) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Load Failed] Texture3D DDS: \"" + filepath + "\"");
		return DirectX::ScratchImage{};
	}

	const DirectX::TexMetadata& meta = image.GetMetadata();
	if(meta.dimension != DirectX::TEX_DIMENSION_TEXTURE3D) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[Load Failed] File is not a Texture3D DDS: \"" + filepath + "\"");
		return DirectX::ScratchImage{};
	}

	DirectX::ScratchImage mipImages;
	if(DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	} else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipImages);
		if(FAILED(hr)) {
			std::lock_guard<std::mutex> lock(s_textureGpuMutex);
			Console::LogWarning("[GenerateMipMaps Failed] Using original image: \"" + filepath + "\"");
			mipImages = std::move(image);
		}
	}
	return mipImages;
}

DxResource AssetLoader<Texture>::CreateTextureResource2D(DxDevice* dxDevice, const DirectX::TexMetadata& metadata) {
	D3D12_RESOURCE_DESC desc{};
	desc.Width = UINT(metadata.width);
	desc.Height = UINT(metadata.height);
	desc.MipLevels = UINT16(metadata.mipLevels);
	desc.DepthOrArraySize = UINT16(metadata.arraySize);
	desc.Format = metadata.format;
	desc.SampleDesc.Count = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	DxResource dxResource;
	dxResource.CreateCommittedResource(dxDevice, &heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);
	return dxResource;
}

DxResource AssetLoader<Texture>::CreateTextureResource3D(DxDevice* dxDevice, const DirectX::TexMetadata& metadata) {
	if(metadata.dimension != DirectX::TEX_DIMENSION_TEXTURE3D) {
		std::lock_guard<std::mutex> lock(s_textureGpuMutex);
		Console::LogError("[CreateTexture3DResource] Metadata is not Texture3D.");
		return DxResource{};
	}

	D3D12_RESOURCE_DESC desc{};
	desc.Width = static_cast<UINT>(metadata.width);
	desc.Height = static_cast<UINT>(metadata.height);
	desc.DepthOrArraySize = static_cast<UINT16>(metadata.depth);
	desc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
	desc.Format = metadata.format;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	DxResource dxResource;
	dxResource.CreateCommittedResource(dxDevice, &heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);
	return dxResource;
}

DxResource AssetLoader<Texture>::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipScratchImage) {
	DxDevice* dxDevice = pDxManager_->GetDxDevice();

	/// 実行しているスレッドに応じてコマンドリストを出し分ける
	auto* workerCtx = ThreadPool::GetWorkerContext();
	ID3D12GraphicsCommandList* cmdList = nullptr;

	if(workerCtx && workerCtx->uploadCmd) {
		cmdList = workerCtx->uploadCmd->GetCommandList();
	} else {
		cmdList = pDxManager_->GetDxCommand()->GetCommandList();
	}

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(dxDevice->GetDevice(), mipScratchImage.GetImages(), mipScratchImage.GetImageCount(), mipScratchImage.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, static_cast<UINT>(subresources.size()));

	DxResource intermediateDxResource;
	intermediateDxResource.CreateResource(dxDevice, intermediateSize);

	UpdateSubresources(cmdList, texture, intermediateDxResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	/// 型依存（DxCommand）を避けるため、手動でリソースバリアを作成
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmdList->ResourceBarrier(1, &barrier);

	return intermediateDxResource;
}
}