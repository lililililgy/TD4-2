#include "ConsoleWindow.h"

/// std
#include <format>
#include <vector>

/// external
#include <imgui.h>

/// engine
#include "Engine/Core/Utility/Time/Time.h"
#include "Engine/Core/Utility/Tools/Log.h"

using namespace Editor;

void ConsoleWindow::ShowImGui() {
	if (!ImGui::Begin("Console")) {
		ImGui::End();
		return;
	}

	// --- ツールバーの描画 ---
	if (ImGui::Button("Clear")) {
		ONEngine::Console::ClearLogBuffer();
	}
	ImGui::SameLine();
	ImGui::Checkbox("Auto-scroll", &autoScroll_);
	
	ImGui::SameLine();
	ImGui::Checkbox("Collapse", &collapse_);
	
	ImGui::SameLine();
	ImGui::TextUnformatted("|");
	
	// レベルフィルタ
	ImGui::SameLine();
	ImGui::Checkbox("Info", &showInfo_);
	ImGui::SameLine();
	ImGui::Checkbox("Warning", &showWarning_);
	ImGui::SameLine();
	ImGui::Checkbox("Error", &showError_);

	ImGui::SameLine();
	ImGui::TextUnformatted("|");

	// カテゴリフィルタ
	ImGui::SameLine();
	ImGui::Checkbox("Engine", &showEngine_);
	ImGui::SameLine();
	ImGui::Checkbox("Script", &showScriptEngine_);
	ImGui::SameLine();
	ImGui::Checkbox("Game", &showApplication_);

	ImGui::Separator();

	// キャッシュの再構築が必要か判定
	bool filtersChanged = (
		showInfo_ != lastShowInfo_ ||
		showWarning_ != lastShowWarning_ ||
		showError_ != lastShowError_ ||
		showEngine_ != lastShowEngine_ ||
		showScriptEngine_ != lastShowScriptEngine_ ||
		showApplication_ != lastShowApplication_ ||
		collapse_ != lastCollapse_
	);

	uint64_t currentCounter = ONEngine::Console::GetUpdateCounter();

	if (currentCounter != lastUpdateCounter_ || filtersChanged) {
		lastUpdateCounter_ = currentCounter;
		lastShowInfo_ = showInfo_;
		lastShowWarning_ = showWarning_;
		lastShowError_ = showError_;
		lastShowEngine_ = showEngine_;
		lastShowScriptEngine_ = showScriptEngine_;
		lastShowApplication_ = showApplication_;
		lastCollapse_ = collapse_;

		RebuildLogCache();
	}

	// --- ログ表示領域 ---
	const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar);

	const auto& allLogs = ONEngine::Console::GetLogVector();
	int itemCount = collapse_ ? static_cast<int>(collapsedLogs_.size()) : static_cast<int>(displayIndices_.size());

	ImGuiListClipper clipper;
	clipper.Begin(itemCount);
	while (clipper.Step()) {
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
			ONEngine::LogLevel level;
			ONEngine::LogCategory category;
			const std::string* message = nullptr;
			int count = 1;

			if (collapse_) {
				level = collapsedLogs_[i].level;
				category = collapsedLogs_[i].category;
				message = &collapsedLogs_[i].message;
				count = collapsedLogs_[i].count;
			} else {
				const auto& entry = allLogs[displayIndices_[i]];
				level = entry.level;
				category = entry.category;
				message = &entry.message;
			}

			// カテゴリプレフィックスと色
			const char* categoryTag = "[Unknown] ";
			if (category == ONEngine::LogCategory::Engine) categoryTag = "[Engine] ";
			else if (category == ONEngine::LogCategory::ScriptEngine) categoryTag = "[Script] ";
			else if (category == ONEngine::LogCategory::Application) categoryTag = "[Game]   ";
			
			// レベルに応じた色分け
			ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default White
			if (level == ONEngine::LogLevel::Warning) color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
			if (level == ONEngine::LogLevel::Error) color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);   // Light Red

			ImGui::PushStyleColor(ImGuiCol_Text, color);
			
			if (collapse_ && count > 1) {
				ImGui::Text("%s(%d) %s", categoryTag, count, message->c_str());
			} else {
				ImGui::Text("%s%s", categoryTag, message->c_str());
			}
			
			ImGui::PopStyleColor();
		}
	}

	if (autoScroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::EndChild();

	ImGui::Separator();

	// --- 下部のステータス表示 ---
	std::string&& stats = std::format("fps: {:.2f} | delta: {:.4f}s", 1.0f / ONEngine::Time::DeltaTime(), ONEngine::Time::DeltaTime());
	ImGui::Text(stats.c_str());

	ImGui::End();
}

void ConsoleWindow::RebuildLogCache() {
	const auto& allLogs = ONEngine::Console::GetLogVector();
	collapsedLogs_.clear();
	displayIndices_.clear();

	if (collapse_) {
		std::unordered_map<std::string, size_t> keyToCollapsedIndex;
		keyToCollapsedIndex.reserve(allLogs.size());
		collapsedLogs_.reserve(allLogs.size());

		for (size_t i = 0; i < allLogs.size(); ++i) {
			const auto& entry = allLogs[i];

			// レベルフィルタリング
			if (entry.level == ONEngine::LogLevel::Info && !showInfo_) continue;
			if (entry.level == ONEngine::LogLevel::Warning && !showWarning_) continue;
			if (entry.level == ONEngine::LogLevel::Error && !showError_) continue;

			// カテゴリフィルタリング
			if (entry.category == ONEngine::LogCategory::Engine && !showEngine_) continue;
			if (entry.category == ONEngine::LogCategory::ScriptEngine && !showScriptEngine_) continue;
			if (entry.category == ONEngine::LogCategory::Application && !showApplication_) continue;

			std::string key = std::format("{}_{}_{}", (int)entry.level, (int)entry.category, entry.message);
			auto it = keyToCollapsedIndex.find(key);
			if (it != keyToCollapsedIndex.end()) {
				collapsedLogs_[it->second].count++;
			} else {
				keyToCollapsedIndex[key] = collapsedLogs_.size();
				collapsedLogs_.push_back({ entry.level, entry.category, entry.message, 1 });
			}
		}
	} else {
		displayIndices_.reserve(allLogs.size());
		for (size_t i = 0; i < allLogs.size(); ++i) {
			const auto& entry = allLogs[i];

			// レベルフィルタリング
			if (entry.level == ONEngine::LogLevel::Info && !showInfo_) continue;
			if (entry.level == ONEngine::LogLevel::Warning && !showWarning_) continue;
			if (entry.level == ONEngine::LogLevel::Error && !showError_) continue;

			// カテゴリフィルタリング
			if (entry.category == ONEngine::LogCategory::Engine && !showEngine_) continue;
			if (entry.category == ONEngine::LogCategory::ScriptEngine && !showScriptEngine_) continue;
			if (entry.category == ONEngine::LogCategory::Application && !showApplication_) continue;

			displayIndices_.push_back(i);
		}
	}
}
