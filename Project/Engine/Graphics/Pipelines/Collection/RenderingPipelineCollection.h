#pragma once

/// std
#include <vector>
#include <memory>

/// engine
#include "../Interface/IRenderingPipeline.h"
#include "../Interface/IPostProcessPipeline.h"


namespace ONEngine {
class DxManager;
class EntityComponentSystem;
class CameraComponent;
class Gizmo3DRenderingPipeline;
class Gizmo2DRenderingPipeline;
}

namespace ONEngine::Asset {
class AssetCollection;
}



namespace ONEngine {

template <typename T>
concept IsRenderingPipeline = std::is_base_of_v<IRenderingPipeline, T>;

template <typename T>
concept IsPostProcessPipeline = std::is_base_of_v<IPostProcessPipeline, T>;

/// ///////////////////////////////////////////////////
/// renderer collection
/// ///////////////////////////////////////////////////
class RenderingPipelineCollection final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	RenderingPipelineCollection(ShaderCompiler* shaderCompiler, DxManager* dxm, EntityComponentSystem* pEntityComponentSystem, Asset::AssetCollection* assetCollection);
	~RenderingPipelineCollection();

	/// @brief 初期化関数
	void Initialize();



	/// @brief rendering pipelineの生成
	/// @tparam T 生成する rendering pipelineの型
	template <IsRenderingPipeline T, typename... Args>
	void Generate3DRenderingPipeline(Args&&... args);

	template <IsRenderingPipeline T, typename... Args>
	void Generate2DRenderingPipeline(Args&&... args);

	template <IsPostProcessPipeline T, typename... Args>
	void GeneratePostProcess3DPipeline(Args&&... args);

	template <IsPostProcessPipeline T, typename... Args>
	void GeneratePostProcessScreenPipeline(Args&&... args);



	/// @brief すべてのPipelineのPreDrawを実行する
	/// @param ecsGroup 対象のECSGroup
	/// @param _3dCamera 3Dカメラ
	/// @param _2dCamera 2Dカメラ
	void PreDrawEntities(ECSGroup* ecsGroup, CameraComponent* _3dCamera, CameraComponent* _2dCamera);

	/// @brief 現在のECSGroupのすべてのEntityを描画する
	/// @param ecsGroup 対象のECSGroup
	/// @param _3dCamera 3Dカメラ
	/// @param _2dCamera 2DCamera
	void DrawEntities(ECSGroup* ecsGroup, CameraComponent* _3dCamera, CameraComponent* _2dCamera);

	/// @brief パーティクルの描画 (ポストエフェクト後に実行)
	/// @param ecsGroup 対象のECSGroup
	void DrawParticles(ECSGroup* ecsGroup, CameraComponent* _3dCamera);

	/// @brief Gizmoの描画
	/// @param ecsGroup 対象のECSGroup
	/// @param _3dCamera 3Dカメラ
	/// @param _2dCamera 2Dカメラ
	void DrawGizmos(ECSGroup* ecsGroup, CameraComponent* _3dCamera, CameraComponent* _2dCamera);

	/// @brief 2DのEntityを描画する
	/// @param ecsGroup 対象のECSGroup
	/// @param _2dCamera 2Dカメラ
	void DrawEntities2D(ECSGroup* ecsGroup, CameraComponent* _2dCamera);

	/// @brief 選択されたPrefabの描画
	/// @param ecsGroup 対象のECSGroup
	/// @param _3dCamera 3Dカメラ
	/// @param _2dCamera 2Dカメラ
	void DrawSelectedPrefab(ECSGroup* ecsGroup, CameraComponent* _3dCamera, CameraComponent* _2dCamera);

	/// @brief 選択されたPrefabの2D描画
	/// @param ecsGroup 対象のECSGroup
	/// @param _2dCamera 2Dカメラ
	void DrawSelectedPrefab2D(ECSGroup* ecsGroup, CameraComponent* _2dCamera);


	/// @brief 3Dポストエフェクトの実行
	/// @param sceneTextureName シーンの名前 (Debug, Game, Prefab etc...)
	/// @param ecsGroup 対象のECSGroup (nullptrの場合はカレントグループ)
	void ExecutePostProcess3D(const std::string& sceneTextureName, ECSGroup* ecsGroup = nullptr);

	/// @brief 画面全体ポストエフェクトの実行 (2D/UIを含む)
	/// @param sceneTextureName シーンの名前 (Debug, Game, Prefab etc...)
	/// @param ecsGroup 対象のECSGroup (nullptrの場合はカレントグループ)
	void ExecutePostProcessScreen(const std::string& sceneTextureName, ECSGroup* ecsGroup = nullptr);


	/// @brief 引数のカメラが有効なのか確認する
	/// @param camera チェックしたいカメラ
	/// @return true: 有効, false: 無効
	bool IsEnableCamera(const CameraComponent* camera) const;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	DxManager* pDxManager_;
	EntityComponentSystem* pEntityComponentSystem_;
	Asset::AssetCollection* pAssetCollection_;
	ShaderCompiler* pShaderCompiler_;

	std::unique_ptr<IRenderingPipeline> prefabRenderingPipeline_;

	std::vector<std::unique_ptr<IRenderingPipeline>>   renderer3ds_;
	std::vector<std::unique_ptr<IRenderingPipeline>>   renderer2ds_;

	std::unique_ptr<IRenderingPipeline>   particleRenderer_;
	std::unique_ptr<Gizmo3DRenderingPipeline> gizmo3D_;
	std::unique_ptr<Gizmo2DRenderingPipeline> gizmo2D_;

	std::vector<std::unique_ptr<IPostProcessPipeline>> postProcesses3D_;
	std::vector<std::unique_ptr<IPostProcessPipeline>> postProcessesScreen_;
};



/// ===================================================
/// inline methods
/// ===================================================

template<IsRenderingPipeline T, typename... Args>
inline void RenderingPipelineCollection::Generate3DRenderingPipeline(Args&&... args) {
	std::unique_ptr<T> renderer = std::make_unique<T>(std::forward<Args>(args)...);
	renderer->Initialize(pShaderCompiler_, pDxManager_);
	renderer3ds_.push_back(std::move(renderer));
}

template<IsRenderingPipeline T, typename... Args>
inline void RenderingPipelineCollection::Generate2DRenderingPipeline(Args&&... args) {
	std::unique_ptr<T> renderer = std::make_unique<T>(std::forward<Args>(args)...);
	renderer->Initialize(pShaderCompiler_, pDxManager_);
	renderer2ds_.push_back(std::move(renderer));
}

template<IsPostProcessPipeline T, typename... Args>
	inline void RenderingPipelineCollection::GeneratePostProcess3DPipeline(Args&&... args) {
	std::unique_ptr<T> postProcess = std::make_unique<T>();
	postProcess->Initialize(pShaderCompiler_, pDxManager_);
	postProcesses3D_.push_back(std::move(postProcess));
}

template<IsPostProcessPipeline T, typename... Args>
inline void RenderingPipelineCollection::GeneratePostProcessScreenPipeline(Args&&... args) {
	std::unique_ptr<T> postProcess = std::make_unique<T>();
	postProcess->Initialize(pShaderCompiler_, pDxManager_);
	postProcessesScreen_.push_back(std::move(postProcess));
}

} /// ONEngine
