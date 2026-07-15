#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"

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
		EntityComponentSystem* entityComponentSystem,
		ECSGroup* ecsGroup = nullptr
	) override;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	struct GrayscaleParams {
		int32_t offsetX;
		int32_t offsetY;
		int32_t virtualWidth;
		int32_t virtualHeight;
	};

	ConstantBuffer<GrayscaleParams> constantBuffer_;
	std::array<size_t, 2> textureIndices_;
};


} /// ONEngine
