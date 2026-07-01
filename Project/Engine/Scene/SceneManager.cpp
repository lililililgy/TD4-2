#include "SceneManager.h"

using namespace ONEngine;

/// std
#include <numbers>
#include <fstream>
#include <filesystem>

/// external
#include <nlohmann/json.hpp>

/// engine
//#include "Scene/Factory/SceneFactory.h"
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"


namespace {
	/// @brief monoに登録する関数で使用するために
	SceneManager* gSceneManager = nullptr;
}

SceneManager::SceneManager(EntityComponentSystem* entityComponentSystem_)
	: pEcs_(entityComponentSystem_) {
}
SceneManager::~SceneManager() {
	/// 最後に開いていたシーンを保存
	std::string sceneToSave = currentScene_;
	if (!temporarySavedSceneName_.empty()) {
		sceneToSave = temporarySavedSceneName_;
	}
	if (!sceneToSave.empty()) {
		nlohmann::json json;
		json["Scene"] = sceneToSave;
		const std::string& filepath = "./Packages/Config/LastOpenScene.json";

		std::filesystem::path path(filepath);
		std::filesystem::path parentDir = path.parent_path();
		if (!parentDir.empty() && !std::filesystem::exists(parentDir)) {
			std::filesystem::create_directories(parentDir);
		}

		std::ofstream ofs(filepath);
		if (ofs.is_open()) {
			ofs << json.dump(4);
			ofs.close();
		}
	}
}


void SceneManager::Initialize(Asset::AssetCollection* assetCollection) {
	gSceneManager = this;

	pAssetCollection_ = assetCollection;

	sceneIO_ = std::make_unique<SceneIO>(pEcs_);

#ifdef DEBUG_MODE
	SetNextScene(LastOpenSceneName());
#else
	SetNextScene("TitleScene");
#endif

	MoveNextToCurrentScene(false);

	pEcs_->MainCameraSetting();
}

void SceneManager::Update() {
	/// 次のシーンが設定されていたらシーンを切り替える
	if (nextScene_.size()) {
		MoveNextToCurrentScene(false);
	}
}

void SceneManager::SetNextScene(const std::string& sceneName) {
	nextScene_ = sceneName;
}

void SceneManager::SaveScene(const std::string& name, ECSGroup* ecsGroup) {
	if (name.empty() || !ecsGroup) {
		Console::LogError("Invalid scene name or ECS group.");
		return;
	}

	sceneIO_->Output(name, ecsGroup);
	SetDirty(false);
}

void SceneManager::SaveCurrentScene() {
	if (currentScene_.empty()) {
		Console::LogError("No current scene to save.");
		return;
	}

	sceneIO_->Output(currentScene_, pEcs_->GetCurrentGroup());
	SetDirty(false);
}

void SceneManager::SaveCurrentSceneTemporary() {
	temporarySavedSceneName_ = currentScene_;
	sceneIO_->OutputTemporary(pEcs_->GetCurrentGroup());
}

void SceneManager::ClearTemporarySavedSceneName() {
	temporarySavedSceneName_.clear();
}

void SceneManager::LoadScene(const std::string& sceneName) {
	SetNextScene(sceneName);
	if (nextScene_.empty()) {
		Console::LogError("Failed to load scene: " + sceneName);
		return;
	}

	// MoveNextToCurrentScene(false);
	// NOTE: シーン遷移処理（旧シーンの破棄と新シーンの読み込み）を即時実行するのではなく、
	// 次のフレームの開始時（SceneManager::Update）まで遅延させます。
	// これにより、OnSelectやOnSubmitなどのスクリプト・システム更新処理のコールバック実行中に
	// エンティティやECSGroupが即座に破棄されてメモリが解放され、呼び出し元のC++コードで
	// use-after-free（ダングリングポインタアクセス）が発生しクラッシュする不具合を完全に防止します。
}

void SceneManager::ReloadScene(bool isTemporary) {
	std::string sceneToLoad = currentScene_;
	if (isTemporary && !temporarySavedSceneName_.empty()) {
		sceneToLoad = temporarySavedSceneName_;
	}

	if (sceneToLoad.empty()) {
		Console::LogError("No current scene to reload.");
		return;
	}
	/// 現在のシーンを再読み込み
	SetNextScene(sceneToLoad);
	if (nextScene_.empty()) {
		Console::LogError("Failed to reload scene: " + sceneToLoad);
		return;
	}
	MoveNextToCurrentScene(isTemporary);
}

SceneIO* SceneManager::GetSceneIO() {
	return sceneIO_.get();
}

std::string SceneManager::LastOpenSceneName() {
	const std::string& filepath = "./Packages/Config/LastOpenScene.json";

	std::ifstream ifs(filepath);
	if (!ifs.is_open()) {
		return "";
	}

	nlohmann::json json;
	ifs >> json;

	ifs.close();
	if (json.contains("Scene") && json["Scene"].is_string()) {
		return json["Scene"];
	}

	return "";
}

void SceneManager::MarkDirty() {
	isDirty_ = true;
}

bool SceneManager::IsDirty() const {
	return isDirty_;
}

void SceneManager::SetDirty(bool isDirty) {
	isDirty_ = isDirty;
}

void SceneManager::MoveNextToCurrentScene(bool isTemporary) {
	/// GPUの処理が終わるまで待つ（リソース破棄中のアクセスを防ぐ）
	pEcs_->GetDxManager()->GetDxCommand()->WaitForGpuComplete();

	ECSGroup* prevSceneGroup = pEcs_->GetCurrentGroup();
	if (prevSceneGroup) {
		prevSceneGroup->RemoveEntityAll();
	}

	currentScene_ = std::move(nextScene_);
	nextScene_.clear();

	ECSGroup* nextSceneGroup = pEcs_->AddECSGroup(GetCurrentSceneName());
	const std::string& sceneName = nextSceneGroup->GetGroupName();

	pEcs_->SetCurrentGroupName(sceneName);

	/// sceneに必要な情報を渡して初期化
	if (isTemporary) {
		sceneIO_->InputTemporary(nextSceneGroup);
		return;
	}

	sceneIO_->Input(sceneName, nextSceneGroup);

	SetDirty(false);

	Time::ResetTime();
}


const std::string& SceneManager::GetCurrentSceneName() const {
	return currentScene_;
}



void MonoInternalMethods::InternalLoadScene(MonoString* sceneName) {
	char* cstr = mono_string_to_utf8(sceneName);
	if (gSceneManager) {
		gSceneManager->LoadScene(cstr);
	}

	mono_free(cstr);
}
