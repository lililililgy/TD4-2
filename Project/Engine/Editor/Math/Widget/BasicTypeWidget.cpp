#include "BasicTypeWidget.h"

/// externals
#include <imgui.h>
#include <imgui_internal.h>

/// editor
#include "Engine/Editor/Manager/EditCommand.h"
#include "Engine/Editor/Commands/ImGuiCommand/ImGuiCommand.h"

using namespace Editor;

namespace {

// レイアウト開始
bool BeginPropertyRow(const std::string& label, float columnWidth) {
	ImGui::PushID(label.c_str());
	ImGuiTableFlags tableFlags = ImGuiTableFlags_NoSavedSettings;
	if(ImGui::BeginTable("##PropertyTable", 2, tableFlags)) {
		ImGui::TableSetupColumn("##Label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
		ImGui::TableSetupColumn("##Value", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s", label.c_str());
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		return true;
	}
	return false;
}

// レイアウト終了
void EndPropertyRow() {
	ImGui::EndTable();
	ImGui::PopID();
}

// Undo/Redo処理用ヘルパー
// pVal         : 変数へのポインタ
// preVal       : ImGui関数を呼ぶ前の値
// staticStartVal: 操作開始時の値を保持する静的変数の参照
template<typename T>
void HandleUndo(T* pVal, const T& preVal, T& staticStartVal) {
	// アイテムがアクティブになった瞬間（クリックした瞬間など）
	// ImGuiの関数実行ですでに値が変わっている可能性があるため、
	// 事前に退避しておいた preVal を開始値として保存する
	if(ImGui::IsItemActivated()) {
		staticStartVal = preVal;
	}

	// 編集終了後（マウスリリース、Enter確定など）
	if(ImGui::IsItemDeactivatedAfterEdit()) {
		// 値が実際に変わっていたらコマンド発行
		if(*pVal != staticStartVal) {
			EditCommand::Execute<ModifyValueCommand<T>>(pVal, staticStartVal, *pVal);
		}
	}
}

} /// namespace


// ==================================================================================
// Int Implementation
// ==================================================================================

bool Editor::DragInt(const std::string& label, int& v, float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags, float columnWidth) {
	bool changed = false;
	static int s_startVal = 0; // 操作開始時の値を保持

	if(BeginPropertyRow(label, columnWidth)) {
		int preVal = v; // ImGui呼び出し前の値を保存

		if(ImGui::DragInt("##v", &v, v_speed, v_min, v_max, format, flags)) {
			changed = true;
		}

		// Undo処理
		HandleUndo(&v, preVal, s_startVal);

		EndPropertyRow();
	}
	return changed;
}

bool Editor::SliderInt(const std::string& label, int& v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags, float columnWidth) {
	bool changed = false;
	static int s_startVal = 0;

	if(BeginPropertyRow(label, columnWidth)) {
		int preVal = v;

		if(ImGui::SliderInt("##v", &v, v_min, v_max, format, flags)) {
			changed = true;
		}

		HandleUndo(&v, preVal, s_startVal);

		EndPropertyRow();
	}
	return changed;
}

bool Editor::InputInt(const std::string& label, int& v, int step, int step_fast, ImGuiInputTextFlags flags, float columnWidth) {
	bool changed = false;
	static int s_startVal = 0;

	if(BeginPropertyRow(label, columnWidth)) {
		int preVal = v;

		if(ImGui::InputInt("##v", &v, step, step_fast, flags)) {
			changed = true;
		}

		HandleUndo(&v, preVal, s_startVal);

		EndPropertyRow();
	}
	return changed;
}

// ==================================================================================
// Float Implementation
// ==================================================================================

bool Editor::DragFloat(const std::string& label, float& v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float columnWidth) {
	bool changed = false;
	static float s_startVal = 0.0f;

	if(BeginPropertyRow(label, columnWidth)) {
		float preVal = v;

		if(ImGui::DragFloat("##v", &v, v_speed, v_min, v_max, format, flags)) {
			changed = true;
		}

		HandleUndo(&v, preVal, s_startVal);

		EndPropertyRow();
	}
	return changed;
}

bool Editor::SliderFloat(const std::string& label, float& v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float columnWidth) {
	bool changed = false;
	static float s_startVal = 0.0f;

	if(BeginPropertyRow(label, columnWidth)) {
		float preVal = v;

		if(ImGui::SliderFloat("##v", &v, v_min, v_max, format, flags)) {
			changed = true;
		}

		HandleUndo(&v, preVal, s_startVal);

		EndPropertyRow();
	}
	return changed;
}

bool Editor::InputFloat(const std::string& label, float& v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags, float columnWidth) {
	bool changed = false;
	static float s_startVal = 0.0f;

	if(BeginPropertyRow(label, columnWidth)) {
		float preVal = v;

		if(ImGui::InputFloat("##v", &v, step, step_fast, format, flags)) {
			changed = true;
		}

		HandleUndo(&v, preVal, s_startVal);

		EndPropertyRow();
	}
	return changed;
}