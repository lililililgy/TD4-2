#pragma once

/// std
#include <string>

/// external
#include <imgui.h>

namespace Editor {
// ==================================================================================
// Int Wrappers
// ==================================================================================

/// @brief ImGui::DragInt のラッパー
bool DragInt(const std::string& label, int& v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0, float columnWidth = 100.0f);

/// @brief ImGui::SliderInt のラッパー
bool SliderInt(const std::string& label, int& v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0, float columnWidth = 100.0f);

/// @brief ImGui::InputInt のラッパー
bool InputInt(const std::string& label, int& v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0, float columnWidth = 100.0f);

// ==================================================================================
// Float Wrappers
// ==================================================================================

/// @brief ImGui::DragFloat のラッパー
bool DragFloat(const std::string& label, float& v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0, float columnWidth = 100.0f);

/// @brief ImGui::SliderFloat のラッパー
bool SliderFloat(const std::string& label, float& v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0, float columnWidth = 100.0f);

/// @brief ImGui::InputFloat のラッパー
bool InputFloat(const std::string& label, float& v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0, float columnWidth = 100.0f);

} /// namespace Editor