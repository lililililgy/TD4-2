#pragma once

/// std
#include <vector>
#include <unordered_map>
#include <string>

/// engine
#include "../../EditorViewCollection.h"
#include "Engine/Core/Utility/Tools/Log.h"


/// ///////////////////////////////////////////////////
/// ImGuiにGameのログを表示するWindow
/// ///////////////////////////////////////////////////
namespace Editor {

class ConsoleWindow : public IEditorWindow {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================
	ConsoleWindow() {}
	~ConsoleWindow() {}

	/// @brief imgui windowの描画処理
	void ShowImGui() override;

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	bool showInfo_ = true;
	bool showWarning_ = true;
	bool showError_ = true;

	bool showEngine_ = true;
	bool showScriptEngine_ = true;
	bool showApplication_ = true;

	bool autoScroll_ = true;
	bool collapse_ = false;

	// キャッシュ制御用
	uint64_t lastUpdateCounter_ = 0;
	bool lastShowInfo_ = true;
	bool lastShowWarning_ = true;
	bool lastShowError_ = true;
	bool lastShowEngine_ = true;
	bool lastShowScriptEngine_ = true;
	bool lastShowApplication_ = true;
	bool lastCollapse_ = false;

	struct CollapsedEntry {
		ONEngine::LogLevel level;
		ONEngine::LogCategory category;
		std::string message;
		int count;
	};

	std::vector<CollapsedEntry> collapsedLogs_;
	std::vector<size_t> displayIndices_;

	void RebuildLogCache();

};

} /// Editor
