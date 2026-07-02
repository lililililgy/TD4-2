#pragma once

/// std
#include <memory>
#include <vector>
#include <string>

/// externals
#include <mono/jit/jit.h>

/// engine
#include "Loader/SceneIO.h"

namespace ONEngine {
class EntityComponentSystem;
}

namespace ONEngine::Asset {
class AssetCollection;
}



/// ///////////////////////////////////////////////////
/// シーンの管理を行うクラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

class SceneManager final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	SceneManager(EntityComponentSystem* entityCompnentSystem);
	~SceneManager();

	/// @brief このクラスの初期化
	void Initialize(Asset::AssetCollection* assetCollection);

	/// @brief シーンの更新
	void Update();

	/// @brief 次のシーンを設定する
	/// @param sceneName 次のシーンの名前
	void SetNextScene(const std::string& sceneName);

	/// @brief シーンの保存
	/// @param name シーン名
	/// @param ecsGroup 保存対象のECSGroup
	void SaveScene(const std::string& name, class ECSGroup* ecsGroup);

	/// @brief 現在のシーンを保存する
	void SaveCurrentScene();
	/// @brief 現在のシーンを一時的に保存する
	void SaveCurrentSceneTemporary();
	/// @brief 一時保存されたシーン名をクリアする
	void ClearTemporarySavedSceneName();


	/// @brief シーンを読み込む
	/// @param sceneName シーンの名前
	void LoadScene(const std::string& sceneName);

	/// @brief 現在のシーンをリロードする
	/// @param isTemporary 一時的なシーンかどうか
	void ReloadScene(bool isTemporary);


	/// @brief シーンIOの取得
	SceneIO* GetSceneIO();

	/// @brief 最後に開いたシーン名を取得する
	std::string LastOpenSceneName();


	/// @brief シーンが変更されたかどうか
	void MarkDirty();

	/// @brief シーンが変更されているかどうかを取得
	bool IsDirty() const;

	/// @brief シーンの変更フラグを設定
	void SetDirty(bool isDirty);

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	/// @brief シーンを現在のシーンに移動する
	/// @param isTemporary 一時的なシーンかどうか
	void MoveNextToCurrentScene(bool isTemporary);


private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	/// ----- other class ----- ///
	EntityComponentSystem* pEcs_;
	Asset::AssetCollection* pAssetCollection_;

	std::string currentScene_;
	std::string nextScene_;

	bool isDirty_ = false;

	std::unique_ptr<SceneIO> sceneIO_;
	std::string temporarySavedSceneName_;


public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	/// @brief 現在のシーン名を取得する
	const std::string& GetCurrentSceneName() const;
};



namespace MonoInternalMethods {

void InternalLoadScene(MonoString* sceneName);
}

} /// ONEngine
