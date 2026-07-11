#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"


namespace ONEngine {

/// ///////////////////////////////////////////////////
/// フォグをかけるためのパイプライン起動用
/// ///////////////////////////////////////////////////
class PostProcessFog : public IPostProcessPipeline{

	enum ROOT_PARAM {
		CBV_FOG_PARAMS,
		CBV_CAMERA_POS,
		SRV_SCENE_COLOR,
		SRV_SCENE_WORLD_POSITION,
		UAV_OUTPUT_COLOR,
	};

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

	std::array<size_t, 3> textureIndices_; ///< テクスチャのインデックス

};

} /// namespace ONEngine