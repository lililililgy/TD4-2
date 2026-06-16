#pragma once

#include "../Interface/IEditorCompute.h"

namespace Editor {

/// /////////////////////////////////////////////////
/// 川に沿って地形を変形させるCSPipelineを起動させるクラス
/// /////////////////////////////////////////////////
class RiverTerrainAdjustPipeline : public IEditorCompute {

	enum ROOT_PARAM {
		CBV_PARAMS,
		UAV_TERRAIN_VERTICES,
		SRV_RIVER_VERTICES,
		SRV_RIVER_INDICES,
	};

public:
	/// =========================================
	/// public : methods
	/// =========================================

	RiverTerrainAdjustPipeline();
	~RiverTerrainAdjustPipeline();

	void Initialize(ONEngine::ShaderCompiler* shaderCompiler, ONEngine::DxManager* dxm) override;
	void Execute(ONEngine::EntityComponentSystem* ecs, ONEngine::DxCommand* dxCommand, ONEngine::Asset::AssetCollection* assetCollection) override;

private:
	/// =========================================
	/// private : objects
	/// =========================================
};

} /// Editor
