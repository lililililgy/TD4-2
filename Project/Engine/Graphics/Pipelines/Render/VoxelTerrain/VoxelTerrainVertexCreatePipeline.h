#pragma once

/// engine
#include "../../Interface/IRenderingPipeline.h"
#include "Engine/Graphics/Shader/ComputePipeline.h"

namespace ONEngine {

class AssetCollection;
class DxManager;

class VoxelTerrainVertexCreatePipeline : public IRenderingPipeline {

	enum ROOT_PARAM {
		CBV_VOXEL_TERRAIN_INFO,
		CBV_MARCHING_CUBE,
		BIT32_CHUNK_INDEX,
		SRV_CHUNKS,
		APPEND_OUT_VERTICES,
		UAV_VERTEX_COUNTER,
		SRV_VOXEL_TEXTURES,
	};

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	VoxelTerrainVertexCreatePipeline(AssetCollection* ac);
	~VoxelTerrainVertexCreatePipeline();

	/// --------------- override methods --------------- ///
	void Initialize(ShaderCompiler* shaderCompiler, class DxManager* dxm) override;
	void PreDraw(class ECSGroup* ecs, class CameraComponent* camera, DxCommand* dxCommand) override;
	void Draw(class ECSGroup*, class CameraComponent*, DxCommand*) override {}

private:

	std::unique_ptr<ComputePipeline> computePipeline_;
	AssetCollection* pAssetCollection_;

	DxManager* pDxManager_;

};

} /// namespace ONEngine