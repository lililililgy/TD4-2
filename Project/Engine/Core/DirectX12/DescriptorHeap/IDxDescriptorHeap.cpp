#include "IDxDescriptorHeap.h"

using namespace ONEngine;

/// lib
#include "Engine/Core/Utility/Tools/Assert.h"
#include "Engine/Core/Utility/Tools/Log.h"


ComPtr<ID3D12DescriptorHeap> ONEngine::CreateHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t numDescriptors, bool isShaderVisible) {
	ComPtr<ID3D12DescriptorHeap> heap;
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = heapType;
	desc.NumDescriptors = numDescriptors;
	desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT result = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
	Assert(SUCCEEDED(result), "miss created descriptor heap");

	return heap;
}



IDxDescriptorHeap::IDxDescriptorHeap(DxDevice* dxDevice, uint32_t maxHeapSize)
	: pDxDevice_(dxDevice), kMaxHeapSize_(maxHeapSize) {}


void IDxDescriptorHeap::Free(uint32_t index) {
	/// ----- すでに解放されているIndexでなければ解放 ----- ///
	auto itr = std::find(spaceIndex_.begin(), spaceIndex_.end(), index);
	if (itr == spaceIndex_.end()) {
		spaceIndex_.push_back(index);
	}
}

uint32_t IDxDescriptorHeap::Allocate() {
	/// ----- 空きIndexがあればそれを返す ----- ///

	/// 削除された index があれば再利用する
	if (!spaceIndex_.empty()) {
		uint32_t index = spaceIndex_.front();
		spaceIndex_.pop_front();
		return index;
	}

	/// 上限を超えていないかチェック
	Assert(useIndex_ < kMaxHeapSize_, "useIndex >= kMaxHeapSize_;  over!!!");
	uint32_t result = useIndex_;
	useIndex_++;
	return result;
}

void IDxDescriptorHeap::BindToCommandList(ID3D12GraphicsCommandList* commandList) {
	ID3D12DescriptorHeap* heaps[] = { descriptorHeap_.Get() };
	const UINT numHeaps = 1;
	commandList->SetDescriptorHeaps(numHeaps, heaps);
}

D3D12_CPU_DESCRIPTOR_HANDLE IDxDescriptorHeap::GetCPUDescriptorHandel(uint32_t index) const {
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += (descriptorSize_ * index);
	return cpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE IDxDescriptorHeap::GetGPUDescriptorHandel(uint32_t index) const {
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += (descriptorSize_ * index);
	return gpuHandle;
}

ID3D12DescriptorHeap* IDxDescriptorHeap::GetHeap() const {
	return descriptorHeap_.Get();
}

uint32_t IDxDescriptorHeap::GetMaxHeapSize() const {
	return kMaxHeapSize_;
}

uint32_t IDxDescriptorHeap::GetUsedIndexCount() const {
	return useIndex_ - static_cast<uint32_t>(spaceIndex_.size());
}



