#pragma once

/// engine
#include "../../Interface/IRenderingPipeline.h"

#include "Engine/Core/Utility/Utility.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"
#include "Engine/Graphics/Buffer/StructuredBuffer.h"
#include "Engine/Graphics/Buffer/VertexBuffer.h"
#include "Engine/Graphics/Buffer/IndexBuffer.h"
#include "Engine/Graphics/Buffer/Data/GPUMaterial.h"


namespace ONEngine::Asset {
class AssetCollection;
}


/// /////////////////////////////////////////////////
/// Terrainの描画pipeline
/// /////////////////////////////////////////////////
namespace ONEngine {

class TerrainRenderingPipeline : public IRenderingPipeline {
public:
	/// =====================================
	/// public : sub class
	/// =====================================

	enum ROOT_PARAM {
		ROOT_PARAM_VIEW_PROJECTION,
		ROOT_PARAM_TRANSFORM,
		ROOT_PARAM_TEX_GRASS,
		ROOT_PARAM_TEX_DIRT,
		ROOT_PARAM_TEX_ROCK,
		ROOT_PARAM_TEX_SNOW,
		ROOT_PARAM_MATERIAL,
	};



public:
	/// ====================================
	/// public : methods
	/// ====================================

	TerrainRenderingPipeline(Asset::AssetCollection* assetCollection);
	~TerrainRenderingPipeline();

	void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) override;
	void Draw(ECSGroup* ecs, CameraComponent* camera, DxCommand* dxCommand) override;

private:
	/// ====================================
	/// private : objects
	/// ====================================

	/// ----- other class ----- ///
	Asset::AssetCollection* pAssetCollection_;
	class Terrain* pTerrain_;

	ConstantBuffer<Matrix4x4> transformBuffer_;
	StructuredBuffer<GPUMaterial> materialBuffer_;

};


} /// ONEngine
