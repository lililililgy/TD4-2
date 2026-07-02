#include "ImGuiCommand.h"
#include "Engine/Editor/Math/ImGuiMath.h"

/// externals
#include <imgui.h>

/// engine
#include "Engine/Editor/Manager/EditCommand.h"

using namespace Editor;

bool ImMathf::DragInt(const std::string& label, int* pv, int step, int min, int max) {
	static int startValue{};

	bool edit = ImGui::DragInt(label.c_str(), pv, static_cast<float>(step), min, max);
	if (ImGui::IsItemActive()) LoopCursorIfDragging();
	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pv;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		int endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<int>>(pv, startValue, endValue);
	}

	return edit;
}

bool ImMathf::DragInt2(const std::string& label, ONEngine::Vector2Int* pv, int step, int min, int max) {
	static ONEngine::Vector2Int startValue{};

	bool edit = ImGui::DragInt2(label.c_str(), &pv->x, static_cast<float>(step), min, max);
	if (ImGui::IsItemActive()) LoopCursorIfDragging();
	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pv;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::Vector2Int endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<ONEngine::Vector2Int>>(pv, startValue, endValue);
	}

	return edit;
}

bool ImMathf::DragInt3(const std::string& label, ONEngine::Vector3Int* pv, int step, int min, int max) {
	static ONEngine::Vector3Int startValue{};

	bool edit = ImGui::DragInt3(label.c_str(), &pv->x, static_cast<float>(step), min, max);
	if (ImGui::IsItemActive()) LoopCursorIfDragging();
	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pv;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::Vector3Int endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<ONEngine::Vector3Int>>(pv, startValue, endValue);
	}

	return edit;
}

bool ImMathf::DragFloat(const std::string& label, float* pv, float step, float min, float max, const char* format) {
	static float startValue{};

	bool edit = ImGui::DragFloat(label.c_str(), pv, step, min, max, format);
	if (ImGui::IsItemActive()) LoopCursorIfDragging();
	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pv;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		float endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<float>>(pv, startValue, endValue);
	}

	return edit;
}

bool ImMathf::DragFloat2(const std::string& label, ONEngine::Vector2* pv, float step, float min, float max) {
	static ONEngine::Vector2 startValue{};

	bool edit = ImGui::DragFloat2(label.c_str(), &pv->x, step, min, max);
	if (ImGui::IsItemActive()) LoopCursorIfDragging();
	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pv;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::Vector2 endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<ONEngine::Vector2>>(pv, startValue, endValue);
	}

	return edit;
}


bool ImMathf::DragFloat3(const std::string& label, ONEngine::Vector3* pv, float step, float min, float max) {
	static ONEngine::Vector3 startValue{};

	bool edit = ImGui::DragFloat3(label.c_str(), &pv->x, step, min, max);
	if (ImGui::IsItemActive()) LoopCursorIfDragging();

	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pv;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::Vector3 endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<ONEngine::Vector3>>(pv, startValue, endValue);
	}

	return edit;
}

bool ImMathf::DragFloat4(const std::string& label, ONEngine::Vector4* pv, float step, float min, float max) {
	static ONEngine::Vector4 startValue{};

	bool edit = ImGui::DragFloat4(label.c_str(), &pv->x, step, min, max);
	if (ImGui::IsItemActive()) LoopCursorIfDragging();

	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pv;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::Vector4 endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<ONEngine::Vector4>>(pv, startValue, endValue);
	}

	return edit;
}

bool ImMathf::DragQuaternion(const std::string& label, ONEngine::Quaternion* pq, float step, float min, float max) {
	static ONEngine::Quaternion startValue{};

	/// Eulerに変換して表示
	ONEngine::Vector3 euler = ONEngine::Quaternion::ToEuler(*pq);
	bool edit = ImGui::DragFloat3(label.c_str(), &euler.x, step, min, max);
	if (ImGui::IsItemActive()) LoopCursorIfDragging();
	if (edit) {
		*pq = ONEngine::Quaternion::FromEuler(euler);
	}

	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pq;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::Quaternion endValue = *pq;
		EditCommand::Execute<ModifyValueCommand<ONEngine::Quaternion>>(pq, startValue, endValue);
	}

	return false;
}

bool ImMathf::Checkbox(const std::string& label, bool* pv) {

	static bool startValue{};

	startValue = *pv;
	bool edit = ImGui::Checkbox(label.c_str(), pv);
	if (edit) {
		bool endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<bool>>(pv, startValue, endValue);
	}


	return edit;
}

bool ImMathf::SliderFloat(const std::string& label, float* pv, float min, float max, const char* format) {
	static float startValue{};

	bool edit = ImGui::SliderFloat(label.c_str(), pv, min, max, format);
	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pv;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		float endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<float>>(pv, startValue, endValue);
	}

	return edit;
}

bool ImMathf::SliderFloat3(const std::string& label, ONEngine::Vector3* pv, float min, float max, const char* format) {
	static ONEngine::Vector3 startValue{};

	float values[3] = { pv->x, pv->y, pv->z };
	bool edit = ImGui::SliderFloat3(label.c_str(), values, min, max, format);
	if (edit) {
		pv->x = values[0];
		pv->y = values[1];
		pv->z = values[2];
	}

	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pv;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::Vector3 endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<ONEngine::Vector3>>(pv, startValue, endValue);
	}

	return edit;
}

bool ImMathf::ColorEdit3(const std::string& label, ONEngine::Vector3* pv) {
	static ONEngine::Vector3 startValue{};

	float color[3] = { pv->x, pv->y, pv->z };
	bool edit = ImGui::ColorEdit3(label.c_str(), color);
	if (edit) {
		pv->x = color[0];
		pv->y = color[1];
		pv->z = color[2];
	}

	/// 操作を始めた
	if (ImGui::IsItemActivated()) {
		startValue = *pv;
	}

	/// 操作を終えた
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::Vector3 endValue = *pv;
		EditCommand::Execute<ModifyValueCommand<ONEngine::Vector3>>(pv, startValue, endValue);
	}

	return edit;
}

