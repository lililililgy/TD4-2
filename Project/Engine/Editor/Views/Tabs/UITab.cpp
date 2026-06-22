#include "UITab.h"

/// external
#include <imgui.h>

/// engine
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "../Windows/Develop/InspectorWindow.h"
#include "../Windows/Develop/HierarchyWindow.h"
#include "../Windows/Develop/UIEditorWindow.h"

using namespace Editor;

UITab::UITab(
	ONEngine::DxManager* dxm,
	ONEngine::EntityComponentSystem* ecs,
	ONEngine::Asset::AssetCollection* assetCollection,
	EditorManager* editorManager,
	ONEngine::SceneManager* sceneManager)
	: IEditorWindowContainer("UI Editor") {

	// Inspector for editing UI Node properties
	InspectorWindow* inspector = static_cast<InspectorWindow*>(
		AddView(std::make_unique<InspectorWindow>("Inspector##UI", dxm, ecs, assetCollection, editorManager)));

	// Main Node-Editor Window
	AddView(std::make_unique<UIEditorWindow>("UI Node Editor", ecs, assetCollection, inspector));

	// Hierarchy specifically for looking at the UI elements in the game scene
	AddView(std::make_unique<NormalHierarchyWindow>("Hierarchy##UI", ecs, editorManager, sceneManager));
}
