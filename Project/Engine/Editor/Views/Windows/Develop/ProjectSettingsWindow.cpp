#include "ProjectSettingsWindow.h"

/// std
#include <filesystem>
#include <fstream>

/// external
#include <imgui.h>

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/Utility/Tools/Log.h"

using namespace Editor;

ProjectSettingsWindow::ProjectSettingsWindow() {
	RefreshScenes();
}

void ProjectSettingsWindow::RefreshScenes() {
	availableScenes_.clear();
	std::string sceneDir = "./Assets/Scene/";
	if (std::filesystem::exists(sceneDir)) {
		for (const auto& entry : std::filesystem::directory_iterator(sceneDir)) {
			if (entry.is_regular_file() && entry.path().extension() == ".scene") {
				availableScenes_.push_back(entry.path().stem().string());
			}
		}
	}
}

void ProjectSettingsWindow::ShowImGui() {
	if (!ImGui::Begin("Project Settings")) {
		ImGui::End();
		return;
	}

	ImGui::Text("Project / Engine Settings");
	ImGui::Separator();

	ImGui::Spacing();

	// 1. 一般ウィンドウ設定
	ImGui::Text("Window / Screen Settings");
	static char titleBuf[256] = "";
	static bool isTitleInitialized = false;
	if (!isTitleInitialized) {
		strcpy_s(titleBuf, ONEngine::EngineConfig::windowTitle.c_str());
		isTitleInitialized = true;
	}

	if (ImGui::InputText("Window Title", titleBuf, sizeof(titleBuf))) {
		ONEngine::EngineConfig::windowTitle = titleBuf;
	}
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::EngineConfig::SaveConfig();
		ONEngine::Console::Log("ProjectSettings: Window title changed to: " + ONEngine::EngineConfig::windowTitle);
	}

	// 解像度設定
	const char* resolutions[] = { "1920 x 1080", "1600 x 900", "1280 x 720", "1024 x 576" };
	int resWidths[] = { 1920, 1600, 1280, 1024 };
	int resHeights[] = { 1080, 900, 720, 576 };

	std::string currentResStr = std::to_string(ONEngine::EngineConfig::windowWidth) + " x " + std::to_string(ONEngine::EngineConfig::windowHeight);
	if (ImGui::BeginCombo("Preset Resolutions", currentResStr.c_str())) {
		for (int i = 0; i < 4; i++) {
			bool isSelected = (ONEngine::EngineConfig::windowWidth == resWidths[i] && ONEngine::EngineConfig::windowHeight == resHeights[i]);
			if (ImGui::Selectable(resolutions[i], isSelected)) {
				ONEngine::EngineConfig::windowWidth = resWidths[i];
				ONEngine::EngineConfig::windowHeight = resHeights[i];
				ONEngine::EngineConfig::SaveConfig();
				ONEngine::Console::Log("ProjectSettings: Resolution changed to " + std::string(resolutions[i]));
			}
		}
		ImGui::EndCombo();
	}

	int currentRes[2] = { ONEngine::EngineConfig::windowWidth, ONEngine::EngineConfig::windowHeight };
	if (ImGui::InputInt2("Custom Resolution", currentRes)) {
		ONEngine::EngineConfig::windowWidth = currentRes[0];
		ONEngine::EngineConfig::windowHeight = currentRes[1];
	}
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::EngineConfig::SaveConfig();
		ONEngine::Console::Log("ProjectSettings: Custom resolution saved");
	}

	bool isFullscreen = ONEngine::EngineConfig::isFullscreen;
	if (ImGui::Checkbox("Fullscreen (Release Build only)", &isFullscreen)) {
		ONEngine::EngineConfig::isFullscreen = isFullscreen;
		ONEngine::EngineConfig::SaveConfig();
		ONEngine::Console::Log(std::string("ProjectSettings: Fullscreen mode ") + (isFullscreen ? "Enabled" : "Disabled"));
	}

	bool enableVSync = ONEngine::EngineConfig::enableVSync;
	if (ImGui::Checkbox("Enable VSync", &enableVSync)) {
		ONEngine::EngineConfig::enableVSync = enableVSync;
		ONEngine::EngineConfig::SaveConfig();
		ONEngine::Console::Log(std::string("ProjectSettings: VSync ") + (enableVSync ? "Enabled" : "Disabled"));
	}

	ImGui::Separator();
	ImGui::Spacing();

	// 2. 開始シーンの設定
	ImGui::Text("Startup Scene Settings");
	if (ImGui::Button("Refresh Available Scenes")) {
		RefreshScenes();
	}

	std::string currentStartScene = ONEngine::EngineConfig::startScene;
	if (ImGui::BeginCombo("Start Scene (Release)", currentStartScene.c_str())) {
		for (const auto& scene : availableScenes_) {
			bool isSelected = (currentStartScene == scene);
			if (ImGui::Selectable(scene.c_str(), isSelected)) {
				ONEngine::EngineConfig::startScene = scene;
				ONEngine::EngineConfig::SaveConfig();
				ONEngine::Console::Log("ProjectSettings: Start scene changed to " + scene);
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::End();
}
