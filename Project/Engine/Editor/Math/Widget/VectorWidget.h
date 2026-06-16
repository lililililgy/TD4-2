#pragma once

/// engine
#include "Engine/Core/Utility/Math/Vector2.h"
#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Vector4.h"
#include <string>

namespace Editor {

// ==================================================================================
// Floating Point Vectors
// ==================================================================================

/// @brief ImGuiを用いて Vector2 (float) を編集するGUIウィジェットを描画します。
/// @param min 最小値 (0かつmaxも0の場合は制限なし)
/// @param max 最大値 (0かつminも0の場合は制限なし)
bool DrawVec2Control(const std::string& label, ONEngine::Vector2& values, float speed = 1.0f, float min = 0.0f, float max = 0.0f, float columnWidth = 100.0f, bool* unified = nullptr, bool useUndo = true, bool* outActivated = nullptr, bool* outDeactivated = nullptr);

bool DrawVec3Control(const std::string& label, ONEngine::Vector3& values, float speed = 1.0f, float min = 0.0f, float max = 0.0f, float columnWidth = 100.0f, bool* unified = nullptr, bool useUndo = true, bool* outActivated = nullptr, bool* outDeactivated = nullptr);

bool DrawVec4Control(const std::string& label, ONEngine::Vector4& values, float speed = 1.0f, float min = 0.0f, float max = 0.0f, float columnWidth = 100.0f, bool* unified = nullptr, bool useUndo = true, bool* outActivated = nullptr, bool* outDeactivated = nullptr);

// ==================================================================================
// Integer Vectors
// ==================================================================================

/// @brief ImGuiを用いて Vector2Int (int) を編集するGUIウィジェットを描画します。
/// @param min 最小値 (0かつmaxも0の場合は制限なし)
/// @param max 最大値 (0かつminも0の場合は制限なし)
bool DrawVec2IntControl(const std::string& label, ONEngine::Vector2Int& values, float speed = 1.0f, int min = 0, int max = 0, float columnWidth = 100.0f, bool useUndo = true, bool* outActivated = nullptr, bool* outDeactivated = nullptr);

bool DrawVec3IntControl(const std::string& label, ONEngine::Vector3Int& values, float speed = 1.0f, int min = 0, int max = 0, float columnWidth = 100.0f, bool useUndo = true, bool* outActivated = nullptr, bool* outDeactivated = nullptr);

bool DrawVec4IntControl(const std::string& label, ONEngine::Vector4Int& values, float speed = 1.0f, int min = 0, int max = 0, float columnWidth = 100.0f, bool useUndo = true, bool* outActivated = nullptr, bool* outDeactivated = nullptr);

} /// namespace Editor