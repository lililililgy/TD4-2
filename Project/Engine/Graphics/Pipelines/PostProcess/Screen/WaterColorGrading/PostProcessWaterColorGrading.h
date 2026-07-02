#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"
#include "Engine/Core/Utility/Math/Vector3.h"

namespace ONEngine {

class PostProcessWaterColorGrading : public IPostProcessPipeline {
public:
	struct ColorGradingParams {
		Vector3 absorption;
		float contrast;
		float saturation;
		Vector3 colorFilter;
	};

	enum ROOT_PARAM {
		CBV_PARAMS,
		CBV_CAMERA,
		SRV_SCENE_COLOR,
		SRV_SCENE_WORLD_POSITION,
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
	std::array<size_t, 3> textureIndices_;
	ConstantBuffer<ColorGradingParams> paramsBuffer_;
};

} /// ONEngine
