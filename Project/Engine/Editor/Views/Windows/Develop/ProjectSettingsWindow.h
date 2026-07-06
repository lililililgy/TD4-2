#pragma once

/// std
#include <vector>
#include <string>

/// engine
#include "../../EditorViewCollection.h"

namespace Editor {

class ProjectSettingsWindow : public IEditorWindow {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================
	ProjectSettingsWindow();
	~ProjectSettingsWindow() {}

	/// @brief imgui windowの描画処理
	void ShowImGui() override;

private:
	/// ===================================================
	/// private : methods
	/// ===================================================
	void RefreshScenes();

private:
	std::vector<std::string> availableScenes_;
};

} /// namespace Editor
