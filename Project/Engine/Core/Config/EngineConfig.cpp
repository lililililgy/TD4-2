#include "EngineConfig.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using namespace ONEngine;

namespace ONEngine::DebugConfig {
	/// @brief デバッグ中かどうか
	bool isDebugging = false;
	bool isPause = false;
	bool isShowDebugScene = true;
	bool isShowGameScene = true;

	int selectedMode_ = 0; ///< 選択中のデバッグウィンドウ
}

namespace ONEngine::EngineConfig {
	std::string startScene = "TitleScene";
	std::string windowTitle = "TwoEngine";
	int windowWidth = 1920;
	int windowHeight = 1080;
	bool isFullscreen = false;
	bool enableVSync = true;

	void LoadConfig() {
		std::string configPath = "./Assets/engine_config.json";
		if (!std::filesystem::exists(configPath)) {
			// デフォルトの設定の決定
			std::string defaultScene = "TitleScene";
			std::string sceneDir = "./Assets/Scene/";
			if (std::filesystem::exists(sceneDir)) {
				for (const auto& entry : std::filesystem::directory_iterator(sceneDir)) {
					if (entry.is_regular_file() && entry.path().extension() == ".scene") {
						std::string sceneName = entry.path().stem().string();
						if (sceneName != "Debug") {
							defaultScene = sceneName;
							break;
						}
					}
				}
			}
			startScene = defaultScene;
			SaveConfig();
		} else {
			std::ifstream ifs(configPath);
			if (ifs.is_open()) {
				nlohmann::json j;
				try {
					ifs >> j;
					if (j.contains("startScene") && j["startScene"].is_string()) {
						startScene = j["startScene"].get<std::string>();
					}
					if (j.contains("windowTitle") && j["windowTitle"].is_string()) {
						windowTitle = j["windowTitle"].get<std::string>();
					}
					if (j.contains("windowWidth") && j["windowWidth"].is_number_integer()) {
						windowWidth = j["windowWidth"].get<int>();
					}
					if (j.contains("windowHeight") && j["windowHeight"].is_number_integer()) {
						windowHeight = j["windowHeight"].get<int>();
					}
					if (j.contains("isFullscreen") && j["isFullscreen"].is_boolean()) {
						isFullscreen = j["isFullscreen"].get<bool>();
					}
					if (j.contains("enableVSync") && j["enableVSync"].is_boolean()) {
						enableVSync = j["enableVSync"].get<bool>();
					}
				} catch (const std::exception&) {
					// 読み込みに失敗した場合はデフォルト値のまま
				}
				ifs.close();
			}
		}
	}

	void SaveConfig() {
		std::string configPath = "./Assets/engine_config.json";
		nlohmann::json j;
		j["startScene"] = startScene;
		j["windowTitle"] = windowTitle;
		j["windowWidth"] = windowWidth;
		j["windowHeight"] = windowHeight;
		j["isFullscreen"] = isFullscreen;
		j["enableVSync"] = enableVSync;
		std::ofstream ofs(configPath);
		if (ofs.is_open()) {
			ofs << j.dump(4);
			ofs.close();
		}
	}
}
