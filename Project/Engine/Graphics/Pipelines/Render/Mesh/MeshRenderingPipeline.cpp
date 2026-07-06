#include "MeshRenderingPipeline.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/MeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/CustomMeshRenderer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/Core/DirectX12/GPUTimeStamp/GPUTimeStamp.h"


MeshRenderingPipeline::MeshRenderingPipeline(Asset::AssetCollection* assetCollection)
	: pAssetCollection_(assetCollection) {
}

MeshRenderingPipeline::~MeshRenderingPipeline() {}

void MeshRenderingPipeline::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {

	{	/// pipeline create

		/// shader compile
		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"./Packages/Shader/Render/Mesh/Mesh.vs.hlsl", L"vs_6_0", Shader::Type::vs);
		shader.CompileShader(L"./Packages/Shader/Render/Mesh/Mesh.ps.hlsl", L"ps_6_0", Shader::Type::ps);

		pipeline_ = std::make_unique<GraphicsPipeline>();
		pipeline_->SetShader(&shader);

		pipeline_->AddInputElement("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
		pipeline_->AddInputElement("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT);
		pipeline_->AddInputElement("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT);

		pipeline_->SetFillMode(D3D12_FILL_MODE_SOLID);
		pipeline_->SetCullMode(D3D12_CULL_MODE_BACK);

		pipeline_->SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		pipeline_->AddCBV(D3D12_SHADER_VISIBILITY_VERTEX, 0); ///< view projection: 0

		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);  ///< material
		pipeline_->AddDescriptorRange(1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);  ///< textureId
		pipeline_->AddDescriptorRange(2, Asset::MAX_TEXTURE_COUNT, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); ///< texture
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);  ///< transform
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 0);       ///< material  : 1
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 1);       ///< textureId : 2
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 2);       ///< texture   : 3
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_VERTEX, 3);      ///< transform : 4

		pipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_PIXEL, 0);         ///< texture sampler

		pipeline_->Add32BitConstant(D3D12_SHADER_VISIBILITY_VERTEX, 1);        ///< instance id: 5


		pipeline_->SetBlendDesc(BlendMode::Normal());
		pipeline_->SetDepthStencilDesc(DefaultDepthStencilDesc());

		pipeline_->CreatePipeline(dxm->GetDxDevice());

		// --- Telegraph専用パイプラインの生成 ---
		telegraphPipeline_ = std::make_unique<GraphicsPipeline>();
		telegraphPipeline_->SetShader(&shader);
		telegraphPipeline_->AddInputElement("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
		telegraphPipeline_->AddInputElement("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT);
		telegraphPipeline_->AddInputElement("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT);
		telegraphPipeline_->SetFillMode(D3D12_FILL_MODE_SOLID);
		telegraphPipeline_->SetCullMode(D3D12_CULL_MODE_BACK);
		telegraphPipeline_->SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		telegraphPipeline_->AddCBV(D3D12_SHADER_VISIBILITY_VERTEX, 0);
		telegraphPipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
		telegraphPipeline_->AddDescriptorRange(1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
		telegraphPipeline_->AddDescriptorRange(2, Asset::MAX_TEXTURE_COUNT, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
		telegraphPipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
		telegraphPipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 0);
		telegraphPipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 1);
		telegraphPipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 2);
		telegraphPipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_VERTEX, 3);
		telegraphPipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_PIXEL, 0);
		telegraphPipeline_->Add32BitConstant(D3D12_SHADER_VISIBILITY_VERTEX, 1);
		
		telegraphPipeline_->SetBlendDesc(BlendMode::Normal());
		telegraphPipeline_->SetDepthStencilDesc(TelegraphDepthStencilDesc()); // Z-Test Always, Z-Write Off
		
		telegraphPipeline_->CreatePipeline(dxm->GetDxDevice());
	}


	pDxManager_ = dxm;

}

