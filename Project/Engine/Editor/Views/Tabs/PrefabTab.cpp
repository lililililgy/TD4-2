#include "PrefabTab.h"

/// external
#include <imgui.h>

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "../Windows/Prefab/PrefabViewWindow.h"
#include "../Windows/Prefab/PrefabFileWindow.h"
#include "../Windows/Develop/InspectorWindow.h"
#include "../Windows/Develop/ProjectWindow.h"
#include "../Windows/Develop/HierarchyWindow.h"

using namespace Editor;

PrefabTab::PrefabTab(
	ONEngine::DxManager* dxm,
	ONEngine::EntityComponentSystem* ecs,
	ONEngine::Asset::AssetCollection* assetCollection,
	EditorManager* editorManager,
	ONEngine::SceneManager* sceneManager)
	: IEditorWindowContainer("Prefab") {

	/// 子windowの追加
	InspectorWindow* inspector = static_cast<InspectorWindow*>(
		AddView(std::make_unique<InspectorWindow>("Inspector##Prefab", dxm, ecs, assetCollection, editorManager)));

	AddView(std::make_unique<PrefabFileWindow>(ecs, assetCollection, inspector));
	AddView(std::make_unique<PrefabViewWindow>(ecs, assetCollection));
	AddView(std::make_unique<HierarchyWindow>("Hierarchy##Prefab", ecs, ecs->GetECSGroup("Debug"), editorManager, sceneManager));
	ProjectWindow* project = static_cast<ProjectWindow*>(AddView(std::make_unique<ProjectWindow>(assetCollection)));

	project->SetWindowName("Prefab Project");

}
