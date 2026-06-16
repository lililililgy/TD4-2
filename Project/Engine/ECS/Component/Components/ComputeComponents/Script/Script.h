#pragma once

/// std
#include <string>
#include <vector>
#include <array>
#include <unordered_map>

/// externals
#include <nlohmann/json.hpp>
#include <jit/jit.h>

/// engine
#include "Engine/ECS/Component/Components/Interface/IComponent.h"


/// ///////////////////////////////////////////////////
/// スクリプトコンポーネント
/// ///////////////////////////////////////////////////
namespace ONEngine {

class Script : public IComponent {
	friend class MonoScriptEngine;
	friend class ScriptUpdateSystem;
public:

	struct ScriptData {
		std::string scriptName;
		bool isAdded = false; ///< スクリプトが追加されたかどうか
		std::array<MonoMethod*, 3> collisionEventMethods = {};

		bool enable;
		bool GetEnable(GameEntity* entity);
		void SetEnable(GameEntity* entity, bool enable);
	};

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	Script();
	~Script() override;


	/// @brief スクリプトを持っているかチェックする
	/// @param scriptName チェックする対象のスクリプト名
	/// @return true: 持っている false: 持っていない
	bool Contains(const std::string& scriptName) const;

	/// @brief 指定されたスクリプト名を追加します。
	/// @param scriptName 追加するスクリプトの名前
	void AddScript(const std::string& scriptName);

	/// @brief 引数のスクリプトを削除する
	/// @param scriptName 削除するスクリプトの名前
	void RemoveScript(const std::string& scriptName);


	/// @brief スクリプトの名前を引数Indexから取得する
	/// @param index Script配列のIndex
	/// @return スクリプトの名前
	const std::string& GetScriptName(size_t index) const;

	/// @brief スクリプトの名前配列を得る
	std::vector<std::string> GetScriptNames() const;


	/// @brief スクリプトのデータのリストを取得する
	const std::vector<ScriptData>& GetScriptDataList() const;
	std::vector<ScriptData>& GetScriptDataList();
	ScriptData* GetScriptData(const std::string& scriptName);


	/// @brief スクリプトの有効/無効を設定する
	/// @param scriptName スクリプト名
	/// @param enable 設定する有効/無効
	void SetEnable(const std::string& scriptName, bool enable);

	/// @brief スクリプトの有効/無効を取得する
	/// @param scriptName スクリプト名
	/// @return true: 有効 false: 無効
	bool GetEnable(const std::string& scriptName);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::unordered_map< std::string, size_t> scriptIndexMap_;
	std::vector<ScriptData> scriptDataList_;
	bool isAdded_;


public:
	/// ===================================================
	/// public : accessors
	/// ===================================================

	/// エンティティが追加されたか
	void SetIsAdded(bool added);
	bool GetIsAdded() const;

};

namespace ComponentDebug {
	void ScriptDebug(Script* script);
}



/// ///////////////////////////////////////////////////
/// json用の関数
/// ///////////////////////////////////////////////////
void from_json(const nlohmann::json& j, Script& s);
void to_json(nlohmann::json& j, const Script& s);

} /// ONEngine
