#include "TextRenderingPipeline.h"
#include <cmath>
#include <algorithm>

using namespace ONEngine;

/// engine
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/ECS/Component/Components/RendererComponents/Text/TextRenderer.h"
#include "Engine/Core/DirectX12/GPUTimeStamp/GPUTimeStamp.h"
#include "Engine/Core/Utility/Tools/Gizmo.h"

TextRenderingPipeline::TextRenderingPipeline(Asset::AssetCollection* assetCollection)
	: pAssetCollection_(assetCollection) {
}

TextRenderingPipeline::~TextRenderingPipeline() {}

void TextRenderingPipeline::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {
	pDxManager_ = dxm;
	{	/// pipeline 
		Shader shader;
		shader.Initialize(shaderCompiler);

		shader.CompileShader(L"Packages/Shader/Render/Text/Text.vs.hlsl", L"vs_6_0", Shader::Type::vs);
		shader.CompileShader(L"Packages/Shader/Render/Text/Text.ps.hlsl", L"ps_6_0", Shader::Type::ps);

		pipeline_ = std::make_unique<GraphicsPipeline>();
		pipeline_->SetShader(&shader);

		pipeline_->AddInputElement("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
		pipeline_->AddInputElement("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT);

		pipeline_->SetFillMode(D3D12_FILL_MODE_SOLID);
		pipeline_->SetCullMode(D3D12_CULL_MODE_NONE);
		pipeline_->SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		pipeline_->SetRTVNum(static_cast<uint32_t>(RTVIndex::Count));
		pipeline_->SetRTVFormat(static_cast<DXGI_FORMAT>(RTVFormat::Color), static_cast<int>(RTVIndex::Color));
		pipeline_->SetRTVFormat(static_cast<DXGI_FORMAT>(RTVFormat::WorldPosition), static_cast<int>(RTVIndex::WorldPosition));
		pipeline_->SetRTVFormat(static_cast<DXGI_FORMAT>(RTVFormat::Normal), static_cast<int>(RTVIndex::Normal));
		pipeline_->SetRTVFormat(static_cast<DXGI_FORMAT>(RTVFormat::Flags), static_cast<int>(RTVIndex::Flags));

		pipeline_->AddCBV(D3D12_SHADER_VISIBILITY_VERTEX, 0);                  ///< ROOT_PARAM_VIEW_PROJECTION : 0

		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);  ///< ROOT_PARAM_MATERIAL
		pipeline_->AddDescriptorRange(1, Asset::MAX_TEXTURE_COUNT, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); ///< ROOT_PARAM_TEXTURES
		pipeline_->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);  ///< ROOT_PARAM_TRANSFORM
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 0);       ///< ROOT_PARAM_MATERIAL : 1
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 1);       ///< ROOT_PARAM_TEXTURES   : 2
		pipeline_->AddDescriptorTable(D3D12_SHADER_VISIBILITY_VERTEX, 2);      ///< ROOT_PARAM_TRANSFORM : 3

		pipeline_->AddStaticSampler(D3D12_SHADER_VISIBILITY_PIXEL, 0);         ///< texture sampler
		pipeline_->SetBlendDesc(BlendMode::Normal());
		pipeline_->SetDepthStencilDesc(DefaultDepthStencilDesc());

		pipeline_->CreatePipeline(dxm->GetDxDevice());
	}

	{	/// buffer
		vertices_ = {
			{ Vector4(-0.5f, 0.5f, 0.0f, 1.0f), Vector2(0.0f, 0.0f) },
			{ Vector4(0.5f, 0.5f, 0.0f, 1.0f), Vector2(1.0f, 0.0f) },
			{ Vector4(-0.5f, -0.5f, 0.0f, 1.0f), Vector2(0.0f, 1.0f) },
			{ Vector4(0.5f, -0.5f, 0.0f, 1.0f), Vector2(1.0f, 1.0f) },
		};

		indices_ = {
			0, 1, 2,
			2, 1, 3,
		};

		const size_t kVertexDataSize = sizeof(VertexData);

		vertexBuffer_.CreateResource(dxm->GetDxDevice(), kVertexDataSize * vertices_.size());
		vbv_.BufferLocation = vertexBuffer_.Get()->GetGPUVirtualAddress();
		vbv_.SizeInBytes = static_cast<UINT>(kVertexDataSize * vertices_.size());
		vbv_.StrideInBytes = static_cast<UINT>(kVertexDataSize);

		indexBuffer_.CreateResource(dxm->GetDxDevice(), sizeof(uint32_t) * indices_.size());
		ibv_.BufferLocation = indexBuffer_.Get()->GetGPUVirtualAddress();
		ibv_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
		ibv_.Format = DXGI_FORMAT_R32_UINT;

		uint32_t* indexData;
		indexBuffer_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
		memcpy(indexData, indices_.data(), sizeof(uint32_t) * indices_.size());
		indexBuffer_.Get()->Unmap(0, nullptr);

		VertexData* vertexData;
		vertexBuffer_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		memcpy(vertexData, vertices_.data(), kVertexDataSize * vertices_.size());
		vertexBuffer_.Get()->Unmap(0, nullptr);
	}
}

