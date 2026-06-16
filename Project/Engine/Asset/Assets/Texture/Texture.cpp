#include "Texture.h"

/// directX12
#include <wrl/client.h>
#include <wincodec.h>

/// std
#include <cassert>
#include <iostream>

/// externals
#include <DirectXTex.h>
#include <magic_enum/magic_enum.hpp>

/// engine
#include "Engine/Core/Utility/Tools/Assert.h"
#include "Engine/Core/DirectX12/Device/DxDevice.h"
#include "Engine/Core/DirectX12/DescriptorHeap/DxSRVHeap.h"
#include "Engine/Core/DirectX12/Command/DxCommand.h"


namespace {
/// printf 互換のフォーマットログ
template <class... Args>
void Printf(const char* fmt, Args... args) {
	// 出力サイズ計算
	int size = std::snprintf(nullptr, 0, fmt, args...);
	if(size <= 0) {
		ONEngine::Console::Log("Format error");
		return;
	}

	// サイズ分の文字列を生成
	std::string msg(size, '\0');

	// 実際にフォーマット文字列を埋め込む
	std::snprintf(&msg[0], size + 1, fmt, args...);

	// Console::Log に渡す
	ONEngine::Console::Log(msg);
}
}

namespace ONEngine::Asset {

void from_json(const nlohmann::json& j, TextureFormat& format) {
	if (j.is_string()) {
		auto opt = magic_enum::enum_cast<TextureFormat>(j.get<std::string>(), magic_enum::case_insensitive);
		format = opt.value_or(TextureFormat::RGBA8_UNORM);
	} else if (j.is_number()) {
		format = static_cast<TextureFormat>(j.get<int>());
	} else {
		format = TextureFormat::RGBA8_UNORM;
	}
}

void to_json(nlohmann::json& j, const TextureFormat& format) {
	j = std::string(magic_enum::enum_name(format));
}

void from_json(const nlohmann::json& j, ColorSpace& colorSpace) {
	if (j.is_string()) {
		auto opt = magic_enum::enum_cast<ColorSpace>(j.get<std::string>(), magic_enum::case_insensitive);
		colorSpace = opt.value_or(ColorSpace::Linear);
	} else if (j.is_number()) {
		colorSpace = static_cast<ColorSpace>(j.get<int>());
	} else {
		colorSpace = ColorSpace::Linear;
	}
}

void to_json(nlohmann::json& j, const ColorSpace& colorSpace) {
	j = std::string(magic_enum::enum_name(colorSpace));
}

Texture::Texture() = default;

Texture::Texture(const Vector2& textureSize)
	: textureSize_(textureSize) {}

void Texture::CreateEmptySRVHandle() {
	srvHandle_.emplace(Handle());
}

void Texture::CreateEmptyUAVHandle() {
	uavHandle_.emplace(Handle());
}

void Texture::CreateUAVTexture(UINT width, UINT height, DxDevice* dxDevice, DxSRVHeap* dxSRVHeap, DXGI_FORMAT dxgiFormat) {
	// テクスチャディスクリプション
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = dxgiFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// リソース作成
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	dxResource_.CreateCommittedResource(
		dxDevice, &heapProperties, D3D12_HEAP_FLAG_NONE,
		&texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr
	);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = dxgiFormat;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Texture2D.PlaneSlice = 0;

	uint32_t index = dxSRVHeap->AllocateUAVTexture();
	CreateEmptyUAVHandle();
	SetUAVDescriptorIndex(index);
	SetUAVCPUHandle(dxSRVHeap->GetCPUDescriptorHandel(index));
	SetUAVGPUHandle(dxSRVHeap->GetGPUDescriptorHandel(index));

	dxDevice->GetDevice()->CreateUnorderedAccessView(dxResource_.Get(), nullptr, &uavDesc, uavHandle_->cpuHandle);



	/// ログに今回行った操作を出力
	Console::Log("[Create UAV Texture] ");
	Console::Log(" - Width: " + std::to_string(width));
	Console::Log(" - Height: " + std::to_string(height));
	Console::Log(" - Format: " + std::string(magic_enum::enum_name(dxgiFormat)));
	Console::Log(" - DescriptorIndex: " + std::to_string(uavHandle_->descriptorIndex));
	Console::Log(" - Dimension: Texture2D");

}


void Texture::CreateUAVTexture3DWithUAV(
	UINT width, UINT height, UINT depth,
	DxDevice* dxDevice,
	DxSRVHeap* dxSRVHeap,
	DXGI_FORMAT uavFormat) {

	ID3D12Device* device = dxDevice->GetDevice();
	uavFormat_ = uavFormat;

	/// --------------- UAV Descriptor 設定 --------------- ///
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = srvFormat_;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.WSize = depth;

	uint32_t descriptorIndex;
	if(!uavHandle_.has_value()) {
		/// 新規作成
		CreateEmptyUAVHandle();
		descriptorIndex = dxSRVHeap->AllocateUAVTexture();
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = dxSRVHeap->GetCPUDescriptorHandel(descriptorIndex);
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = dxSRVHeap->GetGPUDescriptorHandel(descriptorIndex);
		SetUAVHandle(descriptorIndex, cpuHandle, gpuHandle);

		Assert(descriptorIndex != 1029 + 2048);
	} else {
		/// 既存のハンドルを使用
		descriptorIndex = uavHandle_->descriptorIndex;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = dxSRVHeap->GetCPUDescriptorHandel(descriptorIndex);

	/// --------------- UAV生成 --------------- ///
	device->CreateUnorderedAccessView(
		dxResource_.Get(),
		nullptr,
		&uavDesc,
		cpuHandle
	);

	dxResource_.SetCurrentState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	/// --------------- ログ --------------- ///
	Console::Log("[Create UAV Texture3D]");
	Console::Log(" - Texture Name: " + name_);
	Console::Log(" - Width: " + std::to_string(width));
	Console::Log(" - Height: " + std::to_string(height));
	Console::Log(" - Depth: " + std::to_string(depth));
	Console::Log(" - UAV Format: " + std::string(magic_enum::enum_name(uavFormat_)));
	Console::Log(" - SRV Format: " + std::string(magic_enum::enum_name(srvFormat_)));
	Console::Log(" - DescriptorIndex: " + std::to_string(descriptorIndex));
}


void Texture::CreateUAVTexture3D(UINT width, UINT height, UINT depth, DxDevice* dxDevice, DxSRVHeap* dxSRVHeap, DXGI_FORMAT dxgiFormat) {
	// テクスチャディスクリプション
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	texDesc.Alignment = 0;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = static_cast<UINT16>(depth);
	texDesc.MipLevels = 1;
	texDesc.Format = dxgiFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// リソース作成
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	dxResource_.CreateCommittedResource(
		dxDevice, &heapProperties, D3D12_HEAP_FLAG_NONE,
		&texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr
	);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = dxgiFormat;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.WSize = depth;

	uint32_t index = dxSRVHeap->AllocateUAVTexture();
	CreateEmptyUAVHandle();
	SetUAVDescriptorIndex(index);
	SetUAVCPUHandle(dxSRVHeap->GetCPUDescriptorHandel(index));
	SetUAVGPUHandle(dxSRVHeap->GetGPUDescriptorHandel(index));

	dxDevice->GetDevice()->CreateUnorderedAccessView(dxResource_.Get(), nullptr, &uavDesc, uavHandle_->cpuHandle);

	/// ログに今回行った操作を出力
	Console::Log("[Create UAV Texture3D]");
	Console::Log(" - Texture Name: " + name_);
	Console::Log(" - Width: " + std::to_string(width));
	Console::Log(" - Height: " + std::to_string(height));
	Console::Log(" - Depth: " + std::to_string(depth));
	Console::Log(" - UAV Format: " + std::string(magic_enum::enum_name(uavFormat_)));
	Console::Log(" - SRV Format: " + std::string(magic_enum::enum_name(srvFormat_)));
	Console::Log(" - DescriptorIndex: " + std::to_string(index));
}

void Texture::OutputTexture(const std::wstring& filename, DxDevice* dxDevice, DxCommand* dxCommand) {
	/// Readbackリソースを作成（1行ごとのAlignmentに注意）
	D3D12_RESOURCE_DESC desc = dxResource_.Get()->GetDesc();

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	UINT numRows = 0;
	UINT64 rowPitch = 0;
	UINT64 totalBytes = 0;
	dxDevice->GetDevice()->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &numRows, &rowPitch, &totalBytes);

	CD3DX12_RESOURCE_DESC rbDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
	readbackTexture_.CreateCommittedResource(
		dxDevice, &heapProperties, D3D12_HEAP_FLAG_NONE,
		&rbDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr
	);

	/// dst (Readback側) の設定
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = readbackTexture_.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dst.PlacedFootprint = footprint;

	/// src (UAV側) の設定
	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = dxResource_.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex = 0;

	dxResource_.CreateBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, dxCommand);
	dxCommand->GetCommandList()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	dxResource_.CreateBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, dxCommand);

	/// コマンドの実行&リセット
	dxCommand->CommandExecuteAndWait();
	dxCommand->CommandReset();


	/// ReadbackしてDirectXTexに詰め替え
	D3D12_RANGE range = { 0, static_cast<SIZE_T>(totalBytes) };
	void* mappedData = nullptr;
	readbackTexture_.Get()->Map(0, &range, &mappedData);

	DirectX::Image image = {};
	image.width = static_cast<size_t>(desc.Width);
	image.height = desc.Height;
	image.format = desc.Format;
	image.rowPitch = static_cast<size_t>(footprint.Footprint.RowPitch);
	image.slicePitch = static_cast<size_t>(footprint.Footprint.RowPitch) * desc.Height;
	image.pixels = reinterpret_cast<uint8_t*>(mappedData);

	DirectX::ScratchImage scratch;
	scratch.InitializeFromImage(image);

	/// .pngで保存
	DirectX::SaveToWICFile(
		*scratch.GetImage(0, 0, 0),
		DirectX::WIC_FLAGS_NONE,
		GUID_ContainerFormatPng,
		filename.c_str()
	);

	readbackTexture_.Get()->Unmap(0, nullptr);
}

