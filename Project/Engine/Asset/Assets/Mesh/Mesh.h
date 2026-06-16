#pragma once

/// directX
#include <d3d12.h>

/// std
#include <cstdint>
#include <vector>

/// engine
#include "Engine/Core/DirectX12/Resource/DxResource.h"
#include "Engine/Core/Utility/Math/Vector4.h"
#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Vector2.h"


namespace ONEngine {

class DxDevice;

/// ///////////////////////////////////////////////////
/// graphics resource の mesh data
/// ///////////////////////////////////////////////////
template<typename Vertex>
class Mesh final {
public:

	/// ===================================================
	/// public : methods
	/// ===================================================

	Mesh() = default;
	~Mesh() = default;

	/// @brief vertex buffer, index bufferの作成
	/// @param dxDevice DxDeviceクラスへのポインタ
	void CreateBuffer(DxDevice* dxDevice);

	/// @brief vertex bufferをマッピング
	void VertexBufferMapping();

	/// @brief index bufferをマッピング
	void IndexBufferMapping();

	void MemcpyVertexData();

private:

	/// ===================================================
	/// private : objects
	/// ===================================================

	std::vector<Vertex>      vertices_;
	DxResource               vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbv_;
	Vertex*                  mappingVertexData_;

	std::vector<uint32_t>    indices_;
	DxResource               indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW  ibv_;


public:

	/// ===================================================
	/// public : accessor
	/// ===================================================

	/// @brief vertices_をセット
	/// @param vertices 頂点データ配列
	void SetVertices(const std::vector<Vertex>& vertices);

	/// @brief 頂点のインデックスをセット
	/// @param indices 頂点インデックス配列
	void SetIndices(const std::vector<uint32_t>& indices);


	/// @brief 頂点データを取得
	/// @return 頂点データの配列
	const std::vector<Vertex>& GetVertices() const;

	/// @brief 頂点インデックスを取得
	/// @return 頂点インデックスの配列
	const std::vector<uint32_t>& GetIndices() const;

	/// @brief vertex buffer viewを取得
	/// @return vbv date
	const D3D12_VERTEX_BUFFER_VIEW& GetVBV() const;

	/// @brief index buffer viewを取得
	/// @return ibv date
	const D3D12_INDEX_BUFFER_VIEW& GetIBV() const;

};



template<typename Vertex>
void Mesh<Vertex>::CreateBuffer(DxDevice* dxDevice) {
	/// ----- vbv, ibv のBufferを生成、Mapする ----- ///

	const size_t kVertexDataSize = sizeof(Vertex);

	/// vertex buffer
	vertexBuffer_.CreateResource(dxDevice, kVertexDataSize * vertices_.size());
	vbv_.BufferLocation = vertexBuffer_.Get()->GetGPUVirtualAddress();
	vbv_.SizeInBytes = static_cast<UINT>(kVertexDataSize * vertices_.size());
	vbv_.StrideInBytes = static_cast<UINT>(kVertexDataSize);

	/// index buffer
	indexBuffer_.CreateResource(dxDevice, sizeof(uint32_t) * indices_.size());
	ibv_.BufferLocation = indexBuffer_.Get()->GetGPUVirtualAddress();
	ibv_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	ibv_.Format = DXGI_FORMAT_R32_UINT;


	/// mapping
	VertexBufferMapping();
	IndexBufferMapping();
}

template<typename Vertex>
void Mesh<Vertex>::VertexBufferMapping() {
	/// ----- 頂点データをGPUで使用するためにMapする ----- ///
	vertexBuffer_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappingVertexData_));
	std::memcpy(mappingVertexData_, vertices_.data(), sizeof(Vertex) * vertices_.size());
}

template<typename Vertex>
void Mesh<Vertex>::IndexBufferMapping() {
	/// ----- インデックスデータをGPUで使用するためにMapする ----- ///
	uint32_t* mappingData = nullptr;
	indexBuffer_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData));
	std::memcpy(mappingData, indices_.data(), sizeof(uint32_t) * indices_.size());
}

template<typename Vertex>
void Mesh<Vertex>::MemcpyVertexData() {
	/// ----- 頂点データを再度コピーする ※一度VertexBufferMapping()でMapしないと成功しない　----- ///
	if(mappingVertexData_) {
		std::memcpy(mappingVertexData_, vertices_.data(), sizeof(Vertex) * vertices_.size());
	}
}


template<typename Vertex>
void Mesh<Vertex>::SetVertices(const std::vector<Vertex>& vertices) {
	vertices_ = vertices;
}

template<typename Vertex>
void Mesh<Vertex>::SetIndices(const std::vector<uint32_t>& indices) {
	indices_ = indices;
}

template<typename Vertex>
const std::vector<Vertex>& Mesh<Vertex>::GetVertices() const {
	return vertices_;
}

template<typename Vertex>
const std::vector<uint32_t>& Mesh<Vertex>::GetIndices() const {
	return indices_;
}

template<typename Vertex>
const D3D12_VERTEX_BUFFER_VIEW& Mesh<Vertex>::GetVBV() const {
	return vbv_;
}

template<typename Vertex>
const D3D12_INDEX_BUFFER_VIEW& Mesh<Vertex>::GetIBV() const {
	return ibv_;
}


} /// ONEngine
