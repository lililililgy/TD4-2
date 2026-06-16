#pragma once

/// directX
#include <d3d12.h>
#include <d3dx12.h>

/// engine
#include "../ComPtr/ComPtr.h"
#include "Engine/Core/Utility/Math/Vector2.h"
#include "Engine/Core/Utility/Math/Vector4.h"

/// /////////////////////////////////////////////////
/// ID3D12Resourceのラッパークラス
/// /////////////////////////////////////////////////
namespace ONEngine {

class DxResource final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	DxResource();
	~DxResource();

	/// @brief bufferを作成する
	/// @param dxDevice デバイスオブジェクトへのポインタ
	/// @param sizeInByte バッファのサイズ（バイト単位）
	void CreateResource(class DxDevice* dxDevice, size_t sizeInByte);
	void CreateUAVResource(class DxDevice* dxDevice, class DxCommand* dxCommand, size_t sizeInByte);
	void CreateDefaultHeap(class DxDevice* dxDevice, class DxCommand* dxCommand, size_t sizeInByte, D3D12_RESOURCE_STATES initialState);
	void CreateUploadHeap(class DxDevice* dxDevice, class DxCommand* dxCommand, size_t sizeInByte, D3D12_RESOURCE_STATES initialState);

	/// @brief resourceを作成する
	/// @param dxDevice デバイスオブジェクトへのポインタ
	/// @param pHeapProperties ヒーププロパティ
	/// @param heapFlags ヒープフラグ
	/// @param pDesc リソースの記述子
	/// @param initialResourceState 初期リソース状態
	/// @param pOptimizedClearValue 最適化されたクリア値
	void CreateCommittedResource(
		class DxDevice* dxDevice,
		const D3D12_HEAP_PROPERTIES* pHeapProperties,
		D3D12_HEAP_FLAGS heapFlags,
		const D3D12_RESOURCE_DESC* pDesc,
		D3D12_RESOURCE_STATES initialResourceState,
		const D3D12_CLEAR_VALUE* pOptimizedClearValue
	);

	/// @brief render texture resourceを作成する
	/// @param dxDevice デバイスオブジェクトへのポインタ
	/// @param size render textureのサイズ
	/// @param format 書き込みフォーマット
	/// @param clearColor クリアカラー
	void CreateRenderTextureResource(
		class DxDevice* dxDevice,
		const Vector2& size,
		DXGI_FORMAT format,
		const Vector4& clearColor
	);

	/// @brief unordered access view texture resourceを作成する
	/// @param dxDevice DxDeviceのインスタンスへのポインタ
	/// @param size texture size
	/// @param format dxgi format
	void CreateUAVTextureResource(
		class DxDevice* dxDevice,
		const Vector2& size,
		DXGI_FORMAT format
	);

	/// @brief リソースのバリアーを作成する
	/// @param before 前の状態
	/// @param after 変更後の状態
	/// @param dxCommand DxCommandのインスタンスへのポインタ
	void CreateBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, class DxCommand* dxCommand);
	void CreateBarrier(D3D12_RESOURCE_STATES after, class DxCommand* dxCommand);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	ComPtr<ID3D12Resource> resource_ = nullptr;
	D3D12_RESOURCE_STATES  currentState_;

public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	/// @brief リソースオブジェクトを取得する
	/// @return ID3D12Resourceオブジェクトへのポインタ
	ID3D12Resource* Get() const;
	ComPtr<ID3D12Resource>& GetComPtr();

	/// @brief 現在のリソース状態を取得する
	/// @return リソース状態
	D3D12_RESOURCE_STATES GetCurrentState() const;

	/// @brief 現在のステートを変更する(UAVの作成など強制的に状態が変更される場合のみ使用する)
	/// @param state 変更先のステート
	void SetCurrentState(D3D12_RESOURCE_STATES state);

};


std::wstring GetD3D12Name(ID3D12Object* object);

/// ===================================================
/// Barrierを作成する関数
/// ===================================================
void CreateBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, class DxCommand* dxCommand);

void CreateBarriers(std::vector<DxResource*>& resources, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, class DxCommand* dxCommand);

void CreateBarriers(std::vector<DxResource*>& resources, D3D12_RESOURCE_STATES after, class DxCommand* dxCommand);

} /// ONEngine
