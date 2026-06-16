#pragma once

/// directX
#include <d3d12.h>

/// std
#include <cstdint>
#include <deque>

/// engine
#include "../ComPtr/ComPtr.h"
#include "../Device/DxDevice.h"



/// /////////////////////////////////////////////////
/// DescriptorHeapの種類
/// /////////////////////////////////////////////////
enum DescriptorHeapType {
	DescriptorHeapType_RTV,                        /// render target view用
	DescriptorHeapType_CBV_SRV_UAV,                /// cbv, srv, uav用
	DescriptorHeapType_DSV,                        /// depth stencil view用
	DescriptorHeapType_COUNT                       /// 種類数
};



/// /////////////////////////////////////////////////
/// DescriptorHeapの基底クラス
/// /////////////////////////////////////////////////
namespace ONEngine {

class IDxDescriptorHeap {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	IDxDescriptorHeap(DxDevice* dxDevice, uint32_t maxHeapSize);
	virtual ~IDxDescriptorHeap() = default;

	/// @brief 初期化
	virtual void Initialize() = 0;

	/// @brief indexのDescriptorHeapを解放する
	/// @param index 解放したいDescriptorのIndex
	void Free(uint32_t index);

	/// @brief DescriptorHeapのindexを取得する
	/// @return DescriptorHeapのindex
	uint32_t Allocate();

	/// @brief commandListにDescriptorHeapをバインドする
	/// @param commandList ID3D12GraphicsCommandListのポインタ
	void BindToCommandList(ID3D12GraphicsCommandList* commandList);


protected:
	/// ===================================================
	/// protected : objects
	/// ===================================================

	ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;

	const uint32_t               kMaxHeapSize_;             ///< heapのmax
	uint32_t                     descriptorSize_;           ///< heapの1つあたりsize

	uint32_t                     useIndex_;
	std::deque<uint32_t>         spaceIndex_;               ///< 解放された後の空きindex

	DxDevice*                    pDxDevice_ = nullptr;


public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	/// @brief cpu handleのゲッタ
	/// @param index ゲットしたいindex
	/// @return cpu handle
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandel(uint32_t index) const;

	/// @brief gpu handleのゲッタ
	/// @param index ゲットしたいindex
	/// @return gpu handle
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandel(uint32_t index) const;

	/// @brief descriptor heapのゲッタ
	/// @return descriptor heapへのポインタ
	ID3D12DescriptorHeap* GetHeap() const;

	/// @brief descriptor heapの最大サイズのゲッタ
	/// @return descriptor heapの最大サイズ
	uint32_t GetMaxHeapSize() const;

	/// @brief 使用済みのindexの数を取得する
	/// @return 使用済みのindexの数
	uint32_t GetUsedIndexCount() const;


private:

	/// ===================================================
	/// private : copy delete
	/// ===================================================

	IDxDescriptorHeap(const IDxDescriptorHeap&)            = delete;
	IDxDescriptorHeap(IDxDescriptorHeap&&)                 = delete;
	IDxDescriptorHeap& operator=(const IDxDescriptorHeap&) = delete;
	IDxDescriptorHeap& operator=(IDxDescriptorHeap&&)      = delete;
};


/// @brief DescriptorHeapの生成
/// @param device device へのポインタ
/// @param type heapの種類
/// @param numDescriptors descriptorの個数 
/// @param isShaderVisible shader visibleかどうか
/// @return 生成された DescriptorHeap
ComPtr<ID3D12DescriptorHeap> CreateHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool isShaderVisible);

} /// ONEngine
