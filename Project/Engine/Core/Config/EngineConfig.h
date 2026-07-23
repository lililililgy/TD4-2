#pragma once

#include "Engine/Core/Utility/Math/Vector2.h"
#include <string>

namespace ONEngine {

/// /////////////////////////////////////////////////
/// Engineの共通設定
/// /////////////////////////////////////////////////
namespace EngineConfig {
static const Vector2 kWindowSize = { 1920.0f, 1080.0f };

extern std::string startScene;
extern std::string windowTitle;
extern int windowWidth;
extern int windowHeight;
extern bool isFullscreen;
extern bool enableVSync;
extern bool ignoreCSharpLog;
extern bool enableReleaseLogFile;

// Test Mode settings
extern bool isTestMode;
extern std::string testScene;
extern std::string testInputPath;
extern int testDuration;
extern std::string testOutputPath;
extern bool waitDebug;

void LoadConfig();
void SaveConfig();
void ParseCommandLine();
}

/// /////////////////////////////////////////////////
/// デバッグ時の共通設定
/// /////////////////////////////////////////////////
namespace DebugConfig {

static const Vector2 kDebugWindowSize = Vector2(16.0f, 9.0f) * 110.0f; // 120がmax

/// @brief デバッグ中かどうか
extern bool isDebugging;
extern bool isPause;
extern bool isShowDebugScene;
extern bool isShowGameScene;

//enum SELECTED_MODE {
//	SELECTED_MODE_De,
//	SELECTED_MODE_GAME,
//	SELECTED_MODE_EDITOR,
//};
extern int selectedMode_; ///< 選択中のデバッグウィンドウ

enum class SelectedTab {
	Develop,
	Game,
	Prefab,
	Editor,
};

}

} /// ONEngine