void Texture::OutputTexture3D(const std::wstring& filename, DxDevice* dxDevice, DxCommand* dxCommand) {
	auto desc = dxResource_.Get()->GetDesc();
	if(desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D) {
		Assert(false, "Not a 3D texture.");
	}

	// Readback用バッファサイズを取得
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
	UINT numRows = 0;
	UINT64 rowPitch = 0;
	UINT64 totalBytes = 0;
	dxDevice->GetDevice()->GetCopyableFootprints(
		&desc,
		0,      // FirstSubresource
		1,      // NumSubresources = 1 for 3D texture
		0,
		&footprint,
		&numRows,
		&rowPitch,
		&totalBytes
	);

	// Readbackバッファを作成
	CD3DX12_RESOURCE_DESC rbDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);
	D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);

	DxResource readback;
	readback.CreateCommittedResource(
		dxDevice,
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&rbDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr
	);

	// コピー前にリソース状態を変更
	dxResource_.CreateBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, dxCommand);

	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = dxResource_.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = readback.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dst.PlacedFootprint = footprint;

	// コピー
	dxCommand->GetCommandList()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	// コピー後、UAVに戻す場合
	dxResource_.CreateBarrier(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, dxCommand);

	dxCommand->CommandExecuteAndWait();
	dxCommand->CommandReset();

	// ReadbackしてDirectXTexに詰め替え
	D3D12_RANGE range{ 0, static_cast<SIZE_T>(totalBytes) };
	void* mapped = nullptr;
	readback.Get()->Map(0, &range, &mapped);

	DirectX::ScratchImage volumeScratch;
	volumeScratch.Initialize3D(desc.Format, desc.Width, desc.Height, desc.DepthOrArraySize, desc.MipLevels);

	// コピー元（Readbackバッファ）のRowPitch
	const size_t srcRowPitch = static_cast<size_t>(footprint.Footprint.RowPitch);
	// スライスごとのサイズ（パディング込み）
	const size_t srcSlicePitch = srcRowPitch * footprint.Footprint.Height;

	const uint8_t* srcStart = reinterpret_cast<uint8_t*>(mapped) + footprint.Offset;

	for(UINT z = 0; z < desc.DepthOrArraySize; z++) {
		// コピー先のイメージ情報を取得
		const DirectX::Image* dstImage = volumeScratch.GetImage(0, 0, z);

		// 現在のスライスの先頭ポインタ
		const uint8_t* srcSlicePtr = srcStart + (z * srcSlicePitch);
		uint8_t* dstSlicePtr = dstImage->pixels;

		// 1行あたりの有効バイト数（フォーマットに基づく最小サイズ）
		// dstImage->rowPitch と srcRowPitch の小さい方、あるいは明示的に計算しても良いですが
		// 基本的には dstImage->width とフォーマットから計算されるサイズをコピーします。
		// 安全のため、コピー先とコピー元の小さい方に合わせるのが定石です。
		const size_t bytesPerRow = std::min<size_t>(dstImage->rowPitch, srcRowPitch);

		// 高さ（行）ループ
		for(size_t y = 0; y < dstImage->height; ++y) {
			memcpy(
				dstSlicePtr + (y * dstImage->rowPitch), // コピー先アドレス
				srcSlicePtr + (y * srcRowPitch),        // コピー元アドレス（256バイトアラインメント考慮）
				bytesPerRow                             // パディングを含まない有効データサイズ
			);
		}
	}

	DirectX::SaveToDDSFile(
		volumeScratch.GetImages(),
		volumeScratch.GetImageCount(),
		volumeScratch.GetMetadata(),
		DirectX::DDS_FLAGS_NONE,
		filename.c_str()
	);

	readback.Get()->Unmap(0, nullptr);
	volumeScratch.Release();
}

