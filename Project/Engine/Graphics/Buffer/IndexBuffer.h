#pragma once

/// std
#include <vector>

/// engine
#include "Engine/Core/DirectX12/Resource/DxResource.h"


/// ///////////////////////////////////////////////////
/// index buffer view
/// ///////////////////////////////////////////////////
namespace ONEngine {

class IndexBuffer final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	IndexBuffer();
	~IndexBuffer();

	/// @brief Bufferを作成する
	/// @param indicesSize Indexの数
	/// @param dxDevice DxDeviceへのポインタ
	void Create(size_t indicesSize, class DxDevice* dxDevice);

	/// @brief Indices用のメモリを確保する
	/// @param value サイズ
	void Reserve(size_t value);

	/// @brief Indices用のサイズを変更する
	/// @param value サイズ
	void Resize(size_t value);


	/// @brief コマンドリストにバインドする
	/// @param commandList バインド対象のコマンドリスト
	void BindForCommandList(ID3D12GraphicsCommandList* commandList);


	/// @brief GPU用にマッピングする
	void Map();

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	DxResource resource_;
	std::vector<uint32_t> indices_;
	D3D12_INDEX_BUFFER_VIEW ibv_;


public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	const std::vector<uint32_t>& GetIndices() const;

	void SetIndex(size_t indexIndex, uint32_t indexValue);
	void SetIndices(const std::vector<uint32_t>& indices);

};


} /// ONEngine