void MeshRenderingPipeline::Draw(class ECSGroup* ecs, CameraComponent* camera, DxCommand* dxCommand) {

	/// MeshRendererの取得＆存在チェック
	ComponentArray<MeshRenderer>* meshRendererArray = ecs->GetComponentArray<MeshRenderer>();
	if (!meshRendererArray || meshRendererArray->GetUsedComponents().empty()) {
		return;
	}

	GPUTimeStamp::GetInstance().BeginTimeStamp(GPUTimeStampID::MeshRendering);

	/// レイヤーごとに分類
	std::unordered_map<RenderQueue, std::unordered_map<std::string, std::list<MeshRenderer*>>> queueMap;
	for (const auto& meshRenderer : meshRendererArray->GetUsedComponents()) {
		if(!CheckComponentEnable(meshRenderer)) {
			continue;
		}

		RenderQueue queue = meshRenderer->GetRenderQueue();
		std::string meshPath = meshRenderer->GetMeshPath();

		/// meshが読み込まれていなければ、デフォルトのメッシュを使用
		if (!pAssetCollection_->GetModel(meshPath)) {
			meshPath = "./Assets/Models/primitive/cube.obj";
		}
		queueMap[queue][meshPath].push_back(meshRenderer);
	}


	auto cmdList = dxCommand->GetCommandList();

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	auto& textures = pAssetCollection_->GetTextures();

	transformIndex_ = 0;
	instanceIndex_ = 0;

	const std::string& groupName = ecs->GetGroupName();
	auto* transformBuffer = GetOrCreateTransformBuffer(groupName);
	auto* materialBuffer = GetOrCreateMaterialBuffer(groupName);
	auto* textureIdBuffer = GetOrCreateTextureIdBuffer(groupName);

	/// 1. Backgroundの描画
	if (queueMap.count(RenderQueue::Background)) {
		pipeline_->SetPipelineStateForCommandList(dxCommand);
		camera->GetViewProjectionBuffer().BindForGraphicsCommandList(cmdList, 0);
		cmdList->SetGraphicsRootDescriptorTable(3, (*textures.begin()).GetSRVGPUHandle());
		Drawing(cmdList, queueMap[RenderQueue::Background], textures, materialBuffer, textureIdBuffer, transformBuffer);
	}

	/// 2. Telegraphの描画 (専用のZ-Test無視パイプライン)
	if (queueMap.count(RenderQueue::Telegraph)) {
		telegraphPipeline_->SetPipelineStateForCommandList(dxCommand);
		camera->GetViewProjectionBuffer().BindForGraphicsCommandList(cmdList, 0);
		cmdList->SetGraphicsRootDescriptorTable(3, (*textures.begin()).GetSRVGPUHandle());
		Drawing(cmdList, queueMap[RenderQueue::Telegraph], textures, materialBuffer, textureIdBuffer, transformBuffer);
	}

	/// 3. Defaultの描画 (通常のパイプライン)
	if (queueMap.count(RenderQueue::Default)) {
		pipeline_->SetPipelineStateForCommandList(dxCommand);
		camera->GetViewProjectionBuffer().BindForGraphicsCommandList(cmdList, 0);
		cmdList->SetGraphicsRootDescriptorTable(3, (*textures.begin()).GetSRVGPUHandle());
		Drawing(cmdList, queueMap[RenderQueue::Default], textures, materialBuffer, textureIdBuffer, transformBuffer);
	}


	GPUTimeStamp::GetInstance().EndTimeStamp(GPUTimeStampID::MeshRendering);
}

