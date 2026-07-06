#include "HierarchyWindow.h"

/// std
#include <filesystem>
#include <algorithm>
#include <fstream>

/// external
#include <imgui.h>
#include <dialog/ImGuiFileDialog.h>
#include <nlohmann/json.hpp>

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/Utility/Math/Math.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/ECS/Entity/EntityJsonConverter.h"
#include "Engine/Scene/SceneManager.h"

/// editor
#include "Engine/Editor/Commands/WorldEditorCommands/WorldEditorCommands.h"
#include "Engine/Editor/Manager/EditCommand.h"
#include "Engine/Editor/Manager/EditorManager.h"
#include "Engine/Editor/Math/ImGuiMath.h"
#include "Engine/Editor/Math/ImGuiSelection.h"
#include "Engine/Editor/Commands/ImGuiCommand/FocusEntityCommand.h"
#include "Engine/Editor/Math/AssetPayload.h"

namespace Editor {

HierarchyWindow::HierarchyWindow(
	const std::string& windowName,
	ONEngine::EntityComponentSystem* ecs,
	ONEngine::ECSGroup* ecsGroup,
	EditorManager* editorManager,
	ONEngine::SceneManager* sceneManager)
	: windowName_(windowName), pEcs_(ecs), pEcsGroup_(ecsGroup), pEditorManager_(editorManager),
	pSceneManager_(sceneManager), lastEcsGroup_(nullptr), lastIsDebugging_(false) {

	newName_.reserve(1024);
	isNodeOpen_ = false;

	// 無効なGuidで初期化しておく
	renameEntityGuid_ = ONEngine::Guid::kInvalid;
}

void HierarchyWindow::ShowImGui() {
	if(!ImGui::Begin(windowName_.c_str(), nullptr)) {
		ImGui::End();
		return;
	}

	/// グローバルショートカットの処理
	HandleGlobalShortcuts();

	/// メニューバーの描画
	DrawMenuBar();
	ImGui::Separator();

	// スクロール可能なエリアを開始
	ImGui::BeginChild("HierarchyScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	// シーンロードなどでアクティブなECSグループや再生状態が変わった場合に、選択状態の展開処理を再トリガーする
	if (pEcsGroup_ != lastEcsGroup_ || ONEngine::DebugConfig::isDebugging != lastIsDebugging_) {
		lastEcsGroup_ = pEcsGroup_;
		lastIsDebugging_ = ONEngine::DebugConfig::isDebugging;
		lastSelectedGuid_ = ONEngine::Guid::kInvalid;
	}

	// 選択の変更を監視し、新しく選択されたEntityの先祖ノードを展開する
	ONEngine::Guid currentSelected = ImGuiSelection::GetLastSelectedObject();
	if (currentSelected != lastSelectedGuid_) {
		lastSelectedGuid_ = currentSelected;
		forceExpandGuids_.clear();
		if (currentSelected.CheckValid() && ImGuiSelection::GetSelectionType() == SelectionType::Entity) {
			ONEngine::GameEntity* selectedEntity = nullptr;
			for (const auto& groupPair : pEcs_->GetECSGroups()) {
				if (auto* ent = groupPair.second->GetEntityFromGuid(currentSelected)) {
					selectedEntity = ent;
					break;
				}
			}
			if (selectedEntity) {
				ONEngine::GameEntity* parent = selectedEntity->GetParent();
				while (parent) {
					forceExpandGuids_.insert(parent->GetGuid());
					parent = parent->GetParent();
				}
			}
		}
	}

	/// ヒエラルキーの描画
	DrawHierarchy();

	/// ドラッグ＆ドロップの受け入れ（ルートへの移動用余白を確保）
	HandleRootDragDrop();

	ImGui::EndChild();

	// ★追加: ウィンドウ全体の空きスペースへのドロップを受け入れる
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {
			const AssetPayload* assetPayload = *static_cast<AssetPayload**>(payload->Data);
			if (assetPayload->filePath.ends_with(".prefab")) {
				pEditorManager_->ExecuteCommand<InstantiatePrefabCommand>(pEcsGroup_, assetPayload->filePath);
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::End();

	/// 各種ポップアップの表示
	ShowInvalidParentPopup();
	DrawDialog();
	DrawSceneSaveDialog();

	// 新規シーン作成ポップアップ
	if (showNewScenePopup_) {
		ImGui::OpenPopup("New Scene Name");
		showNewScenePopup_ = false;
	}

	if (ImGui::BeginPopupModal("New Scene Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter new scene name:");
		ImGuiInputText("##newSceneName", &newSceneName_);

		if (ImGui::Button("Create", ImVec2(120, 0)) || (ImGui::IsKeyPressed(ImGuiKey_Enter))) {
			if (!newSceneName_.empty()) {
				// 1. 空のシーンを保存（ファイル作成）
				pSceneManager_->SaveScene(newSceneName_, pEcs_->AddECSGroup(newSceneName_));
				// 2. そのシーンをロードして切り替え
				pSceneManager_->LoadScene(newSceneName_);
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void HierarchyWindow::DrawMenuBar() {
	if (ImGui::Button("+")) {
		ImGui::OpenPopup("HierarchyAddPopup");
	}

	if (ImGui::BeginPopup("HierarchyAddPopup")) {
		DrawMenuEntity();
		ImGui::Separator();
		DrawMenuScene();
		ImGui::EndPopup();
	}
}

void HierarchyWindow::DrawMenuEntity() {
	if (ImGui::BeginMenu("Create Entity")) {
		if (ImGui::MenuItem("Empty Object")) {
			pEditorManager_->ExecuteCommand<CreateGameObjectCommand>(pEcsGroup_, "NewEntity", nullptr);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Camera")) {
			pEditorManager_->ExecuteCommand<CreatePrimitiveCommand>(pEcsGroup_, CreatePrimitiveCommand::Type::Camera, nullptr);
		}
		if (ImGui::MenuItem("Directional Light")) {
			pEditorManager_->ExecuteCommand<CreatePrimitiveCommand>(pEcsGroup_, CreatePrimitiveCommand::Type::DirectionalLight, nullptr);
		}
		if (ImGui::MenuItem("Mesh")) {
			pEditorManager_->ExecuteCommand<CreatePrimitiveCommand>(pEcsGroup_, CreatePrimitiveCommand::Type::Mesh, nullptr);
		}
		if (ImGui::MenuItem("Sprite")) {
			pEditorManager_->ExecuteCommand<CreatePrimitiveCommand>(pEcsGroup_, CreatePrimitiveCommand::Type::Sprite, nullptr);
		}
		ImGui::EndMenu();
	}
}

void HierarchyWindow::DrawMenuScene() {
	if (ImGui::BeginMenu("Scene")) {
		if (ImGui::MenuItem("New Scene")) {
			showNewScenePopup_ = true;
			newSceneName_ = "NewScene";
		}
		if (ImGui::MenuItem("Save Scene")) {
			pSceneManager_->SaveCurrentScene();
		}
		if (ImGui::MenuItem("Load Scene")) {
			std::filesystem::path scenePath = std::filesystem::absolute("./Assets/Scene");
			std::filesystem::create_directories(scenePath);

			IGFD::FileDialogConfig config;
			config.path = scenePath.string();
			// ディレクトリを表示しないように設定
			config.userFileAttributes = [](IGFD::FileInfos* infos, IGFD::UserDatas /*userDatas*/) -> bool {
				return !infos->fileType.isDir();
			};
			ImGuiFileDialog::Instance()->OpenDialog("LoadSceneDialog", "Choose Scene", ".scene", config);
		}
		ImGui::EndMenu();
	}
}

void HierarchyWindow::DrawHierarchy() {
	flatHierarchyGuids_.clear();

	std::vector<std::string> groupsToDraw;
	if (isMultiGroup_) {
		const auto& activeGroupNames = pEcs_->GetActiveGroupNames();
		if (activeGroupNames.empty()) {
			if (auto* currentGroup = pEcs_->GetCurrentGroup()) {
				groupsToDraw.push_back(currentGroup->GetGroupName());
			}
		} else {
			groupsToDraw = activeGroupNames;
		}
	} else {
		if (pEcsGroup_) {
			groupsToDraw.push_back(pEcsGroup_->GetGroupName());
		}
	}

	for (const auto& groupName : groupsToDraw) {
		ONEngine::ECSGroup* group = pEcs_->GetECSGroup(groupName);
		if (!group) continue;

		// Temporarily set pEcsGroup_ to the group we are drawing
		pEcsGroup_ = group;

		ImGui::PushID(groupName.c_str());

		const auto& entities = pEcsGroup_->GetEntities();

		// ---------------------------------------------------
		// 0. シーンヘッダーの描画
		// ---------------------------------------------------
		std::string sceneName = groupName;
		if (groupName == pSceneManager_->GetCurrentSceneName()) {
			if (pSceneManager_->IsDirty()) {
				sceneName += " *";
			}
		}

		ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool sceneNodeOpen = ImGui::CollapsingHeader(sceneName.c_str(), sceneFlags);

		// シーンヘッダーへのドラッグ＆ドロップ（ルートへの移動）
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntityData")) {
				ONEngine::GameEntity** srcEntityPtr = static_cast<ONEngine::GameEntity**>(payload->Data);
				ONEngine::GameEntity* srcEntity = *srcEntityPtr;
				// ルートの先頭に配置
				pEditorManager_->ExecuteCommand<ReorderEntityCommand>(pEcsGroup_, srcEntity, nullptr, 0);
			}
			ImGui::EndDragDropTarget();
		}

		// 右クリックでコンテキストメニュー
		if (ImGui::BeginPopupContextItem("SceneHeaderContext")) {
			DrawMenuEntity();
			ImGui::Separator();
			DrawMenuScene();
			ImGui::EndPopup();
		}

		if (sceneNodeOpen) {
			// ---------------------------------------------------
			// 1. 各エンティティの描画
			// ---------------------------------------------------
			uint32_t rootIndex = 0;
			for (const auto& entity : entities) {
				// ルートエンティティのみ開始
				if (!entity->GetParent()) {
					DrawReorderSeparator(nullptr, rootIndex);
					DrawEntity(entity.get());
					rootIndex++;
				}
			}
			// 最後の隙間
			DrawReorderSeparator(nullptr, rootIndex);
		}

		/// 遅延削除の実行
		if(!deleteQueue_.empty()) {
			for(const auto& guid : deleteQueue_) {
				ONEngine::GameEntity* entity = pEcsGroup_->GetEntityFromGuid(guid);
				if(entity) {
					pEditorManager_->ExecuteCommand<DeleteEntityCommand>(pEcsGroup_, entity);
				}
			}
			deleteQueue_.clear();
		}

		ImGui::PopID();
	}

	// ---------------------------------------------------
	// 2. 背景クリックで選択解除
	// ---------------------------------------------------
	if(ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemActive() && !ImGui::GetIO().KeyCtrl) {
		ImGuiSelection::SetSelectedObject(ONEngine::Guid::kInvalid, SelectionType::None);
		shiftStartGuid_ = ONEngine::Guid::kInvalid;
	}

	// ---------------------------------------------------
	// 3. ボックス選択 (Marquee Selection)
	// ---------------------------------------------------
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
		isMarqueeSelecting_ = true;
		marqueeStartPos_ = ImGui::GetMousePos();
		if (!ImGui::GetIO().KeyCtrl) {
			ImGuiSelection::ClearSelection();
		}
	}

	if (isMarqueeSelecting_) {
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			isMarqueeSelecting_ = false;
		} else {
			ImVec2 mousePos = ImGui::GetMousePos();
			marqueeMin_ = ImVec2((std::min)(marqueeStartPos_.x, mousePos.x), (std::min)(marqueeStartPos_.y, mousePos.y));
			marqueeMax_ = ImVec2((std::max)(marqueeStartPos_.x, mousePos.x), (std::max)(marqueeStartPos_.y, mousePos.y));

			// 選択範囲の可視化
			ImGui::GetForegroundDrawList()->AddRectFilled(marqueeMin_, marqueeMax_, ImColor(100, 150, 255, 50));
			ImGui::GetForegroundDrawList()->AddRect(marqueeMin_, marqueeMax_, ImColor(100, 150, 255, 200));
		}
	}
}

void HierarchyWindow::DrawReorderSeparator(ONEngine::GameEntity* parent, uint32_t index) {
	ImGui::PushID(index);
	if (parent) ImGui::PushID(parent->GetId());
	else ImGui::PushID("root");

	// 非常に薄い不可視のボタンを配置
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
	ImGui::InvisibleButton("##reorder_target", ImVec2(-1, 4.0f));

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntityData")) {
			ONEngine::GameEntity** srcEntityPtr = static_cast<ONEngine::GameEntity**>(payload->Data);
			ONEngine::GameEntity* srcEntity = *srcEntityPtr;

			pEditorManager_->ExecuteCommand<ReorderEntityCommand>(pEcsGroup_, srcEntity, parent, index);
		}

		// ホバー中にラインを表示
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x, min.y + 1.0f), ImVec2(max.x, min.y + 3.0f), ImColor(100, 150, 255, 255));

		ImGui::EndDragDropTarget();
	}

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);

	ImGui::PopID();
	ImGui::PopID();
}

