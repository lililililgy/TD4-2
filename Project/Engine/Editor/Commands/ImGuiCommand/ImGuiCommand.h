#pragma once

/// std
#include <string>

/// engine
#include "Engine/Core/Utility/Utility.h"
#include "../IEditCommand.h"

namespace Editor {

/// ///////////////////////////////////////////////////
/// ImGui関連の編集
/// ///////////////////////////////////////////////////
namespace ImMathf {

	/// @brief ImGuiのDragIntでintを操作するコマンド
	/// @param label DragIntのラベル
	/// @param pv intのポインタ
	/// @param step 1回の操作で変化する値
	/// @param min pvの最小値
	/// @param max pvの最大値
	/// @return true: 値が変更された, false: 値が変更されなかった
	bool DragInt(const std::string& label, int* pv, int step = 1, int min = 0, int max = 0);

	/// @brief ImGuiのDragInt2でVector2Intを操作するコマンド
	/// @param label DragInt2のラベル
	/// @param pv Vector2Intのポインタ
	/// @param step 1回の操作で変化する値
	/// @param min pvの最小値
	/// @param max pvの最大値
	/// @return true: 値が変更された, false: 値が変更されなかった
	bool DragInt2(const std::string& label, ONEngine::Vector2Int* pv, int step = 1, int min = 0, int max = 0);

	/// @brief ImGuiのDragInt3でVector3Intを操作するコマンド
	/// @param label DragInt3のラベル
	/// @param pv Vector3Intのポインタ
	/// @param step 1回の操作で変化する値
	/// @param min pvの最小値
	/// @param max pvの最大値
	/// @return true: 値が変更された, false: 値が変更されなかった
	bool DragInt3(const std::string& label, ONEngine::Vector3Int* pv, int step = 1, int min = 0, int max = 0);

	/// @brief ImGuiのDragFloatでfloatを操作するコマンド
	/// @param label DragFloatのラベル
	/// @param pv floatのポインタ
	/// @param step 1回の操作で変化する値
	/// @param min pvの最小値
	/// @param max pvの最大値
	/// @return true: 値が変更された, false: 値が変更されなかった
	bool DragFloat(const std::string& label, float* pv, float step = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.3f");

	/// @brief ImGuiのDragFloat2でVector2を操作するコマンド
	/// @param label DragFloat2のラベル
	/// @param pv floatのポインタ
	/// @param step 1回の操作で変化する値
	/// @param min pvの最小値
	/// @param max pvの最大値
	/// @return true: 値が変更された, false: 値が変更されなかった
	bool DragFloat2(const std::string& label, ONEngine::Vector2* pv, float step = 1.0f, float min = 0.0f, float max = 0.0f);

	/// @brief ImGuiのDragFloat3でVector3を操作するコマンド
	/// @param label DragFloat3のラベル
	/// @param pv Vector3のポインタ
	/// @param step 1回の操作で変化する値
	/// @param min pvの最小値
	/// @param max pvの最大値
	/// @return true: 値が変更された, false: 値が変更されなかった
	bool DragFloat3(const std::string& label, ONEngine::Vector3* pv, float step = 1.0f, float min = 0.0f, float max = 0.0f);

	/// @brief 4つの浮動小数点値をドラッグ操作で編集できるUIウィジェットを表示します。
	/// @param label ウィジェットに表示するラベル文字列。
	/// @param pv 編集対象となる4要素のベクトル（Vector4型）へのポインタ。
	/// @param step ドラッグ時の増減ステップ値（デフォルトは1.0f）。
	/// @param min 値の最小制限（デフォルトは0.0f、0の場合は制限なし）。
	/// @param max 値の最大制限（デフォルトは0.0f、0の場合は制限なし）。
	/// @return 値が変更された場合はtrue、変更されなかった場合はfalse。
	bool DragFloat4(const std::string& label, ONEngine::Vector4* pv, float step = 1.0f, float min = 0.0f, float max = 0.0f);

	/// @brief クォータニオンの値をEulerに変換しドラッグ操作で調整します。
	/// @param label UI上で表示するラベル文字列。
	/// @param pq 調整対象となるクォータニオンへのポインタ。
	/// @param step ドラッグ時の増減ステップ値（デフォルトは0.1f）。
	/// @param min 調整可能な最小値（デフォルトは0.0f、制限なし）。
	/// @param max 調整可能な最大値（デフォルトは0.0f、制限なし）。
	/// @return 値が変更された場合はtrue、変更されなかった場合はfalse。
	bool DragQuaternion(const std::string& label, ONEngine::Quaternion* pq, float step = 0.1f, float min = 0.0f, float max = 0.0f);


	/// @brief フラグを切り替えるチェックボックスを表示します。
	/// @param label UI上で表示するラベル文字列。
	/// @param pv 調整対象となるブール値へのポインタ。
	/// @return true: 値が変更された, false: 値が変更されなかった
	bool Checkbox(const std::string& label, bool* pv);

	/// @brief スライダーでfloat値を調整します。
	bool SliderFloat(const std::string& label, float* pv, float min, float max, const char* format = "%.3f");

	/// @brief スライダーでVector3値を調整します。
	bool SliderFloat3(const std::string& label, ONEngine::Vector3* pv, float min, float max, const char* format = "%.3f");

	/// @brief カラーエディタでVector3値を調整します。
	bool ColorEdit3(const std::string& label, ONEngine::Vector3* pv);

}



/// @brief T型の値を変更するコマンド
/// @tparam T 変更する値の型 (T型のoperator=が定義されている必要がある)
template <typename T>
class ModifyValueCommand : public IEditCommand {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	ModifyValueCommand(T* v, const T& old, const T& newVal)
		: pValue_(v), oldValue_(old), newValue_(newVal) {
	}
	~ModifyValueCommand() = default;
	EDITOR_STATE Execute() {
		if (pValue_) {
			*pValue_ = newValue_;
		} else {
			ONEngine::Console::LogError("ImGuiCommand::ModifyValueCommand : Value is nullptr");
			return EDITOR_STATE::EDITOR_STATE_FAILED;
		}
		return EDITOR_STATE::EDITOR_STATE_FINISH;
	}
	EDITOR_STATE Undo() {
		if (pValue_) {
			*pValue_ = oldValue_;
		} else {
			ONEngine::Console::LogError("ImGuiCommand::ModifyValueCommand : Value is nullptr");
			return EDITOR_STATE::EDITOR_STATE_FAILED;
		}
		return EDITOR_STATE::EDITOR_STATE_FINISH;
	}
private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	T* pValue_;
	T oldValue_, newValue_;
};

} /// Editor
