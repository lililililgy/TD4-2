#include "IndexBuffer.h"

using namespace ONEngine;

IndexBuffer::IndexBuffer() {}
IndexBuffer::~IndexBuffer() {}

void IndexBuffer::Create(size_t indicesSize, DxDevice* dxDevice) {
	/// ----- IndexBufferの作成 ----- ///

	Resize(indicesSize);
	resource_.CreateResource(dxDevice, sizeof(uint32_t) * indices_.size());

	ibv_.BufferLocation = resource_.Get()->GetGPUVirtualAddress();
	ibv_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	ibv_.Format = DXGI_FORMAT_R32_UINT;
}

void IndexBuffer::Reserve(size_t value) {
	indices_.reserve(value);
}

void IndexBuffer::Resize(size_t value) {
	indices_.resize(value);
}

void IndexBuffer::BindForCommandList(ID3D12GraphicsCommandList* commandList) {
	commandList->IASetIndexBuffer(&ibv_);
}

void IndexBuffer::Map() {
	/// ----- インデックスデータをGPU用にマッピング ----- ///
	uint32_t* map = nullptr;
	resource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&map));
	std::memcpy(map, indices_.data(), sizeof(uint32_t) * indices_.size());
}

const std::vector<uint32_t>& IndexBuffer::GetIndices() const {
	return indices_;
}

void IndexBuffer::SetIndex(size_t indexIndex, uint32_t indexValue) {
	indices_[indexIndex] = indexValue;
}

void IndexBuffer::SetIndices(const std::vector<uint32_t>& indices) {
	indices_ = indices;
}