void Texture::ResizeTexture3D(const Vector2& newSize, UINT newDepth, DxDevice* dxDevice, DxCommand* dxCommand, DxSRVHeap* dxSRVHeap) {
	Assert(dxResource_.Get());

	// 旧リソース情報
	D3D12_RESOURCE_DESC oldDesc = dxResource_.Get()->GetDesc();

	Assert(oldDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);

	const UINT oldWidth = static_cast<UINT>(oldDesc.Width);
	const UINT oldHeight = oldDesc.Height;
	const UINT oldDepth = oldDesc.DepthOrArraySize;

	const UINT newWidth = static_cast<UINT>(newSize.x);
	const UINT newHeight = static_cast<UINT>(newSize.y);
	const UINT newDepthSize = newDepth;

	// コピー可能な最小範囲
	const UINT copyWidth = (std::min)(oldWidth, newWidth);
	const UINT copyHeight = (std::min)(oldHeight, newHeight);
	const UINT copyDepth = (std::min)(oldDepth, newDepthSize);

	// 新しい Texture3D 作成
	D3D12_RESOURCE_DESC newDesc = {};
	newDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	newDesc.Width = newWidth;
	newDesc.Height = newHeight;
	newDesc.DepthOrArraySize = static_cast<UINT16>(newDepthSize);
	newDesc.MipLevels = 1;
	newDesc.Format = oldDesc.Format;
	newDesc.SampleDesc.Count = 1;
	newDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	newDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	DxResource newResource;
	newResource.CreateCommittedResource(
		dxDevice,
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&newDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr
	);

	// バリア
	dxResource_.CreateBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, dxCommand);

	// CopyTextureRegion（3D）
	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = dxResource_.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = newResource.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	D3D12_BOX box = {};
	box.left = 0;
	box.top = 0;
	box.front = 0;
	box.right = copyWidth;
	box.bottom = copyHeight;
	box.back = copyDepth;

	dxCommand->GetCommandList()->CopyTextureRegion(
		&dst, 0, 0, 0,
		&src, &box
	);

	// 実行
	dxCommand->CommandExecuteAndWait();
	dxCommand->CommandReset();

	// 旧リソース破棄 → 新リソース差し替え
	dxResource_ = std::move(newResource);
	depth_ = newDepth;
	textureSize_ = newSize;

	// UAV 再生成（既存 DescriptorIndex を再利用）
	if(uavHandle_.has_value()) {
		CreateUAVTexture3DWithUAV(
			newWidth,
			newHeight,
			newDepth,
			dxDevice,
			dxSRVHeap,
			uavFormat_
		);
	}

	Console::Log("[ResizeTexture3D]");
	Console::Log(" - Old: " + std::to_string(oldWidth) + "x" +
				 std::to_string(oldHeight) + "x" + std::to_string(oldDepth));
	Console::Log(" - New: " + std::to_string(newWidth) + "x" +
				 std::to_string(newHeight) + "x" + std::to_string(newDepth));
}



