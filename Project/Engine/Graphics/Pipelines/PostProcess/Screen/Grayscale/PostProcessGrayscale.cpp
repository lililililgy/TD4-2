#include "PostProcessGrayscale.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/Component/Array/ComponentArray.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/RendererComponents/ScreenPostEffectTag/ScreenPostEffectTag.h"

void PostProcessGrayscale::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {

	{
		Shader shader;
		shader.Initialize(shaderCompiler);
		shader.CompileShader(L"Packages/Shader/PostProcess/Screen/Grayscale/Grayscale.cs.hlsl", L"cs_6_6", Shader::Type::cs);

		constantBuffer_.Create(dxm->GetDxDevice());

		pipeline_ = std::make_unique<ComputePipeline>();
		pipeline_->SetShader(&shader);

		pipeline_->AddCBV(D3D12_SHADER_VISIBILITY_ALL, 0); // b0: GrayscaleParams
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // t0
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV); // u0
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 0);
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, 1);
		pipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_ALL, 0);
		pipeline_->CreatePipeline(dxm->GetDxDevice());

	}

}

void PostProcessGrayscale::Execute(const std::string& textureName, DxCommand* dxCommand, Asset::AssetCollection* assetCollection, [[maybe_unused]] EntityComponentSystem* entityComponentSystem, ECSGroup* ecsGroup) {

	/// 配列の取得とタグの確認
	ECSGroup* activeGroup = ecsGroup ? ecsGroup : entityComponentSystem->GetCurrentGroup();
	ComponentArray<ScreenPostEffectTag>* screenPostEffectTagArray = activeGroup->GetComponentArray<ScreenPostEffectTag>();
	if (!screenPostEffectTagArray || screenPostEffectTagArray->GetUsedComponents().empty()) {
		return;
	}

	ScreenPostEffectTag* tag = nullptr;
	for (auto& comp : screenPostEffectTagArray->GetUsedComponents()) {
		if(!comp || !comp->enable) {
			continue;
		}

		tag = comp;
		break;
	}

	if (!tag || !tag->GetPostEffectEnable(PostEffectType_Grayscale)) {
		return; // グレースケールエフェクトが無効な場合は何もしない
	}

	Vector2 offset = ScreenPostEffectTag::GetDispatchStartOffset(ecsGroup, entityComponentSystem);
	Vector2 dispatchSize = ScreenPostEffectTag::GetDispatchSize(ecsGroup, entityComponentSystem);
	constantBuffer_.SetMappedData(GrayscaleParams{
		.offsetX = static_cast<int32_t>(offset.x),
		.offsetY = static_cast<int32_t>(offset.y),
		.virtualWidth = static_cast<int32_t>(dispatchSize.x),
		.virtualHeight = static_cast<int32_t>(dispatchSize.y)
	});

	pipeline_->SetPipelineStateForCommandList(dxCommand);

	auto command = dxCommand->GetCommandList();
	auto& textures = assetCollection->GetTextures();
	textureIndices_[0] = assetCollection->GetTextureIndex(textureName + "Scene");
	textureIndices_[1] = assetCollection->GetTextureIndex("postProcessResult");

	constantBuffer_.BindForComputeCommandList(command, 0);
	command->SetComputeRootDescriptorTable(1, textures[textureIndices_[0]].GetSRVGPUHandle());
	command->SetComputeRootDescriptorTable(2, textures[textureIndices_[1]].GetUAVGPUHandle());

	Vector2 dispatchSize = ScreenPostEffectTag::GetDispatchSize(ecsGroup, entityComponentSystem);
	command->Dispatch(
		Math::DivideAndRoundUp(static_cast<uint32_t>(dispatchSize.x), 16),
		Math::DivideAndRoundUp(static_cast<uint32_t>(dispatchSize.y), 16),
		1
	);


	/// 大本のsceneテクスチャに結果をコピー
	CopyResource(
		textures[textureIndices_[1]].GetDxResource().Get(),
		textures[textureIndices_[0]].GetDxResource().Get(),
		command
	);
}
