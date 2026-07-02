#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"

namespace ONEngine {

class PostProcessWaterDistortion : public ScreenPostProcess {
public:
	struct DistortionParams {
		float strength;
		float speed;
		float frequency;
		float time;
	};

	enum ROOT_PARAM {
		CBV_PARAMS,
		SRV_SCENE_COLOR,
		UAV_OUTPUT_COLOR,
	};

public:
	void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) override;

	void Execute(
		const std::string& textureName,
		DxCommand* dxCommand,
		Asset::AssetCollection* assetCollection,
		EntityComponentSystem* entityComponentSystem
	) override;

private:
	std::array<size_t, 2> textureIndices_;
	ConstantBuffer<DistortionParams> paramsBuffer_;
};

} /// ONEngine
