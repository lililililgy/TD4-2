#pragma once

/// engine
#include "../../Interface/IRenderingPipeline.h"

namespace ONEngine::Asset {
class AssetCollection;
}

/// /////////////////////////////////////////////////
/// 川の描画pipeline
/// /////////////////////////////////////////////////
namespace ONEngine {

class RiverRenderingPipeline : public IRenderingPipeline {

	enum ROOT_PARAM {
		CBV_VIEW_PROJECTION,
		CBV_MATERIAL,
		SRV_TEXTURE,
	};

public:
	/// =================================================
	/// public : methods
	/// =================================================

	RiverRenderingPipeline(Asset::AssetCollection* assetCollection);
	~RiverRenderingPipeline() override;

	void Initialize(ShaderCompiler* shaderCompiler, class DxManager* dxm) override;
	void Draw(class ECSGroup* ecs, class CameraComponent* camera, DxCommand* dxCommand) override;

private:
	/// =================================================
	/// private : objects
	/// =================================================

	Asset::AssetCollection* pAssetCollection_ = nullptr;
};

} /// ONEngine
