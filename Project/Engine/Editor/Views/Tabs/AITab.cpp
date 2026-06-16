#include "AITab.h"

/// external
#include <imgui.h>

/// engine
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "../Windows/Develop/BehaviorTreeEditorWindow.h"

using namespace ONEngine;
using namespace Editor;

AITab::AITab(
	DxManager* dxm,
	EntityComponentSystem* ecs,
	EditorManager* editorManager, SceneManager* sceneManager)
	: IEditorWindowContainer("AI") {

	(void)dxm;
	(void)editorManager;
	(void)sceneManager;

	/// BehaviorTreeEditorWindowを追加
	AddView(std::make_unique<BehaviorTreeEditorWindow>("AI Behavior Tree", ecs));
}
