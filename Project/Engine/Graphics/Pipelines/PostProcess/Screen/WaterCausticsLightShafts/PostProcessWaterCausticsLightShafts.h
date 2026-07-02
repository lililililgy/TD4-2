#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"
#include "Engine/Core/Utility/Math/Vector3.h"

namespace ONEngine {

class PostProcessWaterCausticsLightShafts : public IPostProcessPipeline {
public:
	struct CausticsParams {
		float scale;
		float speed;
		float intensity;
		float lightShaftsIntensity;
		Vector3 lightDir;
		float time;
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
		EntityComponentSystem* entityComponentSystem,
		ECSGroup* ecsGroup = nullptr
	) override;

private:
	std::array<size_t, 3> textureIndices_;
	ConstantBuffer<CausticsParams> paramsBuffer_;
};

} /// ONEngine
