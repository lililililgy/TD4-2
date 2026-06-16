#pragma once

/// engine
#include "../EditorViewCollection.h"	

/// ///////////////////////////////////////////////////
/// エディターウィンドウ
/// ///////////////////////////////////////////////////
namespace Editor {

class PrefabTab : public IEditorWindowContainer {
public:
	/// ===================================================
	/// public : methods   
	/// ===================================================

	PrefabTab(
		ONEngine::DxManager* dxm,
		ONEngine::EntityComponentSystem* ecs, 
		ONEngine::Asset::AssetCollection* assetCollection,
		EditorManager* editorManager, 
		ONEngine::SceneManager* sceneManager
	);
	~PrefabTab() {}

};

} /// Editor
