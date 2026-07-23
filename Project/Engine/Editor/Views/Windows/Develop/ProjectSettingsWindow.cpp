#include "ProjectSettingsWindow.h"

/// std
#include <filesystem>
#include <fstream>

/// external
#include <imgui.h>

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/Script/MonoScriptEngine.h"

using namespace Editor;

namespace {
int GetGCD(int a, int b) {
	while (b != 0) {
		int r = a % b;
		a = b;
		b = r;
	}
	return a;
}

std::string GetAspectRatioString(int width, int height) {
	if (width <= 0 || height <= 0) return "";
	int gcd = GetGCD(width, height);
	return std::to_string(width / gcd) + ":" + std::to_string(height / gcd);
}

struct ResolutionPreset {
	int width;
	int height;
	const char* name;
};

const ResolutionPreset kResolutionPresets[] = {
	// 16:9
	{ 3840, 2160, "4K UHD" },
	{ 2560, 1440, "WQHD" },
	{ 1920, 1080, "FHD" },
	{ 1600, 900, nullptr },
	{ 1280, 720, "HD" },
	{ 1024, 576, nullptr },

	// 16:10
	{ 2560, 1600, "WQXGA" },
	{ 1920, 1200, "WUXGA" },
	{ 1280, 800, "WXGA" },

	// 4:3
	{ 1600, 1200, "UXGA" },
	{ 1024, 768, "XGA" },
	{ 800, 600, "SVGA" },
	{ 640, 480, "VGA" },

	// 21:9
	{ 3440, 1440, "UWQHD" },
	{ 2560, 1080, "UWFHD" }
};
}

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
	int currentWidth = ONEngine::EngineConfig::windowWidth;
	int currentHeight = ONEngine::EngineConfig::windowHeight;
	std::string currentAspect = GetAspectRatioString(currentWidth, currentHeight);
	std::string currentResStr = std::to_string(currentWidth) + " x " + std::to_string(currentHeight);
	if (!currentAspect.empty()) {
		currentResStr += " (" + currentAspect + ")";
	}

	if (ImGui::BeginCombo("Preset Resolutions", currentResStr.c_str())) {
		for (const auto& preset : kResolutionPresets) {
			std::string aspect = GetAspectRatioString(preset.width, preset.height);
			std::string nameStr = preset.name ? (std::string(" [") + preset.name + "]") : "";
			std::string itemStr = std::to_string(preset.width) + " x " + std::to_string(preset.height) + " (" + aspect + ")" + nameStr;

			bool isSelected = (currentWidth == preset.width && currentHeight == preset.height);
			if (ImGui::Selectable(itemStr.c_str(), isSelected)) {
				ONEngine::EngineConfig::windowWidth = preset.width;
				ONEngine::EngineConfig::windowHeight = preset.height;
				ONEngine::EngineConfig::SaveConfig();
				ONEngine::Console::Log("ProjectSettings: Resolution changed to " + itemStr);
			}
		}
		ImGui::EndCombo();
	}

	int currentRes[2] = { currentWidth, currentHeight };
	if (ImGui::InputInt2("Custom Resolution", currentRes)) {
		ONEngine::EngineConfig::windowWidth = currentRes[0];
		ONEngine::EngineConfig::windowHeight = currentRes[1];
	}
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::EngineConfig::SaveConfig();
		ONEngine::Console::Log("ProjectSettings: Custom resolution saved");
	}

	// 現在の解像度に基づくアスペクト比をプレビュー表示
	std::string customAspect = GetAspectRatioString(ONEngine::EngineConfig::windowWidth, ONEngine::EngineConfig::windowHeight);
	if (!customAspect.empty()) {
		ImGui::Text("Current Aspect Ratio: %s", customAspect.c_str());
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

	bool enableReleaseLogFile = ONEngine::EngineConfig::enableReleaseLogFile;
	if (ImGui::Checkbox("Enable Release Log File", &enableReleaseLogFile)) {
		ONEngine::EngineConfig::enableReleaseLogFile = enableReleaseLogFile;
		ONEngine::EngineConfig::SaveConfig();
		ONEngine::Console::Log(std::string("ProjectSettings: Release Log File ") + (enableReleaseLogFile ? "Enabled" : "Disabled"));
	}

	ImGui::Spacing();

	bool ignoreCSharpLog = ONEngine::EngineConfig::ignoreCSharpLog;
	if (ImGui::Checkbox("Ignore C# Log", &ignoreCSharpLog)) {
		ONEngine::EngineConfig::ignoreCSharpLog = ignoreCSharpLog;
		ONEngine::EngineConfig::SaveConfig();
		ONEngine::MonoScriptEngine::GetInstance().ApplyCSharpLogSetting();
		ONEngine::Console::Log(std::string("ProjectSettings: Ignore C# Log ") + (ignoreCSharpLog ? "Enabled" : "Disabled"));
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