void HierarchyWindow::EntityRename(ONEngine::GameEntity* entity) {
	if(ImGuiInputText("##rename", &newName_, ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_EnterReturnsTrue)) {
		pEditorManager_->ExecuteCommand<EntityRenameCommand>(entity, newName_);
		renameEntityGuid_ = ONEngine::Guid::kInvalid; // 完了したらリセット
	}

	// フォーカスが外れたらリネームキャンセル
	if(ONEngine::Input::TriggerMouse(ONEngine::Mouse::Right) || ONEngine::Input::TriggerKey(DIK_ESCAPE)) {
		renameEntityGuid_ = ONEngine::Guid::kInvalid;
	}
}

void HierarchyWindow::DrawDialog() {
	if(ImGuiFileDialog::Instance()->Display("LoadSceneDialog", ImGuiWindowFlags_NoDocking)) {
		if(ImGuiFileDialog::Instance()->IsOk()) {
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string sceneName = std::filesystem::path(filePathName).stem().string();
			pSceneManager_->LoadScene(sceneName);
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

void HierarchyWindow::DrawSceneSaveDialog() {
	if(ImGuiFileDialog::Instance()->Display("save file dialog")) {
		if(ImGuiFileDialog::Instance()->IsOk()) {
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			nlohmann::json j = nlohmann::json::object();
			std::ofstream ofs(filePathName, std::ios::out | std::ios::binary);
			if(ofs) {
				ofs << j.dump(4);
				ofs.close();
			} else {
				ONEngine::Console::LogError("Failed to create file: " + filePathName);
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

bool HierarchyWindow::IsDescendant(ONEngine::GameEntity* ancestor, ONEngine::GameEntity* descendant) {
	if(!descendant) return false;
	ONEngine::GameEntity* current = descendant->GetParent();
	while(current) {
		if(current == ancestor) return true;
		current = current->GetParent();
	}
	return false;
}

void HierarchyWindow::ShowInvalidParentPopup() {
	if(showInvalidParentPopup_) {
		ImGui::OpenPopup("Invalid Parent");
		if(ImGui::BeginPopupModal("Invalid Parent", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Cannot set a descendant as a parent!");
			if(ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();
				showInvalidParentPopup_ = false;
			}
			ImGui::EndPopup();
		}
	}
}

void HierarchyWindow::DrawEntity(ONEngine::GameEntity* entity) {
	bool hasChildren = !entity->GetChildren().empty();
	flatHierarchyGuids_.push_back(entity->GetGuid());

	// ピッキング等の選択時に親ノードを自動展開する
	if (forceExpandGuids_.contains(entity->GetGuid())) {
		ImGui::SetNextItemOpen(true);
		forceExpandGuids_.erase(entity->GetGuid());
	}

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
	ImGui::PushID(entity->GetId());
	bool isSelected = ImGuiSelection::IsSelected(entity->GetGuid());
	if(isSelected) flags |= ImGuiTreeNodeFlags_Selected;
	if(!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	bool nodeOpen = ImGui::TreeNodeEx((void*)entity, flags, "");
	HandleEntityDragDrop(entity);

	if (isMarqueeSelecting_) {
		ImVec2 itemMin = ImGui::GetItemRectMin();
		ImVec2 itemMax = ImGui::GetItemRectMax();
		if (itemMax.x > marqueeMin_.x && itemMin.x < marqueeMax_.x && itemMax.y > marqueeMin_.y && itemMin.y < marqueeMax_.y) {
			ImGuiSelection::AddSelectedObject(entity->GetGuid(), SelectionType::Entity);
		}
	}

	if(DrawEntityContextMenu(entity, isSelected)) {
		ImGui::PopID();
		if(hasChildren && nodeOpen) ImGui::TreePop();
		return;
	}

	if(ImGui::IsItemHovered()) {
		if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			if(ImGui::GetIO().KeyShift) {
				ONEngine::Guid clickedGuid = entity->GetGuid();
				if (!shiftStartGuid_.CheckValid()) {
					shiftStartGuid_ = clickedGuid;
				}
				auto startIt = std::find(flatHierarchyGuids_.begin(), flatHierarchyGuids_.end(), shiftStartGuid_);
				auto endIt = std::find(flatHierarchyGuids_.begin(), flatHierarchyGuids_.end(), clickedGuid);
				if (startIt != flatHierarchyGuids_.end() && endIt != flatHierarchyGuids_.end()) {
					if (!ImGui::GetIO().KeyCtrl) {
						ImGuiSelection::ClearSelection();
					}
					size_t startIndex = std::distance(flatHierarchyGuids_.begin(), startIt);
					size_t endIndex = std::distance(flatHierarchyGuids_.begin(), endIt);
					size_t low = (std::min)(startIndex, endIndex);
					size_t high = (std::max)(startIndex, endIndex);
					for (size_t i = low; i <= high; ++i) {
						ImGuiSelection::AddSelectedObject(flatHierarchyGuids_[i], SelectionType::Entity);
					}
				}
			}
			else if(ImGui::GetIO().KeyCtrl) {
				if(isSelected) ImGuiSelection::RemoveSelectedObject(entity->GetGuid());
				else ImGuiSelection::AddSelectedObject(entity->GetGuid(), SelectionType::Entity);
				shiftStartGuid_ = entity->GetGuid();
			} else {
				ImGuiSelection::SetSelectedObject(entity->GetGuid(), SelectionType::Entity);
				shiftStartGuid_ = entity->GetGuid();
			}
		}
		if(ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			renameEntityGuid_ = entity->GetGuid();
			newName_ = entity->GetName();
		}
	}

	ImGui::SameLine();
	
	// アクティブフラグのチェックボックスを復元
	{
		std::string label = "##active" + std::to_string(entity->GetId());
		if (ImGui::Checkbox(label.c_str(), &entity->active)) {
			// 必要に応じてコマンド化
		}
	}

	ImGui::SameLine();

	if(renameEntityGuid_ == entity->GetGuid()) {
		EntityRename(entity);
	} else {
		ImGui::Text("%s", entity->GetName().c_str());
	}

	HandleEntityShortcuts(entity, isSelected);
	ImGui::PopID();

	if(hasChildren && nodeOpen) {
		uint32_t childIndex = 0;
		for(auto* child : entity->GetChildren()) {
			DrawReorderSeparator(entity, childIndex);
			DrawEntity(child);
			childIndex++;
		}
		// 最後の隙間
		DrawReorderSeparator(entity, childIndex);

		ImGui::TreePop();
	}
}

void HierarchyWindow::HandleRootDragDrop() {
	ImGui::Spacing();
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	
	// クラッシュ防止：サイズが 0 以下にならないようにガード
	windowSize.x = (std::max)(windowSize.x, 1.0f);
	windowSize.y = (std::max)(windowSize.y, 20.0f);

	ImGui::InvisibleButton("HierarchyDropArea", windowSize);
	if(ImGui::BeginDragDropTarget()) {
		if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntityData")) {
			ONEngine::GameEntity** srcEntityPtr = static_cast<ONEngine::GameEntity**>(payload->Data);
			ONEngine::GameEntity* srcEntity = *srcEntityPtr;
			
			// ルートの最後に配置
			const auto& entities = pEcsGroup_->GetEntities();
			pEditorManager_->ExecuteCommand<ReorderEntityCommand>(pEcsGroup_, srcEntity, nullptr, static_cast<uint32_t>(entities.size()));
		}

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {
			const AssetPayload* assetPayload = *static_cast<AssetPayload**>(payload->Data);
			if (assetPayload->filePath.ends_with(".prefab")) {
				pEditorManager_->ExecuteCommand<InstantiatePrefabCommand>(pEcsGroup_, assetPayload->filePath);
			}
		}

		ImGui::EndDragDropTarget();
	}
}

void HierarchyWindow::HandleEntityDragDrop(ONEngine::GameEntity* entity) {
	if(ImGui::BeginDragDropSource()) {
		ImGui::Text(entity->GetName().c_str());
		ONEngine::GameEntity** entityPtr = &entity;
		ImGui::SetDragDropPayload("EntityData", entityPtr, sizeof(ONEngine::GameEntity**));
		ImGui::EndDragDropSource();
	}

	if(ImGui::BeginDragDropTarget()) {
		if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntityData")) {
			ONEngine::GameEntity** srcEntityPtr = static_cast<ONEngine::GameEntity**>(payload->Data);
			ONEngine::GameEntity* srcEntity = *srcEntityPtr;
			if(srcEntity != entity) {
				if(!IsDescendant(srcEntity, entity)) {
					// 項目本体へのドロップは常に「子」にする
					pEditorManager_->ExecuteCommand<ChangeEntityParentCommand>(srcEntity, entity);
				} else {
					showInvalidParentPopup_ = true;
				}
			}
		}

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {
			const AssetPayload* assetPayload = *static_cast<AssetPayload**>(payload->Data);
			if (assetPayload->filePath.ends_with(".prefab")) {
				pEditorManager_->ExecuteCommand<InstantiatePrefabCommand>(pEcsGroup_, assetPayload->filePath, entity);
			}
		}

		ImGui::EndDragDropTarget();
	}
}

bool HierarchyWindow::DrawEntityContextMenu(ONEngine::GameEntity* entity, bool selected) {
	bool isDeleted = false;
	if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ImGui::OpenPopup("EntityContextMenu");
	if(ImGui::BeginPopup("EntityContextMenu")) {
		if(ImGui::BeginMenu("create")) {
			if(ImGui::MenuItem("empty object")) pEditorManager_->ExecuteCommand<CreateGameObjectCommand>(pEcsGroup_, "NewEntity", entity);
			ImGui::Separator();
			if(ImGui::MenuItem("Camera")) pEditorManager_->ExecuteCommand<CreatePrimitiveCommand>(pEcsGroup_, CreatePrimitiveCommand::Type::Camera, entity);
			if(ImGui::MenuItem("Directional Light")) pEditorManager_->ExecuteCommand<CreatePrimitiveCommand>(pEcsGroup_, CreatePrimitiveCommand::Type::DirectionalLight, entity);
			if(ImGui::MenuItem("Mesh")) pEditorManager_->ExecuteCommand<CreatePrimitiveCommand>(pEcsGroup_, CreatePrimitiveCommand::Type::Mesh, entity);
			if(ImGui::MenuItem("Sprite")) pEditorManager_->ExecuteCommand<CreatePrimitiveCommand>(pEcsGroup_, CreatePrimitiveCommand::Type::Sprite, entity);
			ImGui::EndMenu();
		}
		if(ImGui::MenuItem("rename")) { renameEntityGuid_ = entity->GetGuid(); newName_ = entity->GetName(); }
		if(ImGui::MenuItem("delete")) {
			deleteQueue_.push_back(entity->GetGuid());
			if(selected) ImGuiSelection::SetSelectedObject(ONEngine::Guid::kInvalid, SelectionType::None);
			isDeleted = true;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("copy", "Ctrl+C")) {
			pEditorManager_->ExecuteCommand<CopyEntityCommand>(entity);
		}
		if (ImGui::MenuItem("paste", "Ctrl+V")) {
			pEditorManager_->ExecuteCommand<PasteEntityCommand>(pEcsGroup_, entity);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Prefab")) {
			std::string name = entity->GetName();
			pEditorManager_->ExecuteCommand<CreatePrefabCommand>(entity);
			// エンジン側のPrefabキャッシュを更新
			pEcs_->ReloadPrefab(name + ".prefab");
			// このエンティティ自体のPrefab参照を更新
			entity->SetPrefabName(name);
		}
		ImGui::EndPopup();
	}
	return isDeleted;
}

void HierarchyWindow::HandleEntityShortcuts(ONEngine::GameEntity* entity, bool selected) {
	if (!selected || !ImGui::IsWindowFocused()) return;
	if (ImGui::GetIO().WantTextInput) return;

	if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
		deleteQueue_.push_back(entity->GetGuid());
	}
	
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
		pEditorManager_->ExecuteCommand<CopyEntityCommand>(entity);
	}

	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V)) {
		pEditorManager_->ExecuteCommand<PasteEntityCommand>(pEcsGroup_, entity);
	}
}

void HierarchyWindow::HandleGlobalShortcuts() {
	if (!ImGui::IsWindowFocused()) return;
	if (ImGui::GetIO().WantTextInput) return;

	// 何も選択されていない、またはCtrl+Vのみを処理
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V)) {
		// 選択中のエンティティがあればその子として、なければルートにペースト
		ONEngine::Guid selectedGuid = ImGuiSelection::GetLastSelectedObject();
		ONEngine::GameEntity* selectedEntity = pEcsGroup_->GetEntityFromGuid(selectedGuid);
		pEditorManager_->ExecuteCommand<PasteEntityCommand>(pEcsGroup_, selectedEntity);
	}
}

/// NormalHierarchyWindow Implementation
NormalHierarchyWindow::NormalHierarchyWindow(const std::string& windowName, ONEngine::EntityComponentSystem* ecs, EditorManager* editorManager, ONEngine::SceneManager* sceneManager)
	: HierarchyWindow(windowName, ecs, ecs->GetCurrentGroup(), editorManager, sceneManager), pEcs_(ecs) {
	isMultiGroup_ = true;
}

void NormalHierarchyWindow::ShowImGui() {
	pEcsGroup_ = pEcs_->GetCurrentGroup();
	HierarchyWindow::ShowImGui();
}

void NormalHierarchyWindow::DrawSceneDialog() {}

} /// namespace Editor