void Texture::SetName(const std::string& name) {
	name_ = name;
	if(dxResource_.Get()) {
		dxResource_.Get()->SetName(std::wstring(name.begin(), name.end()).c_str());
	}
}


void Texture::SetSRVHandle(const Handle& handle) {
	srvHandle_ = handle;
}

void Texture::SetUAVHandle(const Handle& handle) {
	uavHandle_ = handle;
}

void Texture::SetSRVHandle(uint32_t descriptorIndex, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) {
	srvHandle_ = Handle{ descriptorIndex, cpuHandle, gpuHandle };
}

void Texture::SetUAVHandle(uint32_t descriptorIndex, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) {
	uavHandle_ = Handle{ descriptorIndex, cpuHandle, gpuHandle };
}

void Texture::SetSRVDescriptorIndex(uint32_t index) {
	srvHandle_->descriptorIndex = index;
}

void Texture::SetSRVCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) {
	srvHandle_->cpuHandle = cpuHandle;
}

void Texture::SetSRVGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) {
	srvHandle_->gpuHandle = gpuHandle;
}

void Texture::SetUAVDescriptorIndex(uint32_t index) {
	uavHandle_->descriptorIndex = index;
}

void Texture::SetUAVCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) {
	uavHandle_->cpuHandle = cpuHandle;
}

