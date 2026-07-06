#include "Gizmo2DRenderingPipeline.h"

using namespace ONEngine;

/// std
#include <numbers>

/// engine
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Core/Utility/Tools/Gizmo.h"
#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"

using namespace GizmoPrimitive;

Gizmo2DRenderingPipeline::Gizmo2DRenderingPipeline() {}

void Gizmo2DRenderingPipeline::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {
	{	/// wire frame pipeline
		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"./Packages/Shader/Render/Line/Gizmo3D.vs.hlsl", L"vs_6_0", Shader::Type::vs);
		shader.CompileShader(L"./Packages/Shader/Render/Line/Gizmo3D.ps.hlsl", L"ps_6_0", Shader::Type::ps);

		pipelines_[Wire] = std::make_unique<GraphicsPipeline>();
		auto pipeline = pipelines_[Wire].get();

		pipeline->SetShader(&shader);

		/// input element setting
		pipeline->AddInputElement("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
		pipeline->AddInputElement("OTHER_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
		pipeline->AddInputElement("COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
		pipeline->AddInputElement("THICKNESS", 0, DXGI_FORMAT_R32_FLOAT);
		pipeline->AddInputElement("EXPANSION_DIR", 0, DXGI_FORMAT_R32G32_FLOAT);

		pipeline->SetFillMode(D3D12_FILL_MODE_SOLID);
		pipeline->SetCullMode(D3D12_CULL_MODE_NONE);
		pipeline->SetBlendDesc(BlendMode::None());
		pipeline->SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		pipeline->AddCBV(D3D12_SHADER_VISIBILITY_VERTEX, 0); ///< view projection

		pipeline->SetDepthStencilDesc(DepthNone()); // 2Dなので深度テストを無効化！

		/// create pipeline
		pipeline->CreatePipeline(dxm->GetDxDevice());
	}

	{
		/// verticesを最大数分メモリを確保
		maxVertexNum_ = static_cast<size_t>(std::pow(2, 18));
		vertices_.reserve(maxVertexNum_);

		/// vertex bufferの作成
		vertexBuffer_.CreateResource(dxm->GetDxDevice(), sizeof(VertexData) * maxVertexNum_);
		vertexBuffer_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));

		vbv_.BufferLocation = vertexBuffer_.Get()->GetGPUVirtualAddress();
		vbv_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * maxVertexNum_);
		vbv_.StrideInBytes = static_cast<UINT>(sizeof(VertexData));
	}
}

void Gizmo2DRenderingPipeline::Draw(class ECSGroup* /*ecsGroup*/, [[maybe_unused]] CameraComponent* camera, [[maybe_unused]] DxCommand* dxCommand) {
#ifdef DEBUG_MODE
	if (!camera || !camera->enable || !camera->IsMakeViewProjection()) {
		return;
	}

	const std::vector<Gizmo::SphereData>& wireSphere2D = Gizmo::GetWireSphere2DData();
	const std::vector<Gizmo::CubeData>& wireCube2D = Gizmo::GetWireCube2DData();
	const std::vector<Gizmo::LineData>& line2D = Gizmo::GetLine2DData();

	///!< 描画対象がなければ 早期リターン
	if (wireSphere2D.empty() && wireCube2D.empty() && line2D.empty()) {
		return;
	}

	std::vector<VertexData> passVertices;

	/// sphereのデータを頂点データに積む
	for (auto& data : wireSphere2D) {
		auto vertices = GetSphereVertices(data.position, data.radius, data.color, 4.0f, 12, true);
		passVertices.insert(passVertices.end(), vertices.begin(), vertices.end());
	}

	/// cubeのデータを頂点データに積む
	for (auto& data : wireCube2D) {
		auto vertices = GetCubeVertices(data.position, data.size, data.rotate, data.color, 4.0f, true);
		passVertices.insert(passVertices.end(), vertices.begin(), vertices.end());
	}

	/// lineのデータを頂点データに積む
	for (auto& data : line2D) {
		Vector4 p0 = Math::ConvertToVector4(data.startPosition, 1.0f);
		Vector4 p1 = Math::ConvertToVector4(data.endPosition, 1.0f);

		VertexData v[4];
		for (int i = 0; i < 4; ++i) {
			v[i].position = p0;      // 常に始点
			v[i].otherPosition = p1; // 終点
			v[i].color = data.color;
			v[i].thickness = data.thickness;
		}

		v[0].expansionDir = Vector2(-1.0f, 0.0f); // 始点・左
		v[1].expansionDir = Vector2(1.0f, 0.0f);  // 始点・右
		v[2].expansionDir = Vector2(-1.0f, 1.0f); // 終点・左
		v[3].expansionDir = Vector2(1.0f, 1.0f);  // 終点・右

		// Tri 1: (0, 2, 1)
		passVertices.push_back(v[0]); passVertices.push_back(v[2]); passVertices.push_back(v[1]);
		// Tri 2: (1, 2, 3)
		passVertices.push_back(v[1]); passVertices.push_back(v[2]); passVertices.push_back(v[3]);
	}

	if (passVertices.empty()) {
		return;
	}

	/// 超過した分を削除
	if (passVertices.size() > maxVertexNum_) {
		passVertices.resize(maxVertexNum_);
	}

	std::memcpy(mappingData_, passVertices.data(), sizeof(VertexData) * passVertices.size());

	/// 描画命令を行う
	auto commandList = dxCommand->GetCommandList();
	auto wirePipeline = pipelines_[Wire].get();
	wirePipeline->SetPipelineStateForCommandList(dxCommand);

	D3D12_VERTEX_BUFFER_VIEW vbv = vbv_;
	vbv.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * passVertices.size());

	commandList->IASetVertexBuffers(0, 1, &vbv);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	camera->GetViewProjectionBuffer().BindForGraphicsCommandList(commandList, 0);

	/// draw call
	commandList->DrawInstanced(static_cast<UINT>(passVertices.size()), 1, 0, 0);

#endif _DEBUG
}
