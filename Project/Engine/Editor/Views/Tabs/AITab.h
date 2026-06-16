#pragma once

#include "../EditorViewCollection.h"

namespace ONEngine {
class DxManager;
class EntityComponentSystem;
class SceneManager;
}

namespace Editor {

class EditorManager;

class AITab : public IEditorWindowContainer {
public:
	AITab(
		ONEngine::DxManager* dxm,
		ONEngine::EntityComponentSystem* ecs,
		EditorManager* editorManager,
		ONEngine::SceneManager* sceneManager);
	~AITab() override = default;
};

} /// Editor