void Texture::SetUAVGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) {
	uavHandle_->gpuHandle = gpuHandle;
}

const Texture::Handle& Texture::GetSRVHandle() const {
	Assert(srvHandle_.has_value());
	return srvHandle_.value();
}

const Texture::Handle& Texture::GetUAVHandle() const {
	Assert(uavHandle_.has_value());
	return uavHandle_.value();
}

uint32_t Texture::GetSRVDescriptorIndex() const {
	Assert(srvHandle_.has_value());
	return srvHandle_->descriptorIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetSRVCPUHandle() const {
	Assert(srvHandle_.has_value());
	return srvHandle_->cpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetSRVGPUHandle() const {
	Assert(srvHandle_.has_value());
	return srvHandle_->gpuHandle;
}

bool Texture::HasSRVHandle() const {
	return srvHandle_.has_value();
}

uint32_t Texture::GetUAVDescriptorIndex() const {
	Assert(uavHandle_.has_value());
	return uavHandle_->descriptorIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetUAVCPUHandle() const {
	Assert(uavHandle_.has_value());
	return uavHandle_->cpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetUAVGPUHandle() const {
	Assert(uavHandle_.has_value());
	return uavHandle_->gpuHandle;
}

bool Texture::HasUAVHandle() const {
	return uavHandle_.has_value();
}

const DxResource& Texture::GetDxResource() const {
	return dxResource_;
}

DxResource& Texture::GetDxResource() {
	return dxResource_;
}

const Vector2& Texture::GetTextureSize() const {
	return textureSize_;
}

UINT Texture::GetTextureDepth() const {
	return depth_;
}



void SaveTextureToPNG(const std::wstring& filename, size_t width, size_t height, bool overwrite) {

	/// filenameの先のディレクトリが存在しない場合は作成
	std::filesystem::path filePath(filename);
	if(!std::filesystem::exists(filePath.parent_path())) {
		std::filesystem::create_directories(filePath.parent_path());
	} else {
		// ファイルが既に存在しているかチェック
		if(std::filesystem::is_regular_file(filePath)) {
			/// あった場合上書きするのかどうか
			if(!overwrite) {
				return;
			}
		}
	}


	// テクスチャのサイズとフォーマット
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	const size_t rowPitch = width * 4;       // 1ピクセル4バイト（RGBA）
	const size_t slicePitch = rowPitch * height;

	std::vector<uint8_t> pixelData(slicePitch);
	for(size_t y = 0; y < height; ++y) {
		for(size_t x = 0; x < width; ++x) {
			size_t index = y * rowPitch + x * 4;
			pixelData[index + 0] = 255; // R
			pixelData[index + 1] = 255; // G
			pixelData[index + 2] = 255; // B
			pixelData[index + 3] = 255; // A
		}
	}

	// ScratchImage を作成
	DirectX::ScratchImage image;
	HRESULT hr = image.Initialize2D(format, width, height, 1, 1);
	Assert(SUCCEEDED(hr));

	// ピクセルデータをコピー
	const DirectX::Image* img = image.GetImage(0, 0, 0);
	std::memcpy(img->pixels, pixelData.data(), slicePitch);

	// PNG 形式で保存
	hr = DirectX::SaveToWICFile(*img, DirectX::WIC_FLAGS_NONE, GUID_ContainerFormatPng, filename.c_str());
	Assert(SUCCEEDED(hr));

}

void SaveTextureToDDS(const std::wstring& filename, size_t width, size_t height, size_t depth, bool overwrite) {

	/// filenameの先のディレクトリが存在しない場合は作成
	std::filesystem::path filePath(filename);
	if(!std::filesystem::exists(filePath.parent_path())) {
		std::filesystem::create_directories(filePath.parent_path());
	} else {
		// ファイルが既に存在しているかチェック
		if(std::filesystem::is_regular_file(filePath)) {
			/// あった場合上書きするのかどうか
			if(!overwrite) {
				return;
			}
		}
	}


	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	size_t rowPitch = width * 4;

	// 3Dテクスチャ用 ScratchImage
	DirectX::ScratchImage volumeImage;
	HRESULT hr = volumeImage.Initialize3D(format, width, height, depth, 1);
	Assert(SUCCEEDED(hr));

	// 仮のデータで埋める（上半分を透明にする）
	for(size_t z = 0; z < depth; ++z) {

		// 0.0 ～ 1.0 のグラデーション係数
		float t = static_cast<float>(z) / static_cast<float>(depth - 1);

		const DirectX::Image* img = volumeImage.GetImage(0, 0, z);
		uint8_t* dst = img->pixels;

		for(size_t y = 0; y < height; ++y) {
			for(size_t x = 0; x < width; ++x) {

				size_t index = y * rowPitch + x * 4;

				// t に応じたグラデーション
				uint8_t r = static_cast<uint8_t>(255 * t);        // 黒 → 赤
				uint8_t g = static_cast<uint8_t>(128 * (1 - t));  // 緑成分が減る例
				uint8_t b = static_cast<uint8_t>(255 * (1 - t));  // 青 → 黒

				// 上半分を透明にする
				uint8_t a = (y < height / 2) ? 0 : 255;
				if(a == 0) {
					r = 0;
					g = 0;
					b = 0;
				}

				dst[index + 0] = r;
				dst[index + 1] = g;
				dst[index + 2] = b;
				dst[index + 3] = a;
			}
		}
	}

	// DDS 保存
	hr = DirectX::SaveToDDSFile(
		volumeImage.GetImages(),
		volumeImage.GetImageCount(),
		volumeImage.GetMetadata(),
		DirectX::DDS_FLAGS_NONE,
		filename.c_str()
	);

	Assert(SUCCEEDED(hr));

}

} /// namespace ONEngine::Asset