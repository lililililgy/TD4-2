#include "PostProcessLighting.h"

using namespace ONEngine;

/// std
#include <vector>

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/Utility/Math/Math.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Light/Light.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"

PostProcessLighting::PostProcessLighting() {}
PostProcessLighting::~PostProcessLighting() {}


void PostProcessLighting::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {
	pipeline_ = std::make_unique<ComputePipeline>();

	{	/// shader

		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"Packages/Shader/PostProcess/PerObject/Lighting/Lighting.cs.hlsl", L"cs_6_6", Shader::Type::cs);

		pipeline_->SetShader(&shader);

		pipeline_->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 0); // LightCount
		pipeline_->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 1); // Camera

		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // DirectionalLights
		pipeline_->AddDescriptorRange(1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // PointLights
		pipeline_->AddDescriptorRange(2, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // SpotLights

		pipeline_->AddDescriptorRange(3, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // ColorTex
		pipeline_->AddDescriptorRange(4, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // PositionTex
		pipeline_->AddDescriptorRange(5, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // NormalTex
		pipeline_->AddDescriptorRange(6, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // FlagsTex
		pipeline_->AddDescriptorRange(7, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // EnvironmentTexture

		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV); // OutputTex

		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 0);
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 1);
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 2);

		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 3);
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 4);
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 5);
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 6);
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 7);

		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 8); // UAV

		pipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_ALL, 0);

		pipeline_->CreatePipeline(dxm->GetDxDevice());
	}


	{
		/// constant buffer
		lightCountBufferData_ = std::make_unique<ConstantBuffer<LightCountBufferData>>();
		lightCountBufferData_->Create(dxm->GetDxDevice());

		cameraBufferData_ = std::make_unique<ConstantBuffer<CameraBufferData>>();
		cameraBufferData_->Create(dxm->GetDxDevice());

		/// structured buffer
		directionalLightBuffer_ = std::make_unique<StructuredBuffer<DirectionalLightBufferData>>();
		directionalLightBuffer_->Create(128, dxm->GetDxDevice(), dxm->GetDxSRVHeap());

		pointLightBuffer_ = std::make_unique<StructuredBuffer<PointLightBufferData>>();
		pointLightBuffer_->Create(128, dxm->GetDxDevice(), dxm->GetDxSRVHeap());

		spotLightBuffer_ = std::make_unique<StructuredBuffer<SpotLightBufferData>>();
		spotLightBuffer_->Create(128, dxm->GetDxDevice(), dxm->GetDxSRVHeap());
	}

}

void PostProcessLighting::Execute(const std::string& textureName, DxCommand* dxCommand, Asset::AssetCollection* assetCollection, EntityComponentSystem* pEntityComponentSystem, ECSGroup* pEcsGroup) {

	pipeline_->SetPipelineStateForCommandList(dxCommand);

	auto command = dxCommand->GetCommandList();
	auto& textures = assetCollection->GetTextures();

	{	/// set constant buffers and structured buffers

		ECSGroup* ecsGroup = pEcsGroup ? pEcsGroup : pEntityComponentSystem->GetCurrentGroup();

		std::vector<DirectionalLight*> directionalLights;
		std::vector<PointLight*> pointLights;
		std::vector<SpotLight*> spotLights;

		for (auto& entity : ecsGroup->GetEntities()) {
			if (auto light = entity->GetComponent<DirectionalLight>()) directionalLights.push_back(light);
			if (auto light = entity->GetComponent<PointLight>()) pointLights.push_back(light);
			if (auto light = entity->GetComponent<SpotLight>()) spotLights.push_back(light);
		}

		LightCountBufferData countData = {
			static_cast<uint32_t>(directionalLights.size()),
			static_cast<uint32_t>(pointLights.size()),
			static_cast<uint32_t>(spotLights.size()),
			0
		};
		lightCountBufferData_->SetMappedData(countData);
		lightCountBufferData_->BindForComputeCommandList(command, 0);

		for (size_t i = 0; i < directionalLights.size() && i < 128; ++i) {
			directionalLightBuffer_->SetMappedData(i, {
				Math::ConvertToVector4(directionalLights[i]->GetOwner()->GetPosition(), 1.0f),
				directionalLights[i]->GetColor(),
				directionalLights[i]->GetDirection(),
				directionalLights[i]->GetIntensity()
			});
		}
		directionalLightBuffer_->SRVBindForComputeCommandList(command, 2);

		for (size_t i = 0; i < pointLights.size() && i < 128; ++i) {
			pointLightBuffer_->SetMappedData(i, {
				Math::ConvertToVector4(pointLights[i]->GetOwner()->GetPosition(), 1.0f),
				pointLights[i]->GetColor(),
				pointLights[i]->GetIntensity(),
				pointLights[i]->GetRadius(),
				{0.0f, 0.0f}
			});
		}
		pointLightBuffer_->SRVBindForComputeCommandList(command, 3);

		for (size_t i = 0; i < spotLights.size() && i < 128; ++i) {
			spotLightBuffer_->SetMappedData(i, {
				Math::ConvertToVector4(spotLights[i]->GetOwner()->GetPosition(), 1.0f),
				spotLights[i]->GetColor(),
				spotLights[i]->GetDirection(),
				spotLights[i]->GetIntensity(),
				spotLights[i]->GetRadius(),
				spotLights[i]->GetInnerAngle(),
				spotLights[i]->GetOuterAngle(),
				0.0f
			});
		}
		spotLightBuffer_->SRVBindForComputeCommandList(command, 4);

		CameraComponent* camera = ecsGroup->GetMainCamera();
		if (camera) {
			if (GameEntity* entity = camera->GetOwner()) {
				cameraBufferData_->SetMappedData({ Math::ConvertToVector4(entity->GetPosition(), 1.0f) });
			}
		}

		cameraBufferData_->BindForComputeCommandList(command, 1);
	}


	enum {
		SCENE,
		WORLD_POSITION,
		NORMAL,
		FLAGS,
		SKYBOX,
		POST_PROCESS_RESULT,
	};


	{	/// set textures

		textureIndices_[0] = assetCollection->GetTextureIndex(textureName + "Scene");
		textureIndices_[1] = assetCollection->GetTextureIndex(textureName + "WorldPosition");
		textureIndices_[2] = assetCollection->GetTextureIndex(textureName + "Normal");
		textureIndices_[3] = assetCollection->GetTextureIndex(textureName + "Flags");

		textureIndices_[4] = assetCollection->GetTextureIndex("./Packages/Textures/kloofendal_48d_partly_cloudy_puresky_2k.dds");
		textureIndices_[5] = assetCollection->GetTextureIndex("postProcessResult");

		for (uint32_t index = 0; index < 5; ++index) {
			command->SetComputeRootDescriptorTable(index + 5, textures[textureIndices_[index]].GetSRVGPUHandle());
		}

		command->SetComputeRootDescriptorTable(10, textures[textureIndices_[POST_PROCESS_RESULT]].GetUAVGPUHandle());
	}


	command->Dispatch(
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.x), 16),
		Math::DivideAndRoundUp(static_cast<uint32_t>(EngineConfig::kWindowSize.y), 16),
		1
	);

	/// 大本のsceneテクスチャに結果をコピー
	CopyResource(
		textures[textureIndices_[POST_PROCESS_RESULT]].GetDxResource().Get(),
		textures[textureIndices_[SCENE]].GetDxResource().Get(),
		command
	);



}
