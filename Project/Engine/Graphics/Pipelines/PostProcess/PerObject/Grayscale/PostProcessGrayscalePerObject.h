#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"

/// ///////////////////////////////////////////////////
/// オブジェクト単位でのグレースケール処理
/// ///////////////////////////////////////////////////
namespace ONEngine {

class PostProcessGrayscalePerObject : public PerObjectPostProcess {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) override;

	void Execute(
		const std::string& textureName,
		DxCommand* dxCommand,
		Asset::AssetCollection* assetCollection,
		EntityComponentSystem* entityComponentSystem,
		ECSGroup* ecsGroup = nullptr
	) override;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::array<size_t, 3> textureIndices_;
};


} /// ONEngine
