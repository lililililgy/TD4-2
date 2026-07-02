#include "RenderingPipelineCollection.h"

using namespace ONEngine;

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/ECS/Component/Components/RendererComponents/Sprite/SpriteRenderer.h"

/// pipelines
#include "../Render/Mesh/MeshRenderingPipeline.h"
#include "../Render/Mesh/DissolveMeshRenderingPipeline.h"
#include "../Render/Mesh/SkinMeshRenderingPipeline.h"
#include "../Render/Mesh/SkinMeshSkeletonRenderingPipeline.h"
#include "../Render/Effect/EffectRenderingPipeline.h"
#include "../Render/ParticleSystem/ParticleSystemRenderingPipeline.h"
#include "../Render/ParticleSystem2D/ParticleSystem2DRenderingPipeline.h"
#include "../Render/Primitive/Line2DRenderingPipeline.h"
#include "../Render/Primitive/Line3DRenderingPipeline.h"
#include "../Render/Sprite/SpriteRenderingPipeline.h"
#include "../Render/Gizmo/Gizmo3DRenderingPipeline.h"
#include "../Render/Gizmo/Gizmo2DRenderingPipeline.h"
#include "../Render/Skybox/SkyboxRenderingPipeline.h"
#include "../Render/Terrain/TerrainRenderingPipeline.h"
#include "../Render/Terrain/TerrainProceduralRenderingPipeline.h"
#include "../Render/River/RiverRenderingPipeline.h"
#include "../Render/Grass/GrassRenderingPipeline.h"
#include "../Render/Grid/GridRenderingPipeline.h"
#include "../Render/VoxelTerrain/VoxelTerrainVertexCreatePipeline.h"
#include "../Render/VoxelTerrain/VoxelTerrainVertexShaderRenderingPipeline.h"
#include "../Render/VoxelTerrain/VoxelTerrainRenderingPipeline.h"
#include "../Render/VoxelTerrain/VoxelTerrainTransvoxelRenderingPipeline.h"
#include "../Render/VoxelTerrain/VoxelTerrainBrushPreviewRenderingPipeline.h"

/// post process
#include "../PostProcess/PerObject/Light/PostProcessLighting.h"
#include "../PostProcess/PerObject/Grayscale/PostProcessGrayscalePerObject.h"
#include "../PostProcess/PerObject/Blur/PostProcessGaussianBlurPerObject.h"
#include "../PostProcess/Screen/Grayscale/PostProcessGrayscale.h"
#include "../PostProcess/Screen/RadialBlur/PostProcessRadialBlur.h"
#include "../PostProcess/Screen/Shadow/PostProcessShadowApply.h"
#include "../PostProcess/Screen/Fog/PostProcessFog.h"
#include "../PostProcess/Screen/Fisheye/PostProcessFisheye.h"
#include "../PostProcess/PerObject/TerrainBrush/PostProcessTerrainBrush.h"
#include "../PostProcess/PerObject/VoxelTerrainBrush/PostProcessVoxelTerrainBrush.h"
#include "../PostProcess/Screen/WaterDistortion/PostProcessWaterDistortion.h"
#include "../PostProcess/Screen/WaterDepthFogVignette/PostProcessWaterDepthFogVignette.h"
#include "../PostProcess/Screen/WaterColorGrading/PostProcessWaterColorGrading.h"
#include "../PostProcess/Screen/WaterCausticsLightShafts/PostProcessWaterCausticsLightShafts.h"

RenderingPipelineCollection::RenderingPipelineCollection(ShaderCompiler* shaderCompiler, DxManager* dxm, EntityComponentSystem* pEntityComponentSystem, Asset::AssetCollection* assetCollection)
	: pShaderCompiler_(shaderCompiler), pDxManager_(dxm), pEntityComponentSystem_(pEntityComponentSystem), pAssetCollection_(assetCollection) {}

RenderingPipelineCollection::~RenderingPipelineCollection() {}

