#include "EditorViewCollection.h"

/// external
#include <imgui.h>

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/Window/WindowManager.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Editor/Manager/ImGuiManager.h"
#include "Tabs/DevelopTab.h"
#include "Windows/Develop/ProjectWindow.h"
#include "Tabs/GameTab.h"
#include "Tabs/PrefabTab.h"
#include "Tabs/EditorTab.h"
#include "Tabs/AITab.h"
#include "Tabs/EventTab.h"
#include "Tabs/AnimationTab.h"
#include "Tabs/UITab.h"
#include "Tabs/SettingsTab.h"
#include "Engine/Core/Event/GameEventData.h"

using namespace Editor;

/// ///////////////////////////////////////////////////
/// ImGuiWindowCollection
/// ///////////////////////////////////////////////////
EditorViewCollection::EditorViewCollection(
	ONEngine::DxManager* dxm,
	ONEngine::EntityComponentSystem* ecs,
	ONEngine::Asset::AssetCollection* assetCollection,
	ImGuiManager* imGuiManager,
	EditorManager* editorManager,
	ONEngine::SceneManager* sceneManager)
	: pImGuiManager_(imGuiManager), pSceneManager_(sceneManager), pAssetCollection_(assetCollection) {

	/// ここでwindowを生成する
	AddViewContainer("Develop", std::make_unique<DevelopTab>(dxm, ecs, assetCollection, editorManager, sceneManager));
	AddViewContainer("Game", std::make_unique<GameTab>(assetCollection));
	AddViewContainer("Prefab", std::make_unique<PrefabTab>(dxm, ecs, assetCollection, editorManager, sceneManager));
	AddViewContainer("AI", std::make_unique<AITab>(dxm, ecs, editorManager, sceneManager));
	AddViewContainer("Event", std::make_unique<EventTab>());
	AddViewContainer("Animation", std::make_unique<AnimationTab>(assetCollection));
	AddViewContainer("UI", std::make_unique<UITab>(dxm, ecs, assetCollection, editorManager, sceneManager));
	AddViewContainer("Settings", std::make_unique<SettingsTab>());

	// game windowで開始
	selectedMenuIndex_ = 0;
}

EditorViewCollection::~EditorViewCollection() {}

void EditorViewCollection::Update() {
	MainMenuUpdate();

	/// 終了リクエストの確認
	if(pImGuiManager_->GetWindowManager()->IsCloseRequested()) {
		if(pSceneManager_->IsDirty() || ONEngine::GameEventManager::GetInstance().IsDirty()) {
			ImGui::OpenPopup("Save Changes?");
		} else {
			PostQuitMessage(0);
		}
	}

	if(ImGui::BeginPopupModal("Save Changes?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		std::string msg = "The following have unsaved changes:\n";
		if (pSceneManager_->IsDirty()) msg += " - Scene\n";
		if (ONEngine::GameEventManager::GetInstance().IsDirty()) msg += " - Game Events\n";
		msg += "\nSave changes before exiting?";

		ImGui::Text(msg.c_str());
		ImGui::Separator();

		if(ImGui::Button("Save", ImVec2(120, 0))) {
			if (pSceneManager_->IsDirty()) pSceneManager_->SaveCurrentScene();
			if (ONEngine::GameEventManager::GetInstance().IsDirty()) ONEngine::GameEventManager::GetInstance().Save();
			
			PostQuitMessage(0);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if(ImGui::Button("Don't Save", ImVec2(120, 0))) {
			PostQuitMessage(0);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if(ImGui::Button("Cancel", ImVec2(120, 0))) {
			pImGuiManager_->GetWindowManager()->SetCloseRequested(false);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	/// 選択されたMenuの内容を表示する
	parentWindows_[selectedMenuIndex_]->ShowImGui();
	ONEngine::DebugConfig::selectedMode_ = selectedMenuIndex_;

}

void EditorViewCollection::AddViewContainer(const std::string& name, std::unique_ptr<class IEditorWindowContainer> window) {
	parentWindowNames_.push_back(name);
	window->pImGuiManager_ = pImGuiManager_;
	for(auto& child : window->children_) {
		child->SetImGuiManager(pImGuiManager_);
		child->SetParentContainer(window.get());
	}
	for(auto& child : window->pendingAdditions_) {
		child->SetImGuiManager(pImGuiManager_);
		child->SetParentContainer(window.get());
	}

	parentWindows_.push_back(std::move(window));
}

void EditorViewCollection::MainMenuUpdate() {
	/// ----- MainMenuの更新(選択されたMenuの内容を別の処理で表示する) ----- ///

	if(!ImGui::BeginMainMenuBar()) {
		return;
	}

	for(int i = 0; auto& name : parentWindowNames_) {
		int save = selectedMenuIndex_;

		if(i == save) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
		}

		if(ImGui::Button(name.c_str())) {
			selectedMenuIndex_ = i;
		}

		if(i == save) {
			ImGui::PopStyleColor();
		}

		++i;
	}

	ImGui::EndMainMenuBar();
}




/// ///////////////////////////////////////////////////
/// ImGuiの親windowクラス
/// ///////////////////////////////////////////////////

Editor::IEditorWindowContainer::IEditorWindowContainer(const std::string& windowName)
	: windowName_(windowName) {
}

void Editor::IEditorWindowContainer::ShowImGui() {
	uint32_t imGuiFlags_ = 0;
	imGuiFlags_ |= ImGuiWindowFlags_NoMove;
	imGuiFlags_ |= ImGuiWindowFlags_NoResize;
	imGuiFlags_ |= ImGuiWindowFlags_NoTitleBar;
	imGuiFlags_ |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::SetNextWindowPos(ImVec2(0, 20));
	ImGui::SetNextWindowSize(ImVec2(ONEngine::EngineConfig::kWindowSize.x, ONEngine::EngineConfig::kWindowSize.y));
	if(!ImGui::Begin(windowName_.c_str(), nullptr, imGuiFlags_)) {
		ImGui::End();
		return;
	}

	ImGuiID dockspaceID = ImGui::GetID((windowName_ + "DockSpace").c_str());
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f));

	UpdateViews();

	ImGui::End();
}

void IEditorWindowContainer::UpdateViews() {
	// 保留されていた追加ウィンドウを安全に children_ に移動する
	if (!pendingAdditions_.empty()) {
		for (auto& child : pendingAdditions_) {
			children_.push_back(std::move(child));
		}
		pendingAdditions_.clear();
	}

	std::erase_if(children_, [](const auto& child) {
		return !child->IsOpen();
	});

	// ProjectWindowの数を数える
	int projectWindowCount = 0;
	for (const auto& child : children_) {
		if (std::string(child->GetWindowType()) == "ProjectWindow") {
			projectWindowCount++;
		}
	}

	// 閉じられるかどうかのフラグを更新
	for (auto& child : children_) {
		if (std::string(child->GetWindowType()) == "ProjectWindow") {
			child->SetCanClose(projectWindowCount > 1);
		}
	}

	for(auto& child : children_) {
		child->ShowImGui();
	}
}

IEditorWindow* IEditorWindowContainer::AddView(std::unique_ptr<class IEditorWindow> child) {
	child->SetParentContainer(this);
	child->SetImGuiManager(this->pImGuiManager_);
	class IEditorWindow* childPtr = child.get();
	pendingAdditions_.push_back(std::move(child));
	return childPtr;
}
