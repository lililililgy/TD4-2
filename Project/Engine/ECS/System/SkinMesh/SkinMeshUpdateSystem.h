#pragma once

/// engine
#include "../Interface/ECSISystem.h"
#include "Engine/ECS/Component/Components/RendererComponents/SkinMesh/SkinMeshRenderer.h"

namespace ONEngine {
class DxManager;
class ECSGroup;
class SkinMeshRenderer;
}

namespace ONEngine::Asset {
class AssetCollection;
}


/// /////////////////////////////////////////////
/// スキンメッシュの更新システム
/// /////////////////////////////////////////////
namespace ONEngine {

class SkinMeshUpdateSystem : public ECSISystem {
public:

	SkinMeshUpdateSystem(DxManager* dxm, Asset::AssetCollection* assetCollection);
	~SkinMeshUpdateSystem() override = default;

	void RuntimeUpdate(ECSGroup* ecs) override;

	/// @brief スケルトンの更新
	void UpdateSkeleton(SkinMeshRenderer* smr);

	/// @brief スキンクラスターの更新
	void UpdateSkinCluster(SkinMeshRenderer* smr);

private:
	/// @brief 再帰的にスケルトンを更新
	void UpdateSkeletonRecursive(SkinMeshRenderer* smr, int32_t jointIndex, const std::optional<int32_t>& parentIndex);

	/// =========================================
	/// private : objects
	/// =========================================

	Asset::AssetCollection* pAssetCollection_; ///< グラフィックスリソースコレクション
	DxManager* pDxManager_; ///< DirectXマネージャー

};


} /// ONEngine
