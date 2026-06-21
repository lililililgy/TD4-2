#pragma once

/// std
#include <unordered_map>
#include <vector>
#include <memory>

/// engine
#include "../../Interface/IRenderingPipeline.h"
#include "Engine/Core/DirectX12/Resource/DxResource.h"
#include "GizmoPrimitiveVertices.h"

namespace ONEngine {

class Gizmo3DRenderingPipeline : public IRenderingPipeline {
private:
	enum {
		Solid,
		Wire,
	};

public:
	Gizmo3DRenderingPipeline();
	~Gizmo3DRenderingPipeline() override = default;

	void Initialize(ShaderCompiler* shaderCompiler, class DxManager* dxm) override;
	void Draw(class ECSGroup* ecs, class CameraComponent* camera, DxCommand* dxCommand) override;

private:
	std::unordered_map<size_t, std::unique_ptr<GraphicsPipeline>> pipelines_;
	size_t maxVertexNum_; ///< 最大ライン数

	DxResource vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbv_;
	GizmoPrimitive::VertexData* mappingData_;
	std::vector<GizmoPrimitive::VertexData> vertices_;
};

} /// ONEngine
