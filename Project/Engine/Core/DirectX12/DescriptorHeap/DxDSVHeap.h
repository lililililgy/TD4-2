#pragma once

/// engine
#include "IDxDescriptorHeap.h"

/// /////////////////////////////////////////////////
/// DSVHeap
/// /////////////////////////////////////////////////
namespace ONEngine {

class DxDSVHeap final : public IDxDescriptorHeap {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	DxDSVHeap(DxDevice* dxDevice, uint32_t maxHeapSize);
	~DxDSVHeap();

	/// @brief 初期化
	void Initialize() override;
};


} /// ONEngine
