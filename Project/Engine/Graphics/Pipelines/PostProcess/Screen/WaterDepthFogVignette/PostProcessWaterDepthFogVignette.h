#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"
#include "Engine/Core/Utility/Math/Vector3.h"

namespace ONEngine {

class PostProcessWaterDepthFogVignette : public IPostProcessPipeline {
public:
	struct DepthFogVignetteParams {
		Vector3 fogColor;
		float fogDensity;
		float fogWaterSurfaceY;
		float vignetteStrength;
		float padding[2]; // 16byte alignment
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
	ConstantBuffer<DepthFogVignetteParams> paramsBuffer_;
};

} /// ONEngine
