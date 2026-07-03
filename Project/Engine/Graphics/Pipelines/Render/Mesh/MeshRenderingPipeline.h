#pragma once

/// std
#include <list>
#include <vector>
#include <memory>
#include <unordered_map>

/// engine
#include "../../Interface/IRenderingPipeline.h"
#include "Engine/Asset/Assets/Texture/Texture.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"
#include "Engine/Graphics/Buffer/StructuredBuffer.h"
#include "Engine/Core/DirectX12/Resource/DxResource.h"
#include "Engine/Graphics/Buffer/Data/GPUMaterial.h"
#include "Engine/Core/Utility/Math/Matrix4x4.h"
#include "Engine/Core/Utility/Math/Vector4.h"


namespace ONEngine {
class ShaderCompiler;
class DxManager;
class ECSGroup;
class CameraComponent;
class MeshRenderer;
}

namespace ONEngine::Asset {
class AssetCollection;
}


/// /////////////////////////////////////////////////
/// mesh描画クラス
/// /////////////////////////////////////////////////
namespace ONEngine {

class MeshRenderingPipeline final : public IRenderingPipeline {
public:

	/// ===================================================
	/// public : sub class
	/// ===================================================

	/// @brief 描画に必要なデータ
	struct RenderingData final {
		size_t renderMeshId; /// TODO: stringに変更
		MeshRenderer* meshRenderer;
	};

public:

	/// ===================================================
	/// public : methods
	/// ===================================================

	MeshRenderingPipeline(Asset::AssetCollection* assetCollection);
	~MeshRenderingPipeline();

	/// @brief 初期化関数
	/// @param shaderCompiler 
	/// @param dxDevice 
	void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) override;

	/// @brief 描画処理
	/// @param dxCommand DxCommandへのポインタ
	/// @param entityCollection EntityCollectionへのポインタ
	void Draw(ECSGroup* ecs, CameraComponent* camera, DxCommand* dxCommand) override;

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	void Drawing(
		ID3D12GraphicsCommandList* cmdList,
		std::unordered_map<std::string, std::list<MeshRenderer*>>& pathMeshMap,
		const std::vector<Asset::Texture>& textures,
		StructuredBuffer<GPUMaterial>* materialBuffer,
		StructuredBuffer<uint32_t>* textureIdBuffer,
		StructuredBuffer<Matrix4x4>* transformBuffer
	);

private:

	/// ----- other class ----- ///
	Asset::AssetCollection* pAssetCollection_;
	DxManager* pDxManager_ = nullptr;

	const size_t kMaxRenderingMeshCount_ = 1024; ///< 最大描画メッシュ数

	std::unique_ptr<GraphicsPipeline> pipeline_;
	std::unique_ptr<GraphicsPipeline> telegraphPipeline_;

	std::unordered_map<std::string, std::unique_ptr<StructuredBuffer<Matrix4x4>>> transformBuffers_;
	std::unordered_map<std::string, std::unique_ptr<StructuredBuffer<GPUMaterial>>> materialBuffers_;
	std::unordered_map<std::string, std::unique_ptr<StructuredBuffer<uint32_t>>> textureIdBuffers_;

	StructuredBuffer<Matrix4x4>* GetOrCreateTransformBuffer(const std::string& groupName);
	StructuredBuffer<GPUMaterial>* GetOrCreateMaterialBuffer(const std::string& groupName);
	StructuredBuffer<uint32_t>* GetOrCreateTextureIdBuffer(const std::string& groupName);

	size_t transformIndex_;
	uint32_t instanceIndex_;

};


} /// ONEngine
