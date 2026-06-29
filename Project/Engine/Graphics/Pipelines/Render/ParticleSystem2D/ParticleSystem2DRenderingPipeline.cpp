#include "ParticleSystem2DRenderingPipeline.h"

#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/Core/Utility/Math/Matrix4x4.h"
#include "Engine/Core/DirectX12/GPUTimeStamp/GPUTimeStamp.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include <algorithm>

namespace ONEngine {

ParticleSystem2DRenderingPipeline::ParticleSystem2DRenderingPipeline(Asset::AssetCollection* assetCollection)
    : pAssetCollection_(assetCollection) {}

ParticleSystem2DRenderingPipeline::~ParticleSystem2DRenderingPipeline() {}

void ParticleSystem2DRenderingPipeline::Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) {
    pDxManager_ = dxm;
    {
        // shader compile
        Shader shader;
        shader.Initialize(shaderCompiler);
        shader.CompileShader(L"Packages/Shader/Render/ParticleSystem2D/ParticleSystem2D.vs.hlsl", L"vs_6_0", Shader::Type::vs);
        shader.CompileShader(L"Packages/Shader/Render/ParticleSystem2D/ParticleSystem2D.ps.hlsl", L"ps_6_0", Shader::Type::ps);

        std::array<std::function<D3D12_BLEND_DESC()>, 5> blendModeFuncs{
            BlendMode::Normal,
            BlendMode::Add,
            BlendMode::Subtract,
            BlendMode::Multiply,
            BlendMode::Screen,
        };

        // Create pipelines for each blend mode
        for (size_t i = 0; i < blendModeFuncs.size(); i++) {
            auto& pipeline = pipelines_[i];

            pipeline = std::make_unique<GraphicsPipeline>();
            pipeline->SetShader(&shader);

            pipeline->AddInputElement("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
            pipeline->AddInputElement("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT);
            pipeline->AddInputElement("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT);

            pipeline->SetFillMode(D3D12_FILL_MODE_SOLID);
            pipeline->SetCullMode(D3D12_CULL_MODE_NONE);
            pipeline->SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

            pipeline->AddCBV(D3D12_SHADER_VISIBILITY_VERTEX, 0); // view projection (b0)
            pipeline->AddCBV(D3D12_SHADER_VISIBILITY_VERTEX, 1); // global camera data (b1)
            pipeline->Add32BitConstant(D3D12_SHADER_VISIBILITY_VERTEX, 2, sizeof(PerSystemData) / 4); // per system data (b2)

            pipeline->AddDescriptorRange(0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // particles (t0)
            pipeline->AddDescriptorRange(1, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // materials (t1)
            pipeline->AddDescriptorRange(2, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // textureId (t2)
            pipeline->AddDescriptorRange(3, Asset::MAX_TEXTURE_COUNT, D3D12_DESCRIPTOR_RANGE_TYPE_SRV); // textures (t3)

            pipeline->AddDescriptorTable(D3D12_SHADER_VISIBILITY_VERTEX, 0); // particles
            pipeline->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 1);  // materials
            pipeline->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 2);  // textureId
            pipeline->AddDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, 3);  // textures

            pipeline->AddStaticSampler(D3D12_SHADER_VISIBILITY_PIXEL, 0);

            pipeline->SetBlendDesc(blendModeFuncs[i]());
            pipeline->SetDepthStencilDesc(DepthNone()); // Disable depth read/write for 2D UI/render consistency
            pipeline->CreatePipeline(dxm->GetDxDevice());
        }
    }

    {   // buffer create
        cameraDataBuffer_.Create(dxm->GetDxDevice());

        particleBuffer_.Create(static_cast<uint32_t>(kMaxParticlesTotal_), dxm->GetDxDevice(), dxm->GetDxSRVHeap());
        materialBuffer_.Create(static_cast<uint32_t>(kMaxParticlesTotal_), dxm->GetDxDevice(), dxm->GetDxSRVHeap());
        textureIdBuffer_.Create(static_cast<uint32_t>(kMaxParticlesTotal_), dxm->GetDxDevice(), dxm->GetDxSRVHeap());
    }
}

void ParticleSystem2DRenderingPipeline::Draw(ECSGroup* ecs, CameraComponent* camera, DxCommand* dxCommand) {
    ComponentArray<ParticleSystem2D>* psArray = ecs->GetComponentArray<ParticleSystem2D>();
    if (!psArray || psArray->GetUsedComponents().empty()) {
        return;
    }

    GPUTimeStamp::GetInstance().BeginTimeStamp(GPUTimeStampID::ParticleRendering);

    auto cmdList = dxCommand->GetCommandList();

    // Fill dummy camera data (keeps constant buffer binding consistent)
    CameraData camData{};
    camData.billboardMatrix = Matrix4x4::kIdentity;
    camData.cameraPosition = camera->GetOwner()->GetTransform()->GetMatWorld().ExtractTranslation();
    camData.padding = 0.0f;
    cameraDataBuffer_.SetMappedData(camData);

    size_t globalParticleIndex = 0;

    size_t currentBlendMode = 0;
    pipelines_[currentBlendMode]->SetPipelineStateForCommandList(dxCommand);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // Bind global buffers
    camera->GetViewProjectionBuffer().BindForGraphicsCommandList(cmdList, CBV_VIEW_PROJECTION);
    cameraDataBuffer_.BindForGraphicsCommandList(cmdList, CBV_CAMERA_DATA);

    auto& textures = pAssetCollection_->GetTextures();
    if (textures.empty()) return;

    cmdList->SetGraphicsRootDescriptorTable(SRV_TEXTURES, pDxManager_->GetDxSRVHeap()->GetSRVStartGPUHandle());

    // Sort particle systems by their emitter's Z coordinate (from far to near)
    struct SortingData {
        ParticleSystem2D* ps;
        float z;
    };
    std::vector<SortingData> sortedSystems;
    sortedSystems.reserve(psArray->GetUsedComponents().size());

    for (auto& ps : psArray->GetUsedComponents()) {
        if (!ps || !ps->enable || ps->aliveCount == 0) continue;
        if (GameEntity* owner = ps->GetOwner()) {
            sortedSystems.push_back({ ps, owner->GetPosition().z });
        }
    }

    std::sort(sortedSystems.begin(), sortedSystems.end(), [](const SortingData& a, const SortingData& b) {
        return a.z > b.z;
    });

    for (auto& sortedData : sortedSystems) {
        ParticleSystem2D* ps = sortedData.ps;

        // Per-system data
        PerSystemData perSystemData{};
        perSystemData.emitterWorldMatrix = ps->GetOwner()->GetTransform()->matWorld;
        perSystemData.renderMode = static_cast<uint32_t>(ps->renderer.renderMode);
        perSystemData.renderAlignment = static_cast<uint32_t>(ps->renderer.alignment);
        perSystemData.speedScale = ps->renderer.speedScale;
        perSystemData.lengthScale = ps->renderer.lengthScale;
        perSystemData.instanceOffset = static_cast<uint32_t>(globalParticleIndex);

        size_t blendMode = static_cast<size_t>(ps->renderer.blendMode);
        if (blendMode >= pipelines_.size()) blendMode = 0; 

        if (blendMode != currentBlendMode) {
            if (pipelines_.find(blendMode) != pipelines_.end()) {
                pipelines_[blendMode]->SetPipelineStateForCommandList(dxCommand);
                
                cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                camera->GetViewProjectionBuffer().BindForGraphicsCommandList(cmdList, CBV_VIEW_PROJECTION);
                cameraDataBuffer_.BindForGraphicsCommandList(cmdList, CBV_CAMERA_DATA);
                cmdList->SetGraphicsRootDescriptorTable(SRV_TEXTURES, pDxManager_->GetDxSRVHeap()->GetSRVStartGPUHandle());
                
                currentBlendMode = blendMode;
            }
        }

        // Apply per-system root constants
        cmdList->SetGraphicsRoot32BitConstants(ROOT_PER_SYSTEM, sizeof(PerSystemData) / 4, &perSystemData, 0);

        // Try to get texture from material guid if possible
        std::string texturePath = "./Packages/Textures/white.png"; // Default fallback
        if (!ps->renderer.materialGuid.empty()) {
            Guid guid = Guid::FromString(ps->renderer.materialGuid);
            Asset::AssetType assetType = pAssetCollection_->GetAssetTypeFromGuid(guid);

            if (assetType == Asset::AssetType::Material) {
                const Asset::Material* material = pAssetCollection_->GetAsset<Asset::Material>(guid);
                if (material && material->HasBaseTexture()) {
                    texturePath = pAssetCollection_->GetTexturePath(material->GetBaseTextureGuid());
                }
            } else if (assetType == Asset::AssetType::Texture) {
                texturePath = pAssetCollection_->GetTexturePath(guid);
            }
        }

        int32_t texIndex = pAssetCollection_->GetTextureIndex(texturePath);
        uint32_t texSrvIndex = 0xFFFFFFFF;
        if (texIndex != -1 && static_cast<size_t>(texIndex) < textures.size()) {
            texSrvIndex = textures[texIndex].GetSRVDescriptorIndex();
        }

        // Get mesh
        std::string meshPath = "./Packages/Models/primitive/frontToPlane.obj"; // Default billboard quad
        const Asset::Model* model = nullptr;
        if (!ps->renderer.meshGuid.empty()) {
            const Asset::Model* customModel = pAssetCollection_->GetAsset<Asset::Model>(Guid::FromString(ps->renderer.meshGuid));
            if (customModel) {
                model = customModel;
            } else {
                model = pAssetCollection_->GetModel(meshPath);
            }
        } else {
            model = pAssetCollection_->GetModel(meshPath);
        }
        
        if (!model) continue;

        // Map data to buffers
        size_t startInstance = globalParticleIndex;
        for (size_t i = 0; i < ps->aliveCount; i++) {
            if (globalParticleIndex >= kMaxParticlesTotal_) break;
            
            particleBuffer_.SetMappedData(static_cast<uint32_t>(globalParticleIndex), ps->particles[i]);
            materialBuffer_.SetMappedData(static_cast<uint32_t>(globalParticleIndex), Vector4::One);
            textureIdBuffer_.SetMappedData(static_cast<uint32_t>(globalParticleIndex), texSrvIndex);

            globalParticleIndex++;
        }

        size_t drawCount = globalParticleIndex - startInstance;
        if (drawCount == 0) continue;

        // Bind buffers
        particleBuffer_.SRVBindForGraphicsCommandList(cmdList, SRV_PARTICLES);
        materialBuffer_.SRVBindForGraphicsCommandList(cmdList, SRV_MATERIALS);
        textureIdBuffer_.SRVBindForGraphicsCommandList(cmdList, SRV_TEXTURE_IDS);

        // Draw
        for (auto& mesh : model->GetMeshes()) {
            cmdList->IASetVertexBuffers(0, 1, &mesh->GetVBV());
            cmdList->IASetIndexBuffer(&mesh->GetIBV());

            cmdList->DrawIndexedInstanced(
                static_cast<UINT>(mesh->GetIndices().size()),
                static_cast<UINT>(drawCount),
                0, 0, 0
            );
        }
    }

    GPUTimeStamp::GetInstance().EndTimeStamp(GPUTimeStampID::ParticleRendering);
}

}
