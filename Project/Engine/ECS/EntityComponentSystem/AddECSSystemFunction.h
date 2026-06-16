#pragma once


namespace ONEngine {
class DxManager;
class ECSGroup;
}

namespace ONEngine::Asset {
class AssetCollection;
}


namespace ONEngine {

/// ゲーム用のECSGroupにシステムを追加する関数
void GameECSGroupAddSystemFunction(ECSGroup* ecs, DxManager* dxm, Asset::AssetCollection* assetCollection);

/// デバッグ用のECSGroupにシステムを追加する関数
void DebugECSGroupAddSystemFunction(ECSGroup* ecs, DxManager* dxm, Asset::AssetCollection* assetCollection);

} /// namespace ONEngine