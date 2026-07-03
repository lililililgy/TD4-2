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



/// //////////////////////////////////////////////////
/// レンダリング結果を画面に表示するShader
/// //////////////////////////////////////////////////
namespace ONEngine {

class CopyImageRenderingPipeline : public IRenderingPipeline {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	CopyImageRenderingPipeline(Asset::AssetCollection* assetCollection);
	~CopyImageRenderingPipeline() = default;

	void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) override;
	void Draw(ECSGroup* ecs, CameraComponent* camera, DxCommand* dxCommand) override;
	void DrawTexture(const std::string& textureName, DxCommand* dxCommand);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	Asset::AssetCollection* pAssetCollection_ = nullptr;
};


} /// ONEngine
