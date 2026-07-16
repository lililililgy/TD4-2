#pragma once

/// std
#include <memory>
#include <array>

/// engine
#include "../../../Interface/IPostProcessPipeline.h"

/// ///////////////////////////////////////////////////
/// オブジェクト毎のガウスブラー処理
/// ///////////////////////////////////////////////////
namespace ONEngine {

class PostProcessGaussianBlurPerObject : public PerObjectPostProcess {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	PostProcessGaussianBlurPerObject() = default;
	~PostProcessGaussianBlurPerObject() override = default;

	void Initialize(ShaderCompiler* shaderCompiler, class DxManager* dxm) override;
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
	
	std::array<size_t, 4> textureIndices_; ///< テクスチャのインデックス

};


} /// ONEngine
