#pragma once

/// std
#include <optional>
#include <string>

/// engine
#include "../IAsset.h"
#include "Engine/Core/DirectX12/Resource/DxResource.h"



namespace ONEngine::Asset {
template<typename T>
class AssetLoader;
}

namespace ONEngine {
class DxDevice;
class DxSRVHeap;
class DxCommand;
}



namespace ONEngine::Asset {


/// @brief テクスチャのフォーマットの種類
enum class TextureFormat {
	RGBA8_UNORM,
	RGBA8_SRGB,
	RGBA16_FLOAT,
	RGBA32_FLOAT,
	RGBA32_UINT,
};

/// @brief 色空間の種類
enum class ColorSpace {
	Linear,
	sRGB
};

void from_json(const nlohmann::json& j, TextureFormat& format);
void to_json(nlohmann::json& j, const TextureFormat& format);

void from_json(const nlohmann::json& j, ColorSpace& colorSpace);
void to_json(nlohmann::json& j, const ColorSpace& colorSpace);

/// ///////////////////////////////////////////////////
/// texture
/// ///////////////////////////////////////////////////
class Texture final : public IAsset {
	friend class AssetLoader<Texture>;
public:
	/// ===================================================
	/// public : sub class
	/// ===================================================

	struct MetaData {
		TextureFormat format;
		ColorSpace colorSpace;
	};

	struct Handle {
		uint32_t descriptorIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	};

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	Texture();
	Texture(const Vector2& textureSize);
	~Texture() override = default;

	/// @brief SRVHandleの空の状態を作成する
	void CreateEmptySRVHandle();

	/// @brief UAVHandleの空の状態を作成する
	void CreateEmptyUAVHandle();


	/// @brief UAVTextureとして作成する
	/// @param width テクスチャの幅
	/// @param height テクスチャの高さ
	/// @param dxDevice DxDeviceへのポインタ
	/// @param dxSRVHeap DxSRVHeapへのポインタ
	/// @param dxgiFormat DXGI_FORMAT
	void CreateUAVTexture(UINT width, UINT height, DxDevice* dxDevice, DxSRVHeap* dxSRVHeap, DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT);

	/// @brief メモリ上のピクセルデータからテクスチャを再構築する（動的変更用）
	void RecreateFromPixels(const uint8_t* pixels, int width, int height, DxDevice* dxDevice, DxSRVHeap* dxSRVHeap, DxCommand* dxCommand);

	/// @brief UAVTexture3Dとして作成する
	/// @param width テクスチャの幅
	/// @param height テクスチャの高さ
	/// @param depth テクスチャの奥行き
	/// @param dxDevice DxDeviceへのポインタ
	/// @param dxSRVHeap DxSRVHeapへのポインタ
	/// @param dxgiFormat DXGI_FORMAT
	void CreateUAVTexture3DWithUAV(UINT width, UINT height, UINT depth, DxDevice* dxDevice, DxSRVHeap* dxSRVHeap, DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT);

	/// @brief 3Dテクスチャに対してUAVの状態を追加する
	/// @param width テクスチャの幅
	/// @param height テクスチャの高さ
	/// @param depth テクスチャの奥行き
	/// @param dxDevice DxDeviceへのポインタ
	/// @param dxSRVHeap DxSRVHeapへのポインタ
	/// @param dxgiFormat DXGI_FORMAT
	void CreateUAVTexture3D(UINT width, UINT height, UINT depth, DxDevice* dxDevice, DxSRVHeap* dxSRVHeap, DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT);


	/// @brief テクスチャをファイルに出力する
	/// @param filename ファイル名(パス、拡張子込み)
	/// @param dxDevice DxDeviceへのポインタ
	/// @param dxCommand DxCommandへのポインタ
	void OutputTexture(const std::wstring& filename, DxDevice* dxDevice, DxCommand* dxCommand);
	void OutputTexture3D(const std::wstring& filename, DxDevice* dxDevice, DxCommand* dxCommand);

	void ResizeTexture3D(const Vector2& newSize, UINT newDepth, DxDevice* dxDevice, DxCommand* dxCommand, DxSRVHeap* dxSRVHeap);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::string name_;

	DxResource dxResource_;
	DxResource readbackTexture_;

	std::optional<Handle> srvHandle_;
	std::optional<Handle> uavHandle_;

	Vector2 textureSize_;
	UINT depth_ = 0; // 3Dテクスチャ用
	UINT arraySize_ = 1; // 配列テクスチャ用
	bool isCubeMap_ = false;

	/// テクスチャのフォーマット、UAVを作成する際に必要
	DXGI_FORMAT srvFormat_;
	DXGI_FORMAT uavFormat_;

public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	/// ----- setters ----- ///

	void SetName(const std::string& name);

	/// Handle(cpu, gpu, heap index) を設定
	void SetSRVHandle(const Handle& handle);
	void SetUAVHandle(const Handle& handle);

	/// Handle(cpu, gpu, heap index) を設定
	void SetSRVHandle(uint32_t descriptorIndex, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);
	void SetUAVHandle(uint32_t descriptorIndex, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	/// descriptor index, cpu handle, gpu handle を個別に設定
	void SetSRVDescriptorIndex(uint32_t index);
	void SetSRVCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);
	void SetSRVGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	/// descriptor index, cpu handle, gpu handle を個別に設定
	void SetUAVDescriptorIndex(uint32_t index);
	void SetUAVCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);
	void SetUAVGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);


	/// ----- getters ----- ///

	const Handle& GetSRVHandle() const;
	const Handle& GetUAVHandle() const;

	uint32_t GetSRVDescriptorIndex() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUHandle() const;
	bool HasSRVHandle() const;

	uint32_t GetUAVDescriptorIndex() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetUAVCPUHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetUAVGPUHandle() const;
	bool HasUAVHandle() const;

	const DxResource& GetDxResource() const;
	DxResource& GetDxResource();

	const Vector2& GetTextureSize() const;
	UINT GetTextureDepth() const;
	bool IsCubeMap() const { return isCubeMap_; }
	bool IsStandard2D() const { return !isCubeMap_ && depth_ == 0 && arraySize_ == 1; }

};


/// @brief TextureをPNG形式で保存する
/// @param filename ファイル名
/// @param width テクスチャの幅
/// @param height テクスチャの高さ
/// @param overwrite 上書き保存するかどうか
void SaveTextureToPNG(const std::wstring& filename, size_t width, size_t height, bool overwrite);

/// @brief テクスチャをDDS形式で保存する
/// @param filename ファイル名
/// @param width テクスチャの幅
/// @param height テクスチャの高さ
/// @param depth テクスチャの奥行き
/// @param overwrite 上書き保存するかどうか
void SaveTextureToDDS(const std::wstring& filename, size_t width, size_t height, size_t depth, bool overwrite);

} /// ONEngine