void RenderingPipelineCollection::Initialize() {

	/// ----- 2D用のパイプラインを生成 ----- ///
	Generate2DRenderingPipeline<Line2DRenderingPipeline>();
	Generate2DRenderingPipeline<SpriteRenderingPipeline>(pAssetCollection_);
	Generate2DRenderingPipeline<ParticleSystem2DRenderingPipeline>(pAssetCollection_);

	/// ----- 3D用のパイプラインを生成 ----- ///
	Generate3DRenderingPipeline<Line3DRenderingPipeline>();
	Generate3DRenderingPipeline<SkyboxRenderingPipeline>(pAssetCollection_);
	//Generate3DRenderingPipeline<TerrainRenderingPipeline>(pAssetCollection_);
	//Generate3DRenderingPipeline<VoxelTerrainVertexCreatePipeline>(pAssetCollection_);
	//Generate3DRenderingPipeline<VoxelTerrainRenderingPipeline>(pAssetCollection_);
	//Generate3DRenderingPipeline<VoxelTerrainTransvoxelRenderingPipeline>(pAssetCollection_);
	//Generate3DRenderingPipeline<VoxelTerrainVertexShaderRenderingPipeline>(pAssetCollection_);
	//Generate3DRenderingPipeline<TerrainProceduralRenderingPipeline>(pAssetCollection_);
	//Generate3DRenderingPipeline<RiverRenderingPipeline>(pAssetCollection_);
	Generate3DRenderingPipeline<MeshRenderingPipeline>(pAssetCollection_);
	Generate3DRenderingPipeline<DissolveMeshRenderingPipeline>(pAssetCollection_);
	Generate3DRenderingPipeline<SkinMeshRenderingPipeline>(pAssetCollection_);
#ifdef DEBUG_MODE
	/// Debug用のパイプライン
	Generate3DRenderingPipeline<SkinMeshSkeletonRenderingPipeline>();
	//Generate3DRenderingPipeline<VoxelTerrainBrushPreviewRenderingPipeline>(pAssetCollection_);
	Generate3DRenderingPipeline<GridRenderingPipeline>();
#endif // DEBUG_MODE
	Generate3DRenderingPipeline<EffectRenderingPipeline>(pAssetCollection_);

	particleRenderer_ = std::make_unique<ParticleSystemRenderingPipeline>(pAssetCollection_);
	particleRenderer_->Initialize(pShaderCompiler_, pDxManager_);

	Generate3DRenderingPipeline<GrassRenderingPipeline>(pAssetCollection_);

	/// Gizmoは最後に描画する
	gizmo3D_ = std::make_unique<Gizmo3DRenderingPipeline>();
	gizmo3D_->Initialize(pShaderCompiler_, pDxManager_);

	gizmo2D_ = std::make_unique<Gizmo2DRenderingPipeline>();
	gizmo2D_->Initialize(pShaderCompiler_, pDxManager_);



	/// ----- オブジェクトごとのポストエフェクトのパイプラインを生成 ----- ///
	GeneratePostProcess3DPipeline<PostProcessLighting>();
	GeneratePostProcess3DPipeline<PostProcessGrayscalePerObject>();
	GeneratePostProcess3DPipeline<PostProcessTerrainBrush>();
	GeneratePostProcess3DPipeline<PostProcessVoxelTerrainBrush>();
	GeneratePostProcess3DPipeline<PostProcessGaussianBlurPerObject>();

	/// ----- スクリーンにかける用のポストエフェクトのパイプラインを生成 ----- ///
	GeneratePostProcess3DPipeline<PostProcessShadowApply>();
	GeneratePostProcess3DPipeline<PostProcessFog>();

	/// ----- 2D/UIを含む画面全体にかけるポストエフェクトのパイプラインを生成 ----- ///
	GeneratePostProcessScreenPipeline<PostProcessGrayscale>();
	GeneratePostProcessScreenPipeline<PostProcessRadialBlur>();
	GeneratePostProcessScreenPipeline<PostProcessWaterCausticsLightShafts>();
	GeneratePostProcessScreenPipeline<PostProcessWaterDepthFogVignette>();
	GeneratePostProcessScreenPipeline<PostProcessWaterColorGrading>();
	GeneratePostProcessScreenPipeline<PostProcessWaterDistortion>();
	GeneratePostProcessScreenPipeline<PostProcessFisheye>();
}