void TextRenderingPipeline::PreDraw(ECSGroup* ecsGroup, CameraComponent* camera, DxCommand* dxCommand) {
	ComponentArray<TextRenderer>* textRendererArray = ecsGroup->GetComponentArray<TextRenderer>();
	if (!textRendererArray || textRendererArray->GetUsedComponents().empty()) {
		return;
	}

	for (auto& tr : textRendererArray->GetUsedComponents()) {
		if (!CheckComponentEnable(tr)) {
			continue;
		}
		tr->RenderingSetup(pAssetCollection_);
	}
}

void TextRenderingPipeline::Draw(ECSGroup* ecsGroup, CameraComponent* camera, DxCommand* dxCommand) {
	if (!pDxManager_) {
		pDxManager_ = DxManager::GetInstance();
	}
	if (!pDxManager_) {
		return;
	}

	ComponentArray<TextRenderer>* textRendererArray = ecsGroup->GetComponentArray<TextRenderer>();
	if (!textRendererArray || textRendererArray->GetUsedComponents().empty()) {
		return;
	}

	GPUTimeStamp::GetInstance().BeginTimeStamp(GPUTimeStampID::SpriteRendering);

	struct RenderingData {
		TextRenderer* renderer;
		float z;
		Matrix4x4 matWorld;
		GPUMaterial gpuMaterial;
	};
	std::vector<RenderingData> renderingDataList;
	renderingDataList.reserve(textRendererArray->GetUsedComponents().size() * 2); // 影があるため最大2倍

	for (auto& tr : textRendererArray->GetUsedComponents()) {
		if (!CheckComponentEnable(tr) || tr->GetText().empty()) {
			continue;
		}

		if (GameEntity* owner = tr->GetOwner()) {
			Matrix4x4 matWorld = owner->GetTransform()->GetMatWorld();

			// アスペクト比とアライメントPivotオフセットの適用
			Vector2 texSize = tr->GetTextureSize(pAssetCollection_);
			if (texSize.x > 0.0f && texSize.y > 0.0f) {
				float aspect = texSize.x / texSize.y;

				// 1. Pivotのオフセット計算
				float pivotX = 0.0f;
				if (tr->GetHorizontalAlignment() == HorizontalAlignment::Left) pivotX = 0.5f;
				else if (tr->GetHorizontalAlignment() == HorizontalAlignment::Right) pivotX = -0.5f;

				float pivotY = 0.0f;
				if (tr->GetVerticalAlignment() == VerticalAlignment::Top) pivotY = -0.5f;
				else if (tr->GetVerticalAlignment() == VerticalAlignment::Bottom) pivotY = 0.5f;

				// 2. ローカルスケールとPivot平行移動を行列の左側から合成
				Matrix4x4 matLocalScale = Matrix4x4::MakeScale(Vector3(aspect, 1.0f, 1.0f));
				Matrix4x4 matLocalPivot = Matrix4x4::MakeTranslate(Vector3(pivotX * aspect, pivotY, 0.0f));

				matWorld = (matLocalPivot * matLocalScale) * matWorld;
			}

			// ドロップシャドウが有効な場合、影用の描画データ（1パス目）を先に追加
			if (tr->GetShadowColor().w > 0.0f) {
				GPUMaterial shadowMat = tr->GetGpuMaterial();
				shadowMat.baseColor = tr->GetShadowColor();

				// 1ピクセル = ワールド座標 0.002 単位として換算
				float shadowScale = 0.002f;
				Vector2 shadowOffset = tr->GetShadowOffset() * shadowScale;

				Matrix4x4 matShadowTranslate = Matrix4x4::MakeTranslate(Vector3(shadowOffset.x, shadowOffset.y, 0.0f));
				Matrix4x4 matWorldShadow = matShadowTranslate * matWorld;

				renderingDataList.push_back({
					tr,
					owner->GetPosition().z + 0.001f, // Zファイティング防止のためわずかに奥に配置
					matWorldShadow,
					shadowMat
				});
			}

			// 本体（2パス目）の描画データを追加
			renderingDataList.push_back({
				tr,
				owner->GetPosition().z,
				matWorld,
				tr->GetGpuMaterial()
			});
		}
	}

	// Z順でソート（奥から順に手前に描画）
	std::sort(renderingDataList.begin(), renderingDataList.end(), [](const RenderingData& a, const RenderingData& b) {
		return a.z > b.z;
	});

	const std::string& groupName = ecsGroup->GetGroupName();
	auto* materialsBuffer = GetOrCreateMaterialsBuffer(groupName);
	auto* transformsBuffer = GetOrCreateTransformsBuffer(groupName);

	if (!materialsBuffer || !transformsBuffer) {
		return;
	}

	size_t transformIndex = 0;
	for (const auto& data : renderingDataList) {
		materialsBuffer->SetMappedData(transformIndex, data.gpuMaterial);
		transformsBuffer->SetMappedData(transformIndex, data.matWorld);
		++transformIndex;
	}

	// ギズモの描画 (境界ボックスとアンカー表示)
	for (auto& tr : textRendererArray->GetUsedComponents()) {
		if (!CheckComponentEnable(tr) || tr->GetText().empty()) {
			continue;
		}

		if (GameEntity* owner = tr->GetOwner()) {
			Vector2 texSize = tr->GetTextureSize(pAssetCollection_);
			if (texSize.x <= 0.0f || texSize.y <= 0.0f) continue;

			float aspect = texSize.x / texSize.y;

			float pivotX = 0.0f;
			if (tr->GetHorizontalAlignment() == HorizontalAlignment::Left) pivotX = 0.5f;
			else if (tr->GetHorizontalAlignment() == HorizontalAlignment::Right) pivotX = -0.5f;

			float pivotY = 0.0f;
			if (tr->GetVerticalAlignment() == VerticalAlignment::Top) pivotY = -0.5f;
			else if (tr->GetVerticalAlignment() == VerticalAlignment::Bottom) pivotY = 0.5f;

			Vector3 p0 = { -0.5f * aspect + pivotX * aspect, -0.5f + pivotY, 0.0f };
			Vector3 p1 = {  0.5f * aspect + pivotX * aspect, -0.5f + pivotY, 0.0f };
			Vector3 p2 = {  0.5f * aspect + pivotX * aspect,  0.5f + pivotY, 0.0f };
			Vector3 p3 = { -0.5f * aspect + pivotX * aspect,  0.5f + pivotY, 0.0f };

			Matrix4x4 matWorld = owner->GetTransform()->GetMatWorld();
			Vector3 wp0 = Matrix4x4::Transform(p0, matWorld);
			Vector3 wp1 = Matrix4x4::Transform(p1, matWorld);
			Vector3 wp2 = Matrix4x4::Transform(p2, matWorld);
			Vector3 wp3 = Matrix4x4::Transform(p3, matWorld);

			// 黄緑色のワイヤーフレーム境界ボックスを描画
			Vector4 boxColor = { 0.5f, 1.0f, 0.0f, 1.0f };
			Gizmo::DrawLine(wp0, wp1, boxColor);
			Gizmo::DrawLine(wp1, wp2, boxColor);
			Gizmo::DrawLine(wp2, wp3, boxColor);
			Gizmo::DrawLine(wp3, wp0, boxColor);

			// 赤い基準点アンカー十字を描画
			Vector3 origin = owner->GetTransform()->GetPosition();
			float markSize = 0.1f;
			Gizmo::DrawLine(origin - Vector3(markSize, 0.0f, 0.0f), origin + Vector3(markSize, 0.0f, 0.0f), { 1.0f, 0.0f, 0.0f, 1.0f });
			Gizmo::DrawLine(origin - Vector3(0.0f, markSize, 0.0f), origin + Vector3(0.0f, markSize, 0.0f), { 1.0f, 0.0f, 0.0f, 1.0f });
		}
	}

	if (transformIndex == 0) {
		return;
	}

	auto cmdList = dxCommand->GetCommandList();
	pipeline_->SetPipelineStateForCommandList(dxCommand);

	cmdList->IASetVertexBuffers(0, 1, &vbv_);
	cmdList->IASetIndexBuffer(&ibv_);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	camera->GetViewProjectionBuffer().BindForGraphicsCommandList(cmdList, ROOT_PARAM_VIEW_PROJECTION);
	cmdList->SetGraphicsRootDescriptorTable(ROOT_PARAM_TEXTURES, pDxManager_->GetDxSRVHeap()->GetSRVStartGPUHandle());

	materialsBuffer->SRVBindForGraphicsCommandList(cmdList, ROOT_PARAM_MATERIAL);
	transformsBuffer->SRVBindForGraphicsCommandList(cmdList, ROOT_PARAM_TRANSFORM);

	cmdList->DrawIndexedInstanced(
		static_cast<UINT>(indices_.size()),
		static_cast<UINT>(transformIndex),
		0, 0, 0
	);

	GPUTimeStamp::GetInstance().EndTimeStamp(GPUTimeStampID::SpriteRendering);
}

