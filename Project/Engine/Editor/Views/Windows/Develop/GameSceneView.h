#pragma once

/// engine
#include "../../EditorViewCollection.h"

namespace ONEngine::Asset {
class AssetCollection;
}

namespace Editor {

/// ///////////////////////////////////////////////////
/// GameSceneTextureを表示するためのImGuiWindow
/// ///////////////////////////////////////////////////
class GameSceneView : public IEditorWindow {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================
	GameSceneView(ONEngine::Asset::AssetCollection* ac, const std::string& windowName)
		: pAssetCollection_(ac), windowName_(windowName) {
	}
	~GameSceneView() {}

	/// @brief imgui windowの描画処理
	void ShowImGui() override;

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	ONEngine::Asset::AssetCollection* pAssetCollection_ = nullptr;
	const std::string windowName_ = "GameView";

};

} /// Editor
