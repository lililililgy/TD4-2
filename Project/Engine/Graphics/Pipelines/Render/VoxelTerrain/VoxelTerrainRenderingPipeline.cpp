#include "VoxelTerrainRenderingPipeline.h"

using namespace ONEngine;

/// engine
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Core/DirectX12/GPUTimeStamp/GPUTimeStamp.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/VoxelTerrain/VoxelTerrain.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"

namespace {
ConstantBuffer<Vector4> cBufPos;
}


VoxelTerrainRenderingPipeline::VoxelTerrainRenderingPipeline(Asset::AssetCollection* assetCollection)
	: pAssetCollection_(assetCollection) {
}

VoxelTerrainRenderingPipeline::~VoxelTerrainRenderingPipeline() {}


void VoxelTerrainRenderingPipeline::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {

	pDxManager_ = dxm;

	{	/// Shader
		Shader shader;
		shader.Initialize(shaderCompiler);

		shader.CompileShader(L"./Packages/Shader/Render/VoxelTerrain/VoxelTerrain.as.hlsl", L"as_6_5", Shader::Type::as);
		shader.CompileShader(L"./Packages/Shader/Render/VoxelTerrain/VoxelTerrainMarchingCube.ms.hlsl", L"ms_6_5", Shader::Type::ms);
		//shader.CompileShader(L"./Packages/Shader/Render/VoxelTerrain/VoxelTerrain.ms.hlsl", L"ms_6_5", Shader::Type::ms);
		shader.CompileShader(L"./Packages/Shader/Render/VoxelTerrain/VoxelTerrain.ps.hlsl", L"ps_6_0", Shader::Type::ps);


		/// Pipeline
		pipeline_ = std::make_unique<GraphicsPipeline>();
		CreatePipeline(pipeline_.get(), shader, dxm, D3D12_FILL_MODE_SOLID, BlendMode::Normal());
		wireframeSubtractBlendPipeline_ = std::make_unique<GraphicsPipeline>();
		CreatePipeline(wireframeSubtractBlendPipeline_.get(), shader, dxm, D3D12_FILL_MODE_WIREFRAME, BlendMode::Normal());
		wireframePipeline_ = std::make_unique<GraphicsPipeline>();
		CreatePipeline(wireframePipeline_.get(), shader, dxm, D3D12_FILL_MODE_WIREFRAME, BlendMode::Normal());
	}

	//{
	//	Shader shader;
	//	shader.Initialize(shaderCompiler);

	//	shader.CompileShader(L"./Packages/Shader/Render/VoxelTerrain/VoxelTerrainCubic.as.hlsl", L"as_6_5", Shader::Type::as);
	//	shader.CompileShader(L"./Packages/Shader/Render/VoxelTerrain/VoxelTerrainCubic.ms.hlsl", L"ms_6_5", Shader::Type::ms);
	//	shader.CompileShader(L"./Packages/Shader/Render/VoxelTerrain/VoxelTerrain.ps.hlsl", L"ps_6_0", Shader::Type::ps);
	//	cubicPipeline_ = std::make_unique<GraphicsPipeline>();

	//	CreatePipeline(cubicPipeline_.get(), shader, dxm, D3D12_FILL_MODE_SOLID, BlendMode::Normal());
	//}

	cBufPos.Create(dxm->GetDxDevice());
	cBufPos.SetMappedData(Vector4(180.0f, 465.0f, 182.0f, 1.0f));
}

