#pragma once

/// std
#include <memory>
#include <vector>
#include <unordered_map>

/// engine
#include "../../Interface/IRenderingPipeline.h"
#include "Engine/Core/DirectX12/Resource/DxResource.h"
#include "Engine/Core/Utility/Math/Vector4.h"
#include "Engine/Core/Utility/Math/Vector2.h"
#include "Engine/Core/Utility/Math/Matrix4x4.h"
#include "Engine/Graphics/Buffer/StructuredBuffer.h"
#include "Engine/Graphics/Buffer/Data/GPUMaterial.h"

namespace ONEngine::Asset {
class AssetCollection;
}

namespace ONEngine {

class TextRenderingPipeline final : public IRenderingPipeline {
public:
	struct VertexData {
		Vector4 position;
		Vector2 uv;
	};

	enum ROOT_PARAM {
		ROOT_PARAM_VIEW_PROJECTION,
		ROOT_PARAM_MATERIAL,
		ROOT_PARAM_TEXTURES,
		ROOT_PARAM_TRANSFORM,
	};

public:
	TextRenderingPipeline(Asset::AssetCollection* assetCollection);
	~TextRenderingPipeline() override;

	void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) override;
	void Draw(ECSGroup* ecs, CameraComponent* camera, DxCommand* dxCommand) override;

private:
	Asset::AssetCollection* pAssetCollection_ = nullptr;
	DxManager* pDxManager_ = nullptr;

	const size_t kMaxRenderingTextCount_ = 1024;
	std::unordered_map<std::string, std::unique_ptr<StructuredBuffer<GPUMaterial>>> materialsBuffers_;
	std::unordered_map<std::string, std::unique_ptr<StructuredBuffer<Matrix4x4>>>   transformsBuffers_;

	StructuredBuffer<GPUMaterial>* GetOrCreateMaterialsBuffer(const std::string& groupName);
	StructuredBuffer<Matrix4x4>* GetOrCreateTransformsBuffer(const std::string& groupName);

	std::vector<VertexData>           vertices_;
	DxResource                        vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW          vbv_;

	std::vector<uint32_t>             indices_;
	DxResource                        indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW           ibv_;
};

} // namespace ONEngine
