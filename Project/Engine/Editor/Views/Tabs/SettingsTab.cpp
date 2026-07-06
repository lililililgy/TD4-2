#include "SettingsTab.h"

/// editor
#include "../Windows/Develop/ProjectSettingsWindow.h"

using namespace Editor;

SettingsTab::SettingsTab() : IEditorWindowContainer("Settings") {
	AddView(std::make_unique<ProjectSettingsWindow>());
}
