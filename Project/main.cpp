#include <filesystem>
#include <windows.h>

/// engine
#include "Engine/Core/GameFramework/GameFramework.h"
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/Core/Config/EngineConfig.h"

void SetCorrectCurrentDirectory() {
	if (std::filesystem::exists("./Assets") && std::filesystem::exists("./Packages")) {
		return;
	}

	wchar_t exePath[MAX_PATH];
	if (GetModuleFileNameW(NULL, exePath, MAX_PATH) > 0) {
		std::filesystem::path currentDir = std::filesystem::path(exePath).parent_path();
		while (currentDir.has_relative_path()) {
			if (std::filesystem::exists(currentDir / "Assets") && std::filesystem::exists(currentDir / "Packages")) {
				std::filesystem::current_path(currentDir);
				OutputDebugStringW((L"[ONEngine] Current directory automatically adjusted to: " + currentDir.wstring() + L"\n").c_str());
				break;
			}
			currentDir = currentDir.parent_path();
		}
	}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	SetCorrectCurrentDirectory();

	ONEngine::Console console;

	/// コマンドライン引数をパース
	ONEngine::EngineConfig::ParseCommandLine();

	/// 設定ファイルをロード
	ONEngine::EngineConfig::LoadConfig();

	std::string titleStr = ONEngine::EngineConfig::windowTitle;
	std::wstring titleWStr = ONEngine::ConvertString(titleStr);

	std::unique_ptr<ONEngine::GameFramework> gameFramework = std::make_unique<ONEngine::GameFramework>();
	gameFramework->Initialize(ONEngine::GameFrameworkConfig{
		.windowName = titleWStr,
		.windowSize = ONEngine::Vector2(
			static_cast<float>(ONEngine::EngineConfig::windowWidth),
			static_cast<float>(ONEngine::EngineConfig::windowHeight)
		),
	});

	gameFramework->Run();

	return 0;
}