void VoxelTerrainRenderingPipeline::Draw(ECSGroup* ecs, CameraComponent* camera, DxCommand* dxCommand) {

	/// ---------------------------------------------------
	/// 早期リターンの条件チェック
	/// ---------------------------------------------------
	ComponentArray<VoxelTerrain>* voxelTerrainArray = ecs->GetComponentArray<VoxelTerrain>();
	if(!voxelTerrainArray || voxelTerrainArray->GetUsedComponents().empty()) {
		return;
	}

	VoxelTerrain* voxelTerrain = nullptr;
	for(auto& vt : voxelTerrainArray->GetUsedComponents()) {
		if(CheckComponentEnable(vt)) {
			voxelTerrain = vt;
			break;
		}
	}

	if(!CheckComponentEnable(voxelTerrain)) {
		return;
	}

	auto cmdList = dxCommand->GetCommandList();
	if(!voxelTerrain->CheckCreatedBuffers()) {
		voxelTerrain->SettingChunksGuid(pAssetCollection_);
		voxelTerrain->CreateBuffers(pDxManager_->GetDxDevice(), pDxManager_->GetDxSRVHeap(), pAssetCollection_);
		return;
	}



	if(voxelTerrain->isRenderingCubic_) {
		DrawCubic(voxelTerrain, camera, dxCommand);
	}


	/// ---------------------------------------------------
	/// 描画
	/// ---------------------------------------------------


	if(voxelTerrain->canMeshShaderRendering_) {
		GPUTimeStamp::GetInstance().BeginTimeStamp(
			GPUTimeStampID::VoxelTerrainRegularCell
		);

		/// --------------- パイプラインの設定 --------------- ///
		pipeline_->SetPipelineStateForCommandList(dxCommand);
		pDxManager_->HeapBindToCommandList();

		/// --------------- バッファの設定 --------------- ///
		voxelTerrain->SetupGraphicBuffers(cmdList, { CBV_VOXEL_TERRAIN_INFO, CBV_MATERIAL, SRV_CHUNK_ARRAY, CBV_LOD_INFO, CBV_CLIFF_MATERIAL, CBV_USED_TEXTURE_IDS }, pAssetCollection_);
		voxelTerrain->cBufferCliffMaterial_.BindForGraphicsCommandList(cmdList, CBV_CLIFF_MATERIAL);


		camera->GetViewProjectionBuffer().BindForGraphicsCommandList(dxCommand->GetCommandList(), CBV_VIEW_PROJECTION);
		//camera->GetCameraPosBuffer().BindForGraphicsCommandList(dxCommand->GetCommandList(), CBV_CAMERA_POSITION);
		cBufPos.BindForGraphicsCommandList(dxCommand->GetCommandList(), CBV_CAMERA_POSITION);

		D3D12_GPU_DESCRIPTOR_HANDLE frontSRVHandle = pDxManager_->GetDxSRVHeap()->GetSRVStartGPUHandle();
		cmdList->SetGraphicsRootDescriptorTable(SRV_VOXEL_TERRAIN_TEXTURE3D, frontSRVHandle);
		cmdList->SetGraphicsRootDescriptorTable(SRV_TEXTURES, frontSRVHandle);


		/// --------------- ディスパッチ --------------- ///
		cmdList->DispatchMesh(
			voxelTerrain->GetChunkCountXZ().x,
			1,
			voxelTerrain->GetChunkCountXZ().y
		);


		GPUTimeStamp::GetInstance().EndTimeStamp(
			GPUTimeStampID::VoxelTerrainRegularCell
		);
	}


	if(voxelTerrain->isRenderingWireframe_) {
		if(voxelTerrain->canMeshShaderRendering_) {
			wireframeSubtractBlendPipeline_->SetPipelineStateForCommandList(dxCommand);
		} else {
			wireframePipeline_->SetPipelineStateForCommandList(dxCommand);
		}

		pDxManager_->HeapBindToCommandList();

		/// --------------- バッファの設定 --------------- ///
		voxelTerrain->SetupGraphicBuffers(cmdList, { CBV_VOXEL_TERRAIN_INFO, CBV_MATERIAL, SRV_CHUNK_ARRAY, CBV_LOD_INFO, CBV_CLIFF_MATERIAL, CBV_USED_TEXTURE_IDS }, pAssetCollection_);

		camera->GetViewProjectionBuffer().BindForGraphicsCommandList(dxCommand->GetCommandList(), CBV_VIEW_PROJECTION);
		//camera->GetCameraPosBuffer().BindForGraphicsCommandList(dxCommand->GetCommandList(), CBV_CAMERA_POSITION);
		cBufPos.BindForGraphicsCommandList(dxCommand->GetCommandList(), CBV_CAMERA_POSITION);
		D3D12_GPU_DESCRIPTOR_HANDLE frontSRVHandle = pDxManager_->GetDxSRVHeap()->GetSRVStartGPUHandle();
		cmdList->SetGraphicsRootDescriptorTable(SRV_VOXEL_TERRAIN_TEXTURE3D, frontSRVHandle);
		cmdList->SetGraphicsRootDescriptorTable(SRV_TEXTURES, frontSRVHandle);

		/// --------------- ディスパッチ --------------- ///
		cmdList->DispatchMesh(
			voxelTerrain->GetChunkCountXZ().x,
			1,
			voxelTerrain->GetChunkCountXZ().y
		);
	}

}

void VoxelTerrainRenderingPipeline::DrawCubic(VoxelTerrain* vt, CameraComponent* camera, DxCommand* dxCommand) {
	auto cmdList = dxCommand->GetCommandList();

	/// --------------- パイプラインの設定 --------------- ///
	cubicPipeline_->SetPipelineStateForCommandList(dxCommand);
	pDxManager_->HeapBindToCommandList();

	/// --------------- バッファの設定 --------------- ///
	vt->SetupGraphicBuffers(cmdList, { CBV_VOXEL_TERRAIN_INFO, CBV_MATERIAL, SRV_CHUNK_ARRAY, CBV_LOD_INFO, CBV_CLIFF_MATERIAL, CBV_USED_TEXTURE_IDS }, pAssetCollection_);

	camera->GetViewProjectionBuffer().BindForGraphicsCommandList(dxCommand->GetCommandList(), CBV_VIEW_PROJECTION);
	cBufPos.BindForGraphicsCommandList(dxCommand->GetCommandList(), CBV_CAMERA_POSITION);

	D3D12_GPU_DESCRIPTOR_HANDLE frontSRVHandle = pDxManager_->GetDxSRVHeap()->GetSRVStartGPUHandle();
	cmdList->SetGraphicsRootDescriptorTable(
		SRV_VOXEL_TERRAIN_TEXTURE3D, frontSRVHandle
	);

	cmdList->DispatchMesh(
		vt->GetChunkCountXZ().x,
		1,
		vt->GetChunkCountXZ().y
	);
}

void VoxelTerrainRenderingPipeline::CreatePipeline(GraphicsPipeline* pipeline, Shader& shader, DxManager* dxm, D3D12_FILL_MODE fillMode, D3D12_BLEND_DESC blendMode) {
	pipeline->SetShader(&shader);

	pipeline->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 0); // VoxelTerrainInfo
	pipeline->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 1); // ViewProjection
	pipeline->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 2); // CameraPosition
	pipeline->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 3); // LODInfo
	pipeline->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 4); // Material
	pipeline->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 5); // CliffMaterial
	pipeline->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 6); // UsedTextureIds

	pipeline->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // Chunk array
	pipeline->AddDescriptorRange(1, Asset::MAX_TEXTURE_COUNT, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // VoxelTerrain Texture3D
	pipeline->AddDescriptorRange(2050, Asset::MAX_TEXTURE_COUNT, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // Textures

	pipeline->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 0); // Chunk array
	pipeline->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 1); // VoxelTerrain Texture3D
	pipeline->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 2); // Textures


	pipeline->AddStaticSampler(StaticSampler::ClampSampler(), D3D12_SHADER_VISIBILITY_ALL, 0);
	pipeline->AddStaticSampler(D3D12_SHADER_VISIBILITY_PIXEL, 1);


	pipeline->SetBlendDesc(blendMode);
	pipeline->SetFillMode(fillMode);
	pipeline->SetCullMode(D3D12_CULL_MODE_BACK);
	pipeline->SetDepthStencilDesc(DefaultDepthStencilDesc());

	pipeline->CreatePipeline(dxm->GetDxDevice());
}
