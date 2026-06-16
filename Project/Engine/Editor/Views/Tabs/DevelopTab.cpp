#include "DevelopTab.h"


/// external
#include <imgui.h>

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "../Windows/Develop/ProjectWindow.h"
#include "../Windows/Develop/GameSceneView.h"
#include "../Windows/Develop/DebugSceneView.h"
#include "../Windows/Develop/InspectorWindow.h"
#include "../Windows/Develop/HierarchyWindow.h"
#include "../Windows/Develop/ConsoleWindow.h"
#include "../Windows/Develop/TexturePreviewWindow.h"
#include "../Windows/Develop/BehaviorTreeEditorWindow.h"
#include "../Windows/Develop/AnimationEditorWindow.h"

using namespace ONEngine;
using namespace Editor;

DevelopTab::DevelopTab(
	DxManager* dxm,
	EntityComponentSystem* ecs, Asset::AssetCollection* assetCollection,
	EditorManager* editorManager, SceneManager* sceneManager)
	: IEditorWindowContainer("Game") {

	/// 子windowの追加
	InspectorWindow* inspector = static_cast<InspectorWindow*>(AddView(std::make_unique<InspectorWindow>("Inspector##Game", dxm, ecs, assetCollection, editorManager)));
	AddView(std::make_unique<ProjectWindow>(assetCollection));
	AddView(std::make_unique<GameSceneView>(assetCollection, "GameScene"));
	AddView(std::make_unique<NormalHierarchyWindow>("Hierarchy", ecs, editorManager, sceneManager));
	AddView(std::make_unique<HierarchyWindow>("DebugHierarchy", ecs, ecs->GetECSGroup("Debug"), editorManager, sceneManager));
	AddView(std::make_unique<DebugSceneView>(ecs, assetCollection, sceneManager, inspector));
	AddView(std::make_unique<ConsoleWindow>());
	AddView(std::make_unique<TexturePreviewWindow>(assetCollection));
}
