#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"

/// ///////////////////////////////////////////////////
/// ラジアルブラー処理
/// ///////////////////////////////////////////////////
namespace ONEngine {

class PostProcessRadialBlur : public ScreenPostProcess {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	PostProcessRadialBlur();
	~PostProcessRadialBlur() override;

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

	std::array<size_t, 3> textureIndices_; ///< テクスチャのインデックス
};


} /// ONEngine
