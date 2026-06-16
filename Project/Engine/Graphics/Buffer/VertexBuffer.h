#pragma once

/// std
#include <vector>

/// engine
#include "Engine/Core/DirectX12/Resource/DxResource.h"


/// ///////////////////////////////////////////////////
/// index buffer view
/// ///////////////////////////////////////////////////
namespace ONEngine {

template<typename T>
class VertexBuffer final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	VertexBuffer() = default;
	~VertexBuffer() = default;

	/// @brief Bufferの作成
	/// @param vertexSize 頂点の数
	/// @param dxDevice DxDeviceのポインタ
	void Create(size_t vertexSize, class DxDevice* dxDevice, DxCommand* dxCommand);


	/// @brief vertices_のメモリ確保
	/// @param value サイズ
	void Reserve(size_t value);

	/// @brief vertices_のリサイズ
	/// @param value サイズ
	void Resize(size_t value);


	/// @brief コマンドリストにバインドする
	/// @param commandList 
	void BindForCommandList(ID3D12GraphicsCommandList* commandList);


	/// @brief GPUにマッピングする
	void Map();

	void CopyFromAppendBuffer(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* appendBuffer, uint32_t vertexCount);
	void CopyFromUAVBuffer(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* uavBuffer, uint32_t vertexCount);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	DxResource resource_;
	std::vector<T> vertices_;
	D3D12_VERTEX_BUFFER_VIEW vbv_;
	T* mappingData_ = nullptr;

public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	const std::vector<T>& GetVertices() const;

	void SetVertex(size_t index, const T& vertex);
	void SetVertices(const std::vector<T>& vertices);

};



template<typename T>
inline void VertexBuffer<T>::Create(size_t vertexSize, DxDevice* dxDevice, DxCommand* dxCommand) {
	size_t tSize = sizeof(T);

	Resize(vertexSize);
	/// vertex buffer
	resource_.CreateUploadHeap(dxDevice, dxCommand, tSize * vertices_.size(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	vbv_.BufferLocation = resource_.Get()->GetGPUVirtualAddress();
	vbv_.SizeInBytes = static_cast<UINT>(tSize * vertices_.size());
	vbv_.StrideInBytes = static_cast<UINT>(tSize);
}

template<typename T>
inline void VertexBuffer<T>::Reserve(size_t value) {
	vertices_.reserve(value);
}

template<typename T>
inline void VertexBuffer<T>::Resize(size_t value) {
	vertices_.resize(value);
}

template<typename T>
inline void VertexBuffer<T>::BindForCommandList(ID3D12GraphicsCommandList* commandList) {
	commandList->IASetVertexBuffers(0, 1, &vbv_);
}

template<typename T>
inline void VertexBuffer<T>::Map() {
	resource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));
	std::memcpy(mappingData_, vertices_.data(), sizeof(T) * vertices_.size());
}

template<typename T>
inline void VertexBuffer<T>::CopyFromAppendBuffer(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* appendBuffer, uint32_t vertexCount) {
	// リソースサイズチェック
	UINT64 copySize = static_cast<UINT64>(vertexCount * sizeof(T));
	Assert(copySize <= vbv_.SizeInBytes, "VertexBuffer size is too small for AppendBuffer copy");

	// バリア: VBV -> COPY_DEST
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		resource_.Get(),
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_COPY_DEST
	);
	cmdList->ResourceBarrier(1, &barrier);

	// Copy
	cmdList->CopyBufferRegion(
		resource_.Get(), 0,       // Destination
		appendBuffer, 0,          // Source
		copySize
	);

	// バリア戻す: COPY_DEST -> VBV
	std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
	cmdList->ResourceBarrier(1, &barrier);

	// VBV の SizeInBytes をコピー頂点数に更新
	vbv_.SizeInBytes = static_cast<UINT>(copySize);
}

template<typename T>
inline void VertexBuffer<T>::CopyFromUAVBuffer(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* uavBuffer, uint32_t vertexCount) {
	// リソースサイズチェック
	UINT64 copySize = static_cast<UINT64>(vertexCount * sizeof(T));
	Assert(copySize <= vbv_.SizeInBytes, "VertexBuffer size is too small for UAVBuffer copy");

	// バリア: UAV -> COPY_SOURCE, VBV -> COPY_DEST
	D3D12_RESOURCE_BARRIER barriers[2] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			uavBuffer,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_SOURCE
		),
			CD3DX12_RESOURCE_BARRIER::Transition(
				resource_.Get(),
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
				D3D12_RESOURCE_STATE_COPY_DEST
			)
	};
	cmdList->ResourceBarrier(2, barriers);

	// Copy
	cmdList->CopyBufferRegion(
		resource_.Get(), 0,       // Destination
		uavBuffer, 0,            // Source
		copySize
	);

	// バリア戻す: COPY_SOURCE -> UAV, COPY_DEST -> VBV
	std::swap(barriers[0].Transition.StateBefore, barriers[0].Transition.StateAfter);
	std::swap(barriers[1].Transition.StateBefore, barriers[1].Transition.StateAfter);
	cmdList->ResourceBarrier(2, barriers);

	// VBV の SizeInBytes をコピー頂点数に更新
	vbv_.SizeInBytes = static_cast<UINT>(copySize);
}

template<typename T>
inline const std::vector<T>& VertexBuffer<T>::GetVertices() const {
	return vertices_;
}

template<typename T>
inline void VertexBuffer<T>::SetVertex(size_t index, const T& vertex) {
	if(mappingData_) {
		mappingData_[index] = vertex;
	}
	vertices_[index] = vertex;
}

template<typename T>
inline void VertexBuffer<T>::SetVertices(const std::vector<T>& vertices) {
	vertices_ = vertices;
}

} /// ONEngine
