#pragma once

/// engine
#include "../../Interface/IRenderingPipeline.h"


namespace ONEngine {
class DxManager;
class ECSGroup;
class CameraComponent;
}

namespace ONEngine::Asset {
class AssetCollection;
}



namespace ONEngine {

/// ///////////////////////////////////////////////////
/// VertexShaderによるVoxelTerrain描画パイプライン
/// ///////////////////////////////////////////////////
class VoxelTerrainVertexShaderRenderingPipeline : public IRenderingPipeline {

	enum ROOT_PARAM {
		CBV_VIEW_PROJECTION,
		CBV_MATERIAL
	};

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	VoxelTerrainVertexShaderRenderingPipeline(Asset::AssetCollection* ac);
	~VoxelTerrainVertexShaderRenderingPipeline();

	/// --------------- override methods --------------- ///
	void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) override;
	void Draw(ECSGroup* ecs, CameraComponent* camera, DxCommand* dxCommand) override;


private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	/// --------------- other class pointers --------------- ///
	Asset::AssetCollection* pAssetCollection_;
	DxManager* pDxManager_;

};

} /// namespace ONEngine