#pragma once

/// std
#include <vector>
#include <span>

/// engine
#include "Engine/Core/DirectX12/ComPtr/ComPtr.h"
#include "Engine/Core/DirectX12/Resource/DxResource.h"
#include "Engine/Core/DirectX12/Device/DxDevice.h"
#include "Engine/Core/DirectX12/DescriptorHeap/DxSRVHeap.h"

/// //////////////////////////////////////////////////////
/// ByteAddressBuffer
/// //////////////////////////////////////////////////////
namespace ONEngine {

class ByteAddressBuffer final {
public:
	/// ==================================================
	/// public : methods
	/// ==================================================

	ByteAddressBuffer();
	~ByteAddressBuffer();

	/// @brief Bufferを作成する
	/// @param size Bufferのサイズ
	/// @param dxDevice DxDeviceのポインタ
	/// @param dxSRVHeap DxSRVHeapのポインタ
	void Create(uint32_t size, DxDevice* dxDevice, DxSRVHeap* dxSRVHeap);


	/// @brief 指定したインデックスに対応するマップされたデータの値を設定する
	/// @param index 設定対象のデータのインデックス
	/// @param value 設定する値（32ビット符号なし整数）
	void SetMappedData(size_t index, uint32_t value);

	/// @brief コマンドリストにバインドする
	/// @param rootParameterIndex パラメータインデックス
	/// @param commandList CommandListのポインタ
	void BindToCommandList(UINT rootParameterIndex, ID3D12GraphicsCommandList* commandList);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	uint32_t                    srvDescriptorIndex_;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_;

	DxResource                  bufferResource_;
	uint32_t*                   mappedData_;
	std::span<uint32_t>         mappedDataArray_;

	size_t                      totalSize_;
	size_t                      bufferSize_;

	DxSRVHeap* pDxSRVHeap_;


};

} /// ONEngine
