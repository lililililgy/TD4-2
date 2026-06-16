#include "GameTab.h"

/// editor
#include "../Windows/Develop/GameSceneView.h"

using namespace Editor;

GameTab::GameTab(ONEngine::Asset::AssetCollection* ac)
	: IEditorWindowContainer("Game") {

	AddView(std::make_unique<GameSceneView>(ac, "GameView##GameSceneView"));
}
