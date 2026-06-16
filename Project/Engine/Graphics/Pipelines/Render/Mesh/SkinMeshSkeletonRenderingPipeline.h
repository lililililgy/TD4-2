#pragma once

/// engine
#include "Engine/Core/DirectX12/Resource/DxResource.h"
#include "../../Interface/IRenderingPipeline.h"
#include "../Gizmo/GizmoPrimitiveVertices.h"

/// ///////////////////////////////////////////////////
/// SkinMeshの骨を描画するレンダリングパイプライン
/// ///////////////////////////////////////////////////
namespace ONEngine {

class SkinMeshSkeletonRenderingPipeline : public IRenderingPipeline {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	SkinMeshSkeletonRenderingPipeline();
	~SkinMeshSkeletonRenderingPipeline() override = default;

	void Initialize(ShaderCompiler* shaderCompiler, class DxManager* dxm) override;
	void Draw(class ECSGroup* ecs, class CameraComponent* camera, DxCommand* dxCommand) override;


private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	size_t maxVertexNum_; ///< 最大ライン数

	DxResource vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbv_;
	GizmoPrimitive::VertexData* mappingData_;
	std::vector<GizmoPrimitive::VertexData> vertices_;

};


} /// ONEngine
