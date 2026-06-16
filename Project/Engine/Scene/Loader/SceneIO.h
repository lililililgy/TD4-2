#pragma once

/// std
#include <string>

/// external
#include <nlohmann/json.hpp>

/// ///////////////////////////////////////////////////
/// SceneのIOを行うクラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

class SceneIO {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	SceneIO(class EntityComponentSystem* ecs);
	~SceneIO();

	/// 入出力
	void Output(const std::string& sceneName, class ECSGroup* ecsGroup);
	void Input(const std::string& sceneName, class ECSGroup* ecsGroup);

	/// 仮のシーンの入出力
	void OutputTemporary(class ECSGroup* ecsGroup);
	void InputTemporary(class ECSGroup* ecsGroup);

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	/// シーンの入出力
	void SaveScene(const std::string& filename, class ECSGroup* ecsGroup);
	void LoadScene(const std::string& filename, class ECSGroup* ecsGroup);

	/// ECSGroupをJsonに変換する
	void SaveSceneToJson(nlohmann::json& output, class ECSGroup* ecsGroup);
	void LoadSceneFromJson(const nlohmann::json& input, class ECSGroup* ecsGroup);

	/// Jsonの入出力
	void OutputJson(const nlohmann::json& json, const std::string& filename);

private:
	/// ==================================================
	/// private : objects
	/// ==================================================

	/// ----- other class ----- ///
	class EntityComponentSystem* pEcs_; 

	std::string fileName_; // ioに使うファイル名
	std::string fileDirectory_;

	/// 一時的なシーンの保存に使う、ファイルとして保存はしない
	nlohmann::json tempSceneJson_;

};


} /// ONEngine
