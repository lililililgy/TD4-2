#pragma once

/// editor
#include "../EditorViewCollection.h"

namespace ONEngine {
/// 前方宣言
class DxManager;
class EntityComponentSystem;
class SceneManager;
} // namespace ONEngine

namespace ONEngine::Asset {
class AssetCollection;
}


namespace Editor {

/// 前方宣言
class EditorManager;

/// ///////////////////////////////////////////////////
/// GameWindow
/// ///////////////////////////////////////////////////
class DevelopTab : public IEditorWindowContainer {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	DevelopTab(ONEngine::DxManager* dxm,
		ONEngine::EntityComponentSystem* ecs,
		ONEngine::Asset::AssetCollection* assetCollection,
		EditorManager* editorManager,
		ONEngine::SceneManager* sceneManager);
	~DevelopTab() {}

};

} // namespace Editor
