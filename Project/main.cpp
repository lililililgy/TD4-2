#include <filesystem>
#include <vector>
#include <windows.h>

/// engine
#include "Engine/Core/GameFramework/GameFramework.h"
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/Core/Config/EngineConfig.h"

void SetCorrectCurrentDirectory() {
	try {
		std::error_code ec1, ec2;
		if (std::filesystem::exists("./Assets", ec1) && std::filesystem::exists("./Packages", ec2)) {
			return;
		}

		std::vector<wchar_t> exePath(MAX_PATH);
		DWORD size = 0;
		while (true) {
			size = GetModuleFileNameW(NULL, exePath.data(), static_cast<DWORD>(exePath.size()));
			if (size == 0) {
				return;
			}
			if (size < exePath.size()) {
				break;
			}
			if (exePath.size() >= 32768) {
				break;
			}
			exePath.resize(exePath.size() * 2);
		}

		std::filesystem::path currentDir = std::filesystem::path(exePath.data()).parent_path();
		std::filesystem::path lastDir;

		while (!currentDir.empty() && currentDir != lastDir) {
			std::error_code e1, e2;
			if (std::filesystem::exists(currentDir / "Assets", e1) && std::filesystem::exists(currentDir / "Packages", e2)) {
				std::error_code ecSet;
				std::filesystem::current_path(currentDir, ecSet);
				SetCurrentDirectoryW(currentDir.c_str());
				OutputDebugStringW((L"[ONEngine] Current directory automatically adjusted to: " + currentDir.wstring() + L"\n").c_str());
				break;
			}
			lastDir = currentDir;
			currentDir = currentDir.parent_path();
		}
	} catch (...) {
	}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	try {
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
	} catch (const std::exception& e) {
		MessageBoxA(NULL, e.what(), "Startup Error", MB_ICONERROR | MB_OK);
	} catch (...) {
		MessageBoxA(NULL, "An unknown error occurred during startup.", "Startup Error", MB_ICONERROR | MB_OK);
	}

	return 0;
}