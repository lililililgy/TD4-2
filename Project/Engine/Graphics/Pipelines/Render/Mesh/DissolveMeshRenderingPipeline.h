#pragma once

/// engine
#include "../../Interface/IRenderingPipeline.h"

/// engine
#include "../../Interface/IRenderingPipeline.h"
#include "Engine/Graphics/Buffer/StructuredBuffer.h"
#include "Engine/Graphics/Buffer/Data/GPUMaterial.h"
#include "Engine/Core/Utility/Math/Matrix4x4.h"

namespace ONEngine {
class ShaderCompiler;
class DxManager;
}

namespace ONEngine::Asset {
class AssetCollection;
}

namespace ONEngine {

/// ///////////////////////////////////////////////////
/// DissolveMeshRendererのデータを使い描画を行うパイプライン
/// ///////////////////////////////////////////////////
class DissolveMeshRenderingPipeline : public IRenderingPipeline {

	enum ROOT_PARAM {
		CBV_VIEW_PROJECTION,
		SRV_TRANSFORM,
		SRV_MATERIAL,
		SRV_DISSOLVE_PARAMS,
		SRV_TEXTURE_ID,
		SRV_TEXTURE,
		CBV_INSTANCE_OFFSET,
	};

	struct GPUDissolveParams {
		uint32_t textureId;
		uint32_t dissolveCompare;
		float threshold;
		float edgeWidth;
		Vector4 edgeColor;
	};


public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	DissolveMeshRenderingPipeline(Asset::AssetCollection* ac);
	~DissolveMeshRenderingPipeline() override = default;

	void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) override;
	void Draw(ECSGroup* ecsGroup, CameraComponent* camera, DxCommand* dxCommand) override;


private:
	/// ==================================================
	/// private : objects
	/// ==================================================

	Asset::AssetCollection* pAssetCollection_ = nullptr;

	const uint32_t kMaxRenderingMeshCount_ = 1024; ///< 最大描画メッシュ数

	StructuredBuffer<Matrix4x4> sbufTransforms_;
	StructuredBuffer<GPUMaterial> sbufMaterials_;
	StructuredBuffer<uint32_t> sbufTextureIds_;
	StructuredBuffer<GPUDissolveParams> sbufDissolveParams_;
};

} /// namespace ONEngine