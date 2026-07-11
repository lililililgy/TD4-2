#pragma once

/// std
#include <array>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"

/// ///////////////////////////////////////////////////
/// ピクセレート（解像度ダウン）処理
/// ///////////////////////////////////////////////////
namespace ONEngine {

class PostProcessPixelate : public ScreenPostProcess {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	PostProcessPixelate();
	~PostProcessPixelate() override;

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

	struct PixelateParams {
		float pixelSizeX;
		float pixelSizeY;
		float padding[2]; // 16byte alignment
	};

	ConstantBuffer<PixelateParams> constantBuffer_;
	std::array<size_t, 2> textureIndices_;
};

} /// ONEngine