StructuredBuffer<GPUMaterial>* TextRenderingPipeline::GetOrCreateMaterialsBuffer(const std::string& groupName) {
	if (!pDxManager_ || !pDxManager_->GetDxDevice() || !pDxManager_->GetDxSRVHeap()) {
		return nullptr;
	}

	auto it = materialsBuffers_.find(groupName);
	if (it != materialsBuffers_.end()) {
		return it->second.get();
	}
	auto buffer = std::make_unique<StructuredBuffer<GPUMaterial>>();
	buffer->Create(static_cast<uint32_t>(kMaxRenderingTextCount_), pDxManager_->GetDxDevice(), pDxManager_->GetDxSRVHeap());
	materialsBuffers_[groupName] = std::move(buffer);
	return materialsBuffers_[groupName].get();
}

StructuredBuffer<Matrix4x4>* TextRenderingPipeline::GetOrCreateTransformsBuffer(const std::string& groupName) {
	if (!pDxManager_ || !pDxManager_->GetDxDevice() || !pDxManager_->GetDxSRVHeap()) {
		return nullptr;
	}

	auto it = transformsBuffers_.find(groupName);
	if (it != transformsBuffers_.end()) {
		return it->second.get();
	}
	auto buffer = std::make_unique<StructuredBuffer<Matrix4x4>>();
	buffer->Create(static_cast<uint32_t>(kMaxRenderingTextCount_), pDxManager_->GetDxDevice(), pDxManager_->GetDxSRVHeap());
	transformsBuffers_[groupName] = std::move(buffer);
	return transformsBuffers_[groupName].get();
}
