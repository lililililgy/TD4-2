#include "DxResource.h"

using namespace ONEngine;

/// externals
#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_flags.hpp>

/// engine
#include "../Device/DxDevice.h"
#include "../Command/DxCommand.h"
#include "Engine/Core/Utility/Tools/Assert.h"


DxResource::DxResource() = default;
DxResource::~DxResource() = default;

void DxResource::CreateResource(DxDevice* dxDevice, size_t sizeInByte) {
	HRESULT result = S_FALSE;

	/// 256バイトの倍数に切り上げる (ConstantBufferのアライメント制限)
	size_t alignedSize = (sizeInByte + 255) & ~255;

	/// ヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; /// バッファリソース
	desc.Width = alignedSize;                     /// リソースのサイズ
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	/// リソースの作成
	result = dxDevice->GetDevice()->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource_)
	);

	Assert(SUCCEEDED(result), "Resource creation failed.");
}

void DxResource::CreateUAVResource(DxDevice* dxDevice, class DxCommand* dxCommand, size_t sizeInByte) {
	/// ----- UAVリソースとして作成する ----- ///

	HRESULT result = S_FALSE;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(
		sizeInByte,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	currentState_ = D3D12_RESOURCE_STATE_COMMON;
	dxDevice->GetDevice()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		currentState_,
		nullptr,
		IID_PPV_ARGS(&resource_)
	);


	CreateBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, dxCommand);

	Assert(SUCCEEDED(result), "UAV Resource creation failed.");
}

void DxResource::CreateDefaultHeap(DxDevice* dxDevice, DxCommand* dxCommand, size_t sizeInByte, D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) {
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT; // ここがポイント

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = sizeInByte;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT result = dxDevice->GetDevice()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&resource_)
	);
	Assert(SUCCEEDED(result), "Default Heap Resource creation failed.");

	CreateBarrier(
		D3D12_RESOURCE_STATE_COMMON,
		initState, dxCommand
	);
}

void ONEngine::DxResource::CreateUploadHeap(DxDevice* dxDevice, DxCommand* dxCommand, size_t sizeInByte, D3D12_RESOURCE_STATES initState) {
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = sizeInByte;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT result = dxDevice->GetDevice()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&resource_)
	);
	Assert(SUCCEEDED(result), "Default Heap Resource creation failed.");

	CreateBarrier(
		D3D12_RESOURCE_STATE_COMMON,
		initState, dxCommand
	);
}

void DxResource::CreateCommittedResource(DxDevice* dxDevice, const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS _HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES _InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue) {
	currentState_ = _InitialResourceState;

	HRESULT hr = dxDevice->GetDevice()->CreateCommittedResource(
		pHeapProperties,
		_HeapFlags,
		pDesc,
		_InitialResourceState,
		pOptimizedClearValue,
		IID_PPV_ARGS(&resource_)
	);

	if(!SUCCEEDED(hr)) {
		Console::LogError("[DxResource::CreateCommittedResource] Committed Resource creation failed.");
		Assert(false, "Committed Resource creation failed.");
	}
}

void DxResource::CreateRenderTextureResource(DxDevice* dxDevice, const Vector2& size, DXGI_FORMAT format, const Vector4& clearColor) {
	/// ----- RTVとして利用できるようリソースを作成する ----- ///


	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		static_cast<UINT64>(size.x),
		static_cast<UINT64>(size.y),
		1, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format;
	clearValue.Color[0] = clearColor.x;
	clearValue.Color[1] = clearColor.y;
	clearValue.Color[2] = clearColor.z;
	clearValue.Color[3] = clearColor.w;

	dxDevice->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue,
		IID_PPV_ARGS(&resource_)
	);
}

void DxResource::CreateUAVTextureResource(DxDevice* dxDevice, const Vector2& size, DXGI_FORMAT format) {
	/// ----- UAV用のテクスチャとして作成する ----- ///

	currentState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		static_cast<UINT64>(size.x),
		static_cast<UINT64>(size.y),
		1, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT result = dxDevice->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		currentState_,
		nullptr,
		IID_PPV_ARGS(&resource_)
	);

	Assert(SUCCEEDED(result), "UAV Texture Resource creation failed.");
}

