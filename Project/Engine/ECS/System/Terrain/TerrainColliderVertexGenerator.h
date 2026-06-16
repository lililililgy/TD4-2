#pragma once

/// engine
#include "../Interface/ECSISystem.h"

/// ///////////////////////////////////////////////////
/// 地形コライダーの頂点生成システム
/// ///////////////////////////////////////////////////
namespace ONEngine {

class TerrainColliderVertexGenerator : public ECSISystem {
public:
	/// ========================================
	/// public : methods
	/// ========================================

	TerrainColliderVertexGenerator(class DxManager* dxm);
	~TerrainColliderVertexGenerator() override = default;

	void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;
	void RuntimeUpdate(class ECSGroup* ecs) override;

private:
	/// ========================================
	/// private : objects
	/// ========================================

	class DxManager* pDxManager_;

};


} /// ONEngine
