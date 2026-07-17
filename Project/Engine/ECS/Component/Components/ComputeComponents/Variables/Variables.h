#pragma once

/// std
#include <variant>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/// external
#include <Externals/nlohmann/json.hpp>

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Core/Utility/Math/Vector4.h"
#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Vector2.h"
#include "Engine/Core/Utility/Tools/Log.h"

namespace ONEngine {
class Variables;

/// Json変換
void from_json(const nlohmann::json& j, class Variables& g);
void to_json(nlohmann::json& j, const class Variables& g);


/// ///////////////////////////////////////////////////
/// 変数component
/// ///////////////////////////////////////////////////
class Variables : public IComponent {
	friend void from_json(const nlohmann::json& j, Variables& v);
	friend void to_json(nlohmann::json& j, const Variables& v);
public:
	/// ================================================
	/// public : sub class
	/// ================================================

	enum class VarType {
		kInt,
		kFloat,
		kBool,
		kString,
		kVector2,
		kVector3,
		kVector4,
		kIntList,
		kFloatList,
		kBoolList,
		kStringList,
		kVector3List,
		kObject,
		kObjectList,
		Unknown
	};

	struct GenericObject;

	using Var = std::variant<
		int, float, bool, std::string, Vector2, Vector3, Vector4,
		std::vector<int>, std::vector<float>, std::vector<bool>, std::vector<std::string>, std::vector<Vector3>,
		std::shared_ptr<GenericObject>, std::vector<std::shared_ptr<GenericObject>>
	>;

	struct GenericObject {
		std::string typeName;
		std::map<std::string, Var> fields;

		void Add(const std::string& name, const Var& value) {
			fields[name] = value;
		}

		bool Has(const std::string& name) const {
			return fields.contains(name);
		}
	};

	static Var MonoObjectToVar(void* obj, void* type);
	static void* GetListElementType(void* listClass);
	static std::shared_ptr<GenericObject> MonoObjectToGeneric(void* obj);
	static void VarToMonoObject(void* obj, void* klass, const Var& var);
	static std::shared_ptr<GenericObject> CloneGenericObject(const std::shared_ptr<GenericObject>& src);
	static bool IsEqualGenericObject(const std::shared_ptr<GenericObject>& a, const std::shared_ptr<GenericObject>& b);
	static Var CloneVar(const Var& src);
	static bool IsEqualVar(const Var& a, const Var& b);


	/// @brief 変数のグループ、スクリプトごとに使用する予定
	struct Group {
		std::string name; ///< グループ名
		std::map<std::string, size_t> keyMap;
		std::vector<Var> variables; ///< グループに属する変数

		template <typename T = Var>
		void Add(const std::string& name, const T& value) {
			if (keyMap.contains(name)) {
				variables[keyMap[name]] = value;
				return;
			}

			keyMap[name] = variables.size();
			variables.emplace_back(value);
		}

		template <typename T>
		T& Get(const std::string& name) {
			return std::get<T>(variables[keyMap.at(name)]);
		}

		template <typename T>
		const T& Get(const std::string& name) const {
			return std::get<T>(variables[keyMap.at(name)]);
		}

		const Var& Get(const std::string& varName) const;
		bool Has(const std::string& varName) const;


	};


public:
	/// ================================================
	/// public : methods
	/// ================================================


	Variables();
	~Variables() override;


	/// @brief jsonを読み込んで変数を設定する
	/// @param path 読み込むjsonファイルのパス
	void LoadJson(const std::string& path);

	/// @brief 変数をjsonに保存する
	/// @param path 保存するjsonファイルのパス
	void SaveJson(const std::string& path);


	/// @brief スクリプト内の変数を登録する
	void RegisterScriptVariables();

	/// @brief スクリプト内の変数を再読み込みする
	void ReloadScriptVariables();

	/// @brief スクリプトに変数の値を設定する
	/// @param scriptName 対象のスクリプト名
	void SetScriptVariables(const std::string& scriptName);


	/// @brief 変数のグループ(スクリプト単位)を追加する
	/// @param name グループ名
	/// @return GroupのIndex
	size_t AddGroup(const std::string& name);

	/// @brief 指定した名前に対応するグループを取得する
	/// @param name 取得するグループの名前。
	/// @return 指定された名前に対応するGroup
	const Group& GetGroup(const std::string& name) const;

	/// @brief 引数のGroupを持っているか
	/// @param name Group名
	/// @return true: 持っている false: 持っていない
	bool HasGroup(const std::string& name) const;


	/// @brief グループのキーのマップを取得する
	const std::unordered_map<std::string, size_t>& GetGroupKeyMap() const;

	/// @brief グループの配列をすべて得る
	const std::vector<Group>& GetGroups() const;

	/// @brief 変数を設定する（既存なら上書き、なければ追加）
	/// @param groupName グループ名
	/// @param varName 変数名
	/// @param value 値
	void SetVariable(const std::string& groupName, const std::string& varName, const Var& value);

public:
	static void RegisterSerializeField(const std::string& className, const std::string& fieldName);
	static bool IsSerializeFieldRegistered(const std::string& className, const std::string& fieldName);

private:
	static std::unordered_set<std::string> serializeFieldCache_;

private:
	/// ================================================
	/// private : objects
	/// ================================================

	std::unordered_map<std::string, size_t> groupKeyMap_; /// グループ名とインデックスのマップ
	std::vector<Group> groups_; /// 変数のグループ
};


/// ==================================================
/// public : methods
/// ==================================================


/// @brief ComponentのDebug
namespace ComponentDebug {
	/// @brief Variableをデバッグする
	/// @param variables 対象のポインタ
	void VariablesDebug(Variables* variables);
}

} /// ONEngine
