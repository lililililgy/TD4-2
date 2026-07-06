/// engine
#include "Engine/Core/GameFramework/GameFramework.h"
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/Core/Config/EngineConfig.h"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	ONEngine::Console console;

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