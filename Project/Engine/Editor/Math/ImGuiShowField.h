#pragma once

/// std
#include <string>
#include <unordered_map>
#include <memory>

/// engine
#include "Engine/ECS/Component/Components/ComputeComponents/Variables/Variables.h"

/// external
#include <mono/jit/jit.h>
#include <imgui.h>


namespace ONEngine { class Variables; }

namespace Editor {

/// ///////////////////////////////////////////////////////
/// C#のフィールドをImGuiで表示するための名前空間
/// ///////////////////////////////////////////////////////
namespace CSGui {


/// @brief ImGuiでC#の[SerializeField]のフィールドを表示する
/// @param scriptName スクリプト名
/// @param type 変数の型
/// @param obj 表示するオブジェクト
/// @param field 表示するフィールド
/// @param name ImGuiで表示する変数名
void ShowField(const std::string& scriptName, int type, MonoObject* obj, MonoClassField* field, const char* name);

/// @brief ImGuiでVariablesコンポーネント経由でC#の[SerializeField]のフィールドを表示する
/// @param vars Variablesコンポーネント
/// @param groupName グループ名 (スクリプト名)
/// @param type 変数の型
/// @param field 表示するフィールド
/// @param name ImGuiで表示する変数名
void ShowFieldForVariables(ONEngine::Variables* vars, const std::string& groupName, int type, MonoClassField* field, const char* name);



/// @brief ImGuiでCSのフィールドを表示するための構造体
struct ImGuiShowField {
	virtual ~ImGuiShowField() = default;
	virtual void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) = 0;
};

/// @brief intをImGuiで表示するための構造体
struct IntField : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
};

/// @brief floatをImGuiで表示するための構造体
struct FloatField : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
};

/// @brief doubleをImGuiで表示するための構造体
struct DoubleField : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
};

/// @brief boolをImGuiで表示するための構造体
struct BoolField : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
};

/// @brief stringをImGuiで表示するための構造体
struct StringField : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
};

/// @brief ListをImGuiで表示するための構造体
struct ListField : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
};

/// @brief enumをImGuiで表示するための構造体
struct EnumField : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
};

/// -------------------------------------
/// 自作構造体の表示用
/// -------------------------------------

struct StructGui : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
	void Register();
	std::unordered_map<std::string, std::unique_ptr<ImGuiShowField>> fieldDrawers; ///< フィールドの型ごとに表示用の構造体を保持
	std::unordered_map<std::string, std::shared_ptr<ONEngine::Variables::GenericObject>> startValues; ///< フィールド編集開始時の値を保持
};

/// @brief Vector2をImGuiで表示するための構造体
struct Vector2Field : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
};

/// @brief Vector3をImGuiで表示するための構造体
struct Vector3Field : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
};

/// @brief Vector4をImGuiで表示するための構造体
struct Vector4Field : public ImGuiShowField {
	void Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) override;
};

}

} /// Editor