void RenderingPipelineCollection::PreDrawEntities(CameraComponent* _3dCamera, CameraComponent* _2dCamera) {

	/// ----- すべてのPipelineのPreDrawを実行する ----- ///
	ECSGroup* ecsGroup = pEntityComponentSystem_->GetCurrentGroup();

	/// 2d,3d 両方ともカメラが有効かチェックしてから描画する
	if(IsEnableCamera(_3dCamera)) {
		for(auto& renderer : renderer3ds_) {
			renderer->PreDraw(ecsGroup, _3dCamera, pDxManager_->GetDxCommand());
		}
		if(particleRenderer_) particleRenderer_->PreDraw(ecsGroup, _3dCamera, pDxManager_->GetDxCommand());
		if(gizmo3D_)     gizmo3D_->PreDraw(ecsGroup, _3dCamera, pDxManager_->GetDxCommand());
	} else {
		// Console::LogError("RenderingPipelineCollection::DrawEntities: 3D Camera is null");
	}

	if(IsEnableCamera(_2dCamera)) {
		for(auto& renderer : renderer2ds_) {
			renderer->PreDraw(ecsGroup, _2dCamera, pDxManager_->GetDxCommand());
		}
		if(gizmo2D_)     gizmo2D_->PreDraw(ecsGroup, _2dCamera, pDxManager_->GetDxCommand());
	} else {
		// Console::LogError("RenderingPipelineCollection::DrawEntities: 2D Camera is null");
	}
}

void RenderingPipelineCollection::DrawEntities(CameraComponent* _3dCamera, CameraComponent* _2dCamera) {

	/// シーンを描画するので現在のGroupを使用する
	ECSGroup* ecsGroup = pEntityComponentSystem_->GetCurrentGroup();

	/// 3dカメラが有効なら3D描画を実行
	if(IsEnableCamera(_3dCamera)) {
		for(auto& renderer : renderer3ds_) {
			renderer->Draw(ecsGroup, _3dCamera, pDxManager_->GetDxCommand());
		}
	}
}

void RenderingPipelineCollection::DrawParticles(CameraComponent* _3dCamera) {
	if(particleRenderer_ && IsEnableCamera(_3dCamera)) {
		particleRenderer_->Draw(pEntityComponentSystem_->GetCurrentGroup(), _3dCamera, pDxManager_->GetDxCommand());
	}
}

void RenderingPipelineCollection::DrawGizmos(CameraComponent* _3dCamera, CameraComponent* _2dCamera) {
#ifdef DEBUG_MODE
	if(gizmo3D_) {
		gizmo3D_->Draw(pEntityComponentSystem_->GetCurrentGroup(), _3dCamera, pDxManager_->GetDxCommand());
	}
	if(gizmo2D_) {
		gizmo2D_->Draw(pEntityComponentSystem_->GetCurrentGroup(), _2dCamera, pDxManager_->GetDxCommand());
	}
	Gizmo::Reset();
#endif
}

void RenderingPipelineCollection::DrawEntities2D(CameraComponent* _2dCamera, const std::string& groupName) {
	/// 対象のGroupを取得
	ECSGroup* ecsGroup = groupName.empty() ? pEntityComponentSystem_->GetCurrentGroup() : pEntityComponentSystem_->GetECSGroup(groupName);
	if(!ecsGroup) return;

	/// 2dカメラが有効なら2D描画を実行
	if(IsEnableCamera(_2dCamera)) {
		// 検証用ログ
		static int drawLogCount = 0;
		if(drawLogCount < 10) {
			Console::Log("[RenderingCollection] DrawEntities2D executing. Group: " + ecsGroup->GetGroupName() + " Camera: " + std::to_string((uint64_t)_2dCamera), LogCategory::Engine);
			drawLogCount++;
		}

		for(auto& renderer : renderer2ds_) {
			renderer->Draw(ecsGroup, _2dCamera, pDxManager_->GetDxCommand());
		}
	} else {
		static int failLogCount = 0;
		if(failLogCount < 10) {
			std::string camInfo = _2dCamera ? "Present but invalid" : "Null";
			Console::Log("[RenderingCollection] DrawEntities2D skipped. Group: " + ecsGroup->GetGroupName() + " Camera: " + camInfo, LogCategory::Engine);
			failLogCount++;
		}
	}
}

void RenderingPipelineCollection::DrawSelectedPrefab(CameraComponent* _3dCamera, CameraComponent* _2dCamera) {
	/// ----- 選択されているPrefabの描画 ----- ///

	/// デバッグ用のGroupを使用する
	ECSGroup* ecsGroup = pEntityComponentSystem_->GetECSGroup("Debug");

	/// 3dカメラが有効なら3D描画を実行
	if(IsEnableCamera(_3dCamera)) {
		for(auto& renderer : renderer3ds_) {
			renderer->Draw(ecsGroup, _3dCamera, pDxManager_->GetDxCommand());
		}
	}
}

void RenderingPipelineCollection::DrawSelectedPrefab2D(CameraComponent* _2dCamera, const std::string& groupName) {
	/// デバッグ用のGroupを使用する
	std::string targetGroup = groupName.empty() ? "Debug" : groupName;
	ECSGroup* ecsGroup = pEntityComponentSystem_->GetECSGroup(targetGroup);
	if(!ecsGroup) return;

	/// 2dカメラが有効なら2D描画を実行
	if(IsEnableCamera(_2dCamera)) {
		for(auto& renderer : renderer2ds_) {
			renderer->Draw(ecsGroup, _2dCamera, pDxManager_->GetDxCommand());
		}
	}
}


void RenderingPipelineCollection::ExecutePostProcess3D(const std::string& sceneTextureName) {
	// 検証用ログ
	static int post3DLogCount = 0;
	if(post3DLogCount < 10) {
		Console::Log("[RenderingCollection] ExecutePostProcess3D for scene: " + sceneTextureName, LogCategory::Engine);
		post3DLogCount++;
	}

	for(auto& postProcess : postProcesses3D_) {
		postProcess->Execute(sceneTextureName, pDxManager_->GetDxCommand(), pAssetCollection_, pEntityComponentSystem_);
	}
}

void RenderingPipelineCollection::ExecutePostProcessScreen(const std::string& sceneTextureName) {
	// 検証用ログ
	static int postScreenLogCount = 0;
	if(postScreenLogCount < 10) {
		Console::Log("[RenderingCollection] ExecutePostProcessScreen for scene: " + sceneTextureName, LogCategory::Engine);
		postScreenLogCount++;
	}

	for(auto& postProcess : postProcessesScreen_) {
		postProcess->Execute(sceneTextureName, pDxManager_->GetDxCommand(), pAssetCollection_, pEntityComponentSystem_);
	}
}

bool RenderingPipelineCollection::IsEnableCamera(const CameraComponent* camera) const {
	/*
	* チェック項目
	* 1, カメラのポインタが有効
	* 2, Componentの有効フラグがtrue
	* 3, Bufferとして利用できるViewProjectionがあるか
	*/

	if(!camera) {
		// static int nullCamLog = 0;
		// if (nullCamLog < 1) { Console::Log("[RenderingCollection] IsEnableCamera: Camera is null", LogCategory::Engine); nullCamLog++; }
		return false;
	}

	if(!camera->enable) {
		static int disableCamLog = 0;
		if(disableCamLog < 10) { Console::Log("[RenderingCollection] IsEnableCamera: Camera is disabled", LogCategory::Engine); disableCamLog++; }
		return false;
	}

	if(!camera->IsMakeViewProjection()) {
		static int noVPLog = 0;
		if(noVPLog < 10) { Console::Log("[RenderingCollection] IsEnableCamera: Camera ViewProjection is not ready", LogCategory::Engine); noVPLog++; }
		return false;
	}

	return true;
}