void MeshRenderingPipeline::Drawing(
	ID3D12GraphicsCommandList* cmdList,
	std::unordered_map<std::string, std::list<MeshRenderer*>>& pathMeshMap,
	const std::vector<Asset::Texture>& textures,
	StructuredBuffer<GPUMaterial>* materialBuffer,
	StructuredBuffer<uint32_t>* textureIdBuffer,
	StructuredBuffer<Matrix4x4>* transformBuffer
) {
	for (auto& [meshPath, renderers] : pathMeshMap) {

		/// modelの取得、なければ次へ
		const Asset::Model*&& model = pAssetCollection_->GetModel(meshPath);
		if (!model) {
			continue;
		}

		/// transform, material を mapping
		for (auto& renderer : renderers) {
			/// TextureのIdをGuidからセット
			renderer->SetupRenderData(pAssetCollection_);

			uint32_t id = renderer->GetGpuMaterial().baseTextureId;
			if(id < 0 || id >= textures.size()) {
				continue;
			}

			materialBuffer->SetMappedData(
				transformIndex_,
				renderer->GetGpuMaterial()
			);

			textureIdBuffer->SetMappedData(
				transformIndex_,
				textures[renderer->GetGpuMaterial().baseTextureId].GetSRVDescriptorIndex()
			);

			/// transform のセット
			transformBuffer->SetMappedData(
				transformIndex_,
				renderer->GetOwner()->GetTransform()->GetMatWorld()
			);


			++transformIndex_;
		}

		/// 上でセットしたデータをバインド
		materialBuffer->SRVBindForGraphicsCommandList(cmdList, 1);
		textureIdBuffer->SRVBindForGraphicsCommandList(cmdList, 2);
		transformBuffer->SRVBindForGraphicsCommandList(cmdList, 4);

		/// 現在のinstance idをセット
		cmdList->SetGraphicsRoot32BitConstant(5, instanceIndex_, 0);

		/// mesh の描画
		for (auto& mesh : model->GetMeshes()) {
			/// vbv, ibvのセット
			cmdList->IASetVertexBuffers(0, 1, &mesh->GetVBV());
			cmdList->IASetIndexBuffer(&mesh->GetIBV());

			/// 描画
			cmdList->DrawIndexedInstanced(
				static_cast<UINT>(mesh->GetIndices().size()),
				static_cast<UINT>(renderers.size()),
				0, 0, 0
			);
		}

		instanceIndex_ += static_cast<UINT>(renderers.size());
	}
}

StructuredBuffer<Matrix4x4>* MeshRenderingPipeline::GetOrCreateTransformBuffer(const std::string& groupName) {
	auto it = transformBuffers_.find(groupName);
	if (it != transformBuffers_.end()) {
		return it->second.get();
	}
	auto buffer = std::make_unique<StructuredBuffer<Matrix4x4>>();
	buffer->Create(static_cast<uint32_t>(kMaxRenderingMeshCount_), pDxManager_->GetDxDevice(), pDxManager_->GetDxSRVHeap());
	transformBuffers_[groupName] = std::move(buffer);
	return transformBuffers_[groupName].get();
}

StructuredBuffer<GPUMaterial>* MeshRenderingPipeline::GetOrCreateMaterialBuffer(const std::string& groupName) {
	auto it = materialBuffers_.find(groupName);
	if (it != materialBuffers_.end()) {
		return it->second.get();
	}
	auto buffer = std::make_unique<StructuredBuffer<GPUMaterial>>();
	buffer->Create(static_cast<uint32_t>(kMaxRenderingMeshCount_), pDxManager_->GetDxDevice(), pDxManager_->GetDxSRVHeap());
	materialBuffers_[groupName] = std::move(buffer);
	return materialBuffers_[groupName].get();
}

StructuredBuffer<uint32_t>* MeshRenderingPipeline::GetOrCreateTextureIdBuffer(const std::string& groupName) {
	auto it = textureIdBuffers_.find(groupName);
	if (it != textureIdBuffers_.end()) {
		return it->second.get();
	}
	auto buffer = std::make_unique<StructuredBuffer<uint32_t>>();
	buffer->Create(static_cast<uint32_t>(kMaxRenderingMeshCount_), pDxManager_->GetDxDevice(), pDxManager_->GetDxSRVHeap());
	textureIdBuffers_[groupName] = std::move(buffer);
	return textureIdBuffers_[groupName].get();
}

