#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"

namespace ONEngine {

class PostProcessBloom : public PerObjectPostProcess {
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
	std::unique_ptr<ComputePipeline> extractPipeline_;
	std::unique_ptr<ComputePipeline> blurPipeline_;
	std::unique_ptr<ComputePipeline> compositePipeline_;

	std::array<size_t, 5> textureIndices_;
};

} /// namespace ONEngine
