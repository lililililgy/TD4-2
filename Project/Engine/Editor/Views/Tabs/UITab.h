#pragma once

/// engine
#include "../EditorViewCollection.h"

namespace Editor {

class UITab : public IEditorWindowContainer {
public:
	UITab(
		ONEngine::DxManager* dxm,
		ONEngine::EntityComponentSystem* ecs,
		ONEngine::Asset::AssetCollection* assetCollection,
		EditorManager* editorManager,
		ONEngine::SceneManager* sceneManager
	);
	~UITab() override = default;
};

} /// namespace Editor
