#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"

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
		EntityComponentSystem* entityComponentSystem,
		ECSGroup* ecsGroup = nullptr
	) override;


private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	struct RadialBlurParams {
		int32_t offsetX;
		int32_t offsetY;
		int32_t virtualWidth;
		int32_t virtualHeight;
	};

	ConstantBuffer<RadialBlurParams> constantBuffer_;
	std::array<size_t, 3> textureIndices_; ///< テクスチャのインデックス
};


} /// ONEngine
