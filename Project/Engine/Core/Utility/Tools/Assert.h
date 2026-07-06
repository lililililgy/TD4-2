#pragma once

/// window
#include <windows.h> 

/// std
#include <string>
#include <source_location>
#include <filesystem>

/// engine
#include "Log.h"
#include "Engine/Core/Config/EngineConfig.h"
#include <fstream>
#include <nlohmann/json.hpp>


namespace ONEngine {

/// <summary>
/// conditionがfalseの場合、エラーメッセージを表示してデバッグを停止する
/// </summary>
/// <param name="condition">    : 止めるかどうかの条件、falseの場合止まる </param>
/// <param name="errorMessage"> : メッセージボックスに表示するテキスト     </param>
inline void Assert(bool condition, const char* errorMessage, const std::source_location& location = std::source_location::current()) {
	if (!condition) {

		/// ファイルパスを取得
		std::filesystem::path filePath(location.file_name());

		/// エラーメッセージを作成
		std::string errorMsg = "ONEngine Assertion failed:\n";
		errorMsg += errorMessage;
		errorMsg += "\n\nLocation:\n";
		errorMsg += "File: ";
		errorMsg += filePath.filename().string();
		errorMsg += "\nFunction: ";
		errorMsg += location.function_name();
		errorMsg += "\nLine: ";
		errorMsg += std::to_string(location.line());

		if (EngineConfig::isTestMode) {
			nlohmann::json results;
			results["success"] = false;
			results["message"] = errorMsg;
			std::ofstream ofs(EngineConfig::testOutputPath);
			if (ofs.is_open()) {
				ofs << results.dump(4);
				ofs.close();
			}
			exit(1);
		} else {
			/// ポップアップウィンドウを表示
			MessageBoxA(nullptr, errorMsg.c_str(), "ONEngine Assertion", MB_OK | MB_ICONERROR);
			Console::Log("[ASSERTION ERROR] " + errorMsg); // Log the last part if any

			Console::Shutdown();
			__debugbreak();
		}
	}
}


/// <summary>
/// conditionがfalseの場合、エラーメッセージを表示してデバッグを停止する
/// </summary>
/// <param name="condition">    : 止めるかどうかの条件、falseの場合止まる </param>
inline void Assert(bool condition, const std::source_location& location = std::source_location::current()) {
	Assert(condition, "Assertion failed", location); ///< デフォルトのエラーメッセージを使用
}

} /// namespace ONEngine