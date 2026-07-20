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
#include "Engine/ECS/System/Audio/AudioPlaybackSystem.h"


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

	if (EngineConfig::isTestMode && !EngineConfig::testScene.empty()) {
		SetNextScene(EngineConfig::testScene);
	} else {
#ifdef DEBUG_MODE
		SetNextScene(LastOpenSceneName());
#else
		SetNextScene(EngineConfig::startScene);
#endif
	}

	MoveNextToCurrentScene(false);
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
	if (DebugConfig::isDebugging) {
		Console::LogWarning("Cannot save scene while the game is playing.");
		return;
	}

	if (currentScene_.empty()) {
		Console::LogError("No current scene to save.");
		return;
	}

	sceneIO_->Output(currentScene_, pEcs_->GetCurrentGroup());
	SetDirty(false);
}

void SceneManager::SaveCurrentSceneTemporary() {
	temporarySavedSceneName_ = currentScene_;
	temporarySavedAdditiveScenes_.clear();
	for (const auto& activeName : pEcs_->GetActiveGroupNames()) {
		if (activeName != currentScene_) {
			temporarySavedAdditiveScenes_.push_back(activeName);
		}
	}
	sceneIO_->OutputTemporary(pEcs_->GetCurrentGroup());
}

void SceneManager::ClearTemporarySavedSceneName() {
	temporarySavedSceneName_.clear();
	temporarySavedAdditiveScenes_.clear();
}

void SceneManager::LoadScene(const std::string& sceneName) {
	SetNextScene(sceneName);
	isNextSceneAdditive_ = false;
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

void SceneManager::AddScene(const std::string& sceneName) {
	SetNextScene(sceneName);
	isNextSceneAdditive_ = true;
	if (nextScene_.empty()) {
		Console::LogError("Failed to add scene: " + sceneName);
		return;
	}
}

void SceneManager::ReloadScene(bool isTemporary) {
	std::string sceneToLoad = currentScene_;
	std::vector<std::string> additivesToLoad;
	if (isTemporary) {
		if (!temporarySavedSceneName_.empty()) {
			sceneToLoad = temporarySavedSceneName_;
		}
		additivesToLoad = temporarySavedAdditiveScenes_;
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

	/// 追加シーンを再読み込み
	for (const auto& additiveScene : additivesToLoad) {
		AddScene(additiveScene);
	}
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

void SceneManager::UnloadScene(const std::string& sceneName) {
	pEcs_->GetDxManager()->GetDxCommand()->WaitForGpuComplete();

	ECSGroup* group = pEcs_->GetECSGroup(sceneName);
	if (group) {
		group->RemoveEntityAll();
	}

	pEcs_->RemoveActiveGroupName(sceneName);

	// If the current active scene is the one unloaded, switch to the remaining active scene
	if (currentScene_ == sceneName) {
		const auto& activeGroups = pEcs_->GetActiveGroupNames();
		if (!activeGroups.empty()) {
			currentScene_ = activeGroups.back();
			pEcs_->SetCurrentGroupName(currentScene_);
		} else {
			currentScene_.clear();
		}
	}
}

void SceneManager::MoveNextToCurrentScene(bool isTemporary) {
	/// GPUの処理が終わるまで待つ（リソース破棄中のアクセスを防ぐ）
	pEcs_->GetDxManager()->GetDxCommand()->WaitForGpuComplete();

	ECSGroup* prevSceneGroup = pEcs_->GetCurrentGroup();
	if (!isNextSceneAdditive_) {
		// Clear all active scenes
		if (pEcs_->GetActiveGroupNames().empty()) {
			if (prevSceneGroup) {
				if (auto* audioSys = prevSceneGroup->GetSystem<AudioPlaybackSystem>()) {
					audioSys->StopAllAudio();
				}
				prevSceneGroup->RemoveEntityAll();
			}
		} else {
			for (const auto& activeName : pEcs_->GetActiveGroupNames()) {
				if (auto* group = pEcs_->GetECSGroup(activeName)) {
					if (auto* audioSys = group->GetSystem<AudioPlaybackSystem>()) {
						audioSys->StopAllAudio();
					}
					group->RemoveEntityAll();
				}
			}
			pEcs_->ClearActiveGroupNames();
		}
	} else {
		// Make sure existing scene names are in activeGroupNames_
		if (pEcs_->GetActiveGroupNames().empty()) {
			if (prevSceneGroup) {
				pEcs_->AddActiveGroupName(prevSceneGroup->GetGroupName());
			}
		}
	}

	bool isAdditive = isNextSceneAdditive_;
	isNextSceneAdditive_ = false; // Reset the flag

	std::string sceneToLoad = std::move(nextScene_);
	nextScene_.clear();

	ECSGroup* nextSceneGroup = pEcs_->AddECSGroup(sceneToLoad);
	const std::string& sceneName = nextSceneGroup->GetGroupName();

	if (!isAdditive) {
		currentScene_ = sceneToLoad;
		pEcs_->SetCurrentGroupName(sceneName);
	}
	pEcs_->AddActiveGroupName(sceneName);

	/// sceneに必要な情報を渡して初期化
	if (isTemporary) {
		sceneIO_->InputTemporary(nextSceneGroup);
		pEcs_->MainCameraSetting();
		return;
	}

	sceneIO_->Input(sceneName, nextSceneGroup);
	pEcs_->MainCameraSetting();

	SetDirty(false);

	Time::ResetTime();
}


const std::string& SceneManager::GetCurrentSceneName() const {
	return currentScene_;
}

void SceneManager::SetUpdatePaused(const std::string& sceneName, bool paused) {
	if (ECSGroup* group = pEcs_->GetECSGroup(sceneName)) {
		group->SetUpdatePaused(paused);
	} else {
		Console::LogWarning("SetUpdatePaused: ECSGroup '" + sceneName + "' not found.");
	}
}

bool SceneManager::IsUpdatePaused(const std::string& sceneName) {
	if (ECSGroup* group = pEcs_->GetECSGroup(sceneName)) {
		return group->IsUpdatePaused();
	}
	return false;
}

void SceneManager::SetDrawPaused(const std::string& sceneName, bool paused) {
	if (ECSGroup* group = pEcs_->GetECSGroup(sceneName)) {
		group->SetDrawPaused(paused);
	} else {
		Console::LogWarning("SetDrawPaused: ECSGroup '" + sceneName + "' not found.");
	}
}

bool SceneManager::IsDrawPaused(const std::string& sceneName) {
	if (ECSGroup* group = pEcs_->GetECSGroup(sceneName)) {
		return group->IsDrawPaused();
	}
	return false;
}



void MonoInternalMethods::InternalLoadScene(MonoString* sceneName) {
	char* cstr = mono_string_to_utf8(sceneName);
	if (gSceneManager) {
		gSceneManager->LoadScene(cstr);
	}

	mono_free(cstr);
}

void MonoInternalMethods::InternalAddScene(MonoString* sceneName) {
	char* cstr = mono_string_to_utf8(sceneName);
	if (gSceneManager) {
		gSceneManager->AddScene(cstr);
	}

	mono_free(cstr);
}

void MonoInternalMethods::InternalUnloadScene(MonoString* sceneName) {
	char* cstr = mono_string_to_utf8(sceneName);
	if (gSceneManager) {
		gSceneManager->UnloadScene(cstr);
	}

	mono_free(cstr);
}

void MonoInternalMethods::InternalSetUpdatePaused(MonoString* sceneName, bool paused) {
	char* cstr = mono_string_to_utf8(sceneName);
	if (gSceneManager) {
		gSceneManager->SetUpdatePaused(cstr, paused);
	}
	mono_free(cstr);
}

bool MonoInternalMethods::InternalIsUpdatePaused(MonoString* sceneName) {
	char* cstr = mono_string_to_utf8(sceneName);
	bool result = false;
	if (gSceneManager) {
		result = gSceneManager->IsUpdatePaused(cstr);
	}
	mono_free(cstr);
	return result;
}

void MonoInternalMethods::InternalSetDrawPaused(MonoString* sceneName, bool paused) {
	char* cstr = mono_string_to_utf8(sceneName);
	if (gSceneManager) {
		gSceneManager->SetDrawPaused(cstr, paused);
	}
	mono_free(cstr);
}

bool MonoInternalMethods::InternalIsDrawPaused(MonoString* sceneName) {
	char* cstr = mono_string_to_utf8(sceneName);
	bool result = false;
	if (gSceneManager) {
		result = gSceneManager->IsDrawPaused(cstr);
	}
	mono_free(cstr);
	return result;
}
