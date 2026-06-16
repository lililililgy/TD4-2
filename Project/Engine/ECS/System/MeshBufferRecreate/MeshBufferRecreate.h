#pragma once

/// engine
#include "../Interface/ECSISystem.h"

/// /////////////////////////////////////////////////
/// カスタムメッシュのバッファを再作成するシステム
/// /////////////////////////////////////////////////
namespace ONEngine {

class MeshBufferRecreate final : public ECSISystem {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================
	MeshBufferRecreate(class DxDevice* dxDevice);
	~MeshBufferRecreate() = default;

	void RuntimeUpdate(class ECSGroup* ecs) override;


private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	class DxDevice* pDxDevice_ = nullptr;
};


} /// ONEngine