void DxResource::CreateBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, DxCommand* dxCommand) {
	::CreateBarrier(resource_.Get(), before, after, dxCommand);
	currentState_ = after;

	/// ログ出力 (リソース名、Before、After)
	//{
	//	Console::Log("[DxResource::CreateBarrier]");
	//	Console::Log(L" - Name: " + GetD3D12Name(resource_.Get()));
	//	Console::Log(" - Before State: "
	//		+ std::to_string(static_cast<int>(before)) + " : "
	//		+ std::string(magic_enum::enum_name<D3D12_RESOURCE_STATES>(before))
	//	);

	//	Console::Log(" - After State: "
	//		+ std::to_string(static_cast<int>(after)) + " : "
	//		+ std::string(magic_enum::enum_name<D3D12_RESOURCE_STATES>(after))
	//	);
	//}
}

void DxResource::CreateBarrier(D3D12_RESOURCE_STATES after, DxCommand* dxCommand) {
	::CreateBarrier(resource_.Get(), currentState_, after, dxCommand);

	/// ログ出力 (リソース名、Before、After)
	//{
	//	Console::Log("[DxResource::CreateBarrier]");
	//	Console::Log(L" - Name: " + GetD3D12Name(resource_.Get()));
	//	Console::Log(" - Before State: "
	//		+ std::to_string(static_cast<int>(currentState_)) + " : "
	//		+ std::string(magic_enum::enum_name<D3D12_RESOURCE_STATES>(currentState_))
	//	);

	//	Console::Log(" - After State: "
	//		+ std::to_string(static_cast<int>(after)) + " : "
	//		+ std::string(magic_enum::enum_name<D3D12_RESOURCE_STATES>(after))
	//	);
	//}

	currentState_ = after;
}

ID3D12Resource* DxResource::Get() const {
	return resource_.Get();
}

ComPtr<ID3D12Resource>& DxResource::GetComPtr() {
	return resource_;
}

D3D12_RESOURCE_STATES DxResource::GetCurrentState() const {
	return currentState_;
}

void DxResource::SetCurrentState(D3D12_RESOURCE_STATES state) {
	currentState_ = state;
}


std::wstring ONEngine::GetD3D12Name(ID3D12Object* object) {
	UINT size = 0;

	/// まずサイズを調べる
	HRESULT hr = object->GetPrivateData(WKPDID_D3DDebugObjectNameW, &size, nullptr);
	if(FAILED(hr) || size == 0) {
		return L""; // 名前なし
	}

	std::wstring name(size / sizeof(wchar_t), L'\0');

	hr = object->GetPrivateData(WKPDID_D3DDebugObjectNameW, &size, name.data());
	if(FAILED(hr)) {
		return L"";
	}

	/// 末尾の null を削る
	if(!name.empty() && name.back() == L'\0') {
		name.pop_back();
	}

	return name;
}

void ONEngine::CreateBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, DxCommand* dxCommand) {
	/// ----- リソースバリアーの作成 ----- ///

	if(before == after) {
		return;
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;

	dxCommand->GetCommandList()->ResourceBarrier(1, &barrier);
}

void ONEngine::CreateBarriers(std::vector<DxResource*>& resources, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, DxCommand* dxCommand) {
	/// ----- 複数リソースのバリアー作成 ----- ///

	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	barriers.reserve(resources.size());

	for(auto& res : resources) {
		if(res->GetCurrentState() != after) {
			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = res->Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = before;
			barrier.Transition.StateAfter = after;
			barriers.push_back(barrier);
		}

	}

	if(barriers.empty()) {
		return;
	}

	dxCommand->GetCommandList()->ResourceBarrier(
		static_cast<UINT>(barriers.size()), barriers.data()
	);

	for(auto& res : resources) {
		res->SetCurrentState(after);
	}
}

void ONEngine::CreateBarriers(std::vector<DxResource*>& resources, D3D12_RESOURCE_STATES after, DxCommand* dxCommand) {

	/// ----- 複数リソースのバリアー作成 ----- ///

	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	barriers.reserve(resources.size());

	for(auto& res : resources) {
		if(res->GetCurrentState() != after) {
			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = res->Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = res->GetCurrentState();
			barrier.Transition.StateAfter = after;
			barriers.push_back(barrier);
		}

	}

	if(barriers.empty()) {
		return;
	}

	dxCommand->GetCommandList()->ResourceBarrier(
		static_cast<UINT>(barriers.size()), barriers.data()
	);

	for(auto& res : resources) {
		res->SetCurrentState(after);
	}

}
