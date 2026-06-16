#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"

/// ///////////////////////////////////////////////////
/// グレースケール処理
/// ///////////////////////////////////////////////////
namespace ONEngine {

class PostProcessGrayscale : public ScreenPostProcess {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) override;

	void Execute(
		const std::string& textureName,
		DxCommand* dxCommand,
		Asset::AssetCollection* assetCollection,
		EntityComponentSystem* entityComponentSystem
	) override;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::array<size_t, 2> textureIndices_;
};


} /// ONEngine
