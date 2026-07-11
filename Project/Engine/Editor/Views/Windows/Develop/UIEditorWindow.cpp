#include "UIEditorWindow.h"

/// external
#include <imgui.h>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <dialog/ImGuiFileDialog.h>
#include <algorithm>

/// engine
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIGroupComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIElementComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UILinkNavigationComponent.h"
#include "Engine/Editor/Commands/ComponentEditCommands/ComponentJsonConverter.h"
#include "Engine/ECS/Component/Components/Interface/IComponent.h"
#include "Engine/Core/Utility/Input/Input.h"
#include "InspectorWindow.h"

using namespace Editor;
using namespace ONEngine;
using json = nlohmann::json;

// Anonymous namespace removed because we use UILinkNavigationComponent helper methods.

UIEditorWindow::UIEditorWindow(
	const std::string& title,
	ONEngine::EntityComponentSystem* ecs,
	ONEngine::Asset::AssetCollection* assetCollection,
	InspectorWindow* inspector)
	: windowName_(title), pEcs_(ecs), pAssetCollection_(assetCollection), pInspector_(inspector) {

	ed::Config config;
	config.SettingsFile = nullptr; // 自動セーブ無効
	m_Editor = ed::CreateEditor(&config);

	m_CurrentPrefabPath = "./Assets/Prefabs/TitleMenu.prefab";
}

UIEditorWindow::~UIEditorWindow() {
	if (m_Editor) {
		ed::DestroyEditor(m_Editor);
	}
}

void UIEditorWindow::ShowImGui() {
	ImGui::Begin(windowName_.c_str());

	// ツールバー
	DrawToolbar();

	ImGui::Separator();

	// ノードエディター
	DrawNodeEditor();

	ImGui::End();

	DrawFileDialogs();
}

void UIEditorWindow::DrawToolbar() {
	ImGui::Text("Prefab File Path: %s", m_CurrentPrefabPath.empty() ? "(None)" : m_CurrentPrefabPath.c_str());
	ImGui::SameLine();

	if (ImGui::Button("Load")) {
		std::filesystem::path uiPath = std::filesystem::absolute("./Assets/UI");
		std::filesystem::create_directories(uiPath);

		IGFD::FileDialogConfig config;
		config.path = uiPath.string();
		ImGuiFileDialog::Instance()->OpenDialog("LoadPrefabDialog", "Choose Prefab", ".prefab", config);
	}

	ImGui::SameLine();
	if (ImGui::Button("Save")) {
		if (m_CurrentPrefabPath.empty()) {
			std::filesystem::path uiPath = std::filesystem::absolute("./Assets/UI");
			std::filesystem::create_directories(uiPath);

			IGFD::FileDialogConfig config;
			config.path = uiPath.string();
			ImGuiFileDialog::Instance()->OpenDialog("SavePrefabDialog", "Save Prefab", ".prefab", config);
		} else {
			SavePrefab(m_CurrentPrefabPath);
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Save As...")) {
		std::filesystem::path uiPath = std::filesystem::absolute("./Assets/UI");
		std::filesystem::create_directories(uiPath);

		IGFD::FileDialogConfig config;
		config.path = uiPath.string();
		ImGuiFileDialog::Instance()->OpenDialog("SavePrefabDialog", "Save Prefab As", ".prefab", config);
	}

	ImGui::SameLine();
	if (ImGui::Button("Sync Preview")) {
		SyncWithEngineScene();
	}
}

void UIEditorWindow::DrawNodeEditor() {
	ed::SetCurrentEditor(m_Editor);
	ed::Begin("UI Node Editor Canvas");

	// 1. ノードを描画
	for (const auto& node : m_Nodes) {
		// グループとエレメントでノードの色を変える
		ImColor headerColor = (node->type == NodeType::Group) ? ImColor(40, 100, 200) : ImColor(40, 180, 100);
		
		ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(30, 30, 30));
		ed::PushStyleColor(ed::StyleColor_HovNodeBorder, ImColor(80, 80, 80));
		ed::PushStyleColor(ed::StyleColor_SelNodeBorder, ImColor(120, 120, 120));
		ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(2.0f, 8.0f, 2.0f, 8.0f));

		ed::BeginNode(node->id);

		// タイトル部
		ImGui::TextColored(headerColor, "%s Node [%s]", (node->type == NodeType::Group ? "Group" : "Element"), node->name.c_str());
		ImGui::Dummy(ImVec2(220.0f, 0.0f)); // Enforce a stable node width of 220px
		
		// Draw a node-constrained separator line
		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImVec2 p1 = ImVec2(p0.x + 220.0f, p0.y);
		ImGui::GetWindowDrawList()->AddLine(p0, p1, ImColor(ImGui::GetStyle().Colors[ImGuiCol_Separator]));
		ImGui::Dummy(ImVec2(0.0f, 4.0f));

		// 入力ピン
		for (const auto& pin : node->inputs) {
			ed::BeginPin(pin.id, ed::PinKind::Input);
			ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), ">");
			ed::EndPin();
			ImGui::SameLine();
			ImGui::TextUnformatted(pin.name.c_str());
		}

		// コンテンツ部
		std::string nodeIdStr = std::to_string(node->id.Get());
		if (node->type == NodeType::Group) {
			ImGui::Checkbox(("Is Focused##" + nodeIdStr).c_str(), &node->isFocused);
			ImGui::Checkbox(("Is Visible##" + nodeIdStr).c_str(), &node->isVisible);

			ImGui::Spacing();
			ImGui::Text("Submit Keys:");

			std::string preview = "";
			for (size_t i = 0; i < node->submitKeys.size(); ++i) {
				if (!preview.empty()) preview += ", ";
				preview += node->submitKeys[i];
			}
			if (preview.empty()) {
				preview = "(Select keys...)";
			}

			ImGui::PushItemWidth(140.0f);
			std::string buttonId = preview + "##BtnGroupCombo_" + nodeIdStr;
			if (ImGui::Button(buttonId.c_str(), ImVec2(140.0f, 0.0f))) {
				ImGui::OpenPopup(("SubmitKeySelectorPopup_" + nodeIdStr).c_str());
			}
			ImGui::PopItemWidth();

			ImGui::Spacing();
			ImGui::Text("Cancel Keys:");

			std::string cancelPreview = "";
			for (size_t i = 0; i < node->cancelKeys.size(); ++i) {
				if (!cancelPreview.empty()) cancelPreview += ", ";
				cancelPreview += node->cancelKeys[i];
			}
			if (cancelPreview.empty()) {
				cancelPreview = "(Select keys...)";
			}

			ImGui::PushItemWidth(140.0f);
			std::string cancelBtnId = cancelPreview + "##BtnCancelGroupCombo_" + nodeIdStr;
			if (ImGui::Button(cancelBtnId.c_str(), ImVec2(140.0f, 0.0f))) {
				ImGui::OpenPopup(("CancelKeySelectorPopup_" + nodeIdStr).c_str());
			}
			ImGui::PopItemWidth();
		} else {
			char idBuf[64];
			strncpy_s(idBuf, node->elementId.c_str(), sizeof(idBuf));
			ImGui::PushItemWidth(120.0f);
			if (ImGui::InputText(("ID##" + nodeIdStr).c_str(), idBuf, sizeof(idBuf))) {
				node->elementId = idBuf;
			}
			ImGui::InputInt(("Idx##" + nodeIdStr).c_str(), &node->elementIndex);
			ImGui::PopItemWidth();

			// カスタムキーピンの追加UI
			ImGui::Spacing();
			ImGui::Text("Add Key Pin:");
			
			auto& selectedFlags = m_SelectedKeysMap[node->id.Get()];
			const auto& keyNames = ONEngine::UILinkNavigationComponent::GetSupportedKeyNames();
			if (selectedFlags.size() != keyNames.size()) {
				selectedFlags.assign(keyNames.size(), false);
			}
			
			// 選択状態を表すプレビュー文字列を作成
			std::string preview = "";
			for (size_t i = 0; i < keyNames.size(); ++i) {
				if (selectedFlags[i]) {
					if (!preview.empty()) preview += ", ";
					preview += keyNames[i];
				}
			}
			if (preview.empty()) {
				preview = "(Select keys...)";
			}
			
			ImGui::PushItemWidth(140.0f);
			std::string buttonId = preview + "##BtnCombo_" + std::to_string(node->id.Get());
			if (ImGui::Button(buttonId.c_str(), ImVec2(140.0f, 0.0f))) {
				ImGui::OpenPopup(("KeySelectorPopup_" + std::to_string(node->id.Get())).c_str());
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			
			std::string addBtnId = "+##AddKeyPin_" + std::to_string(node->id.Get());
			if (ImGui::Button(addBtnId.c_str())) {
				std::vector<int32_t> keyCodes;
				for (size_t i = 0; i < keyNames.size(); ++i) {
					if (selectedFlags[i]) {
						int32_t code = ONEngine::UILinkNavigationComponent::ParseKeyCodeString(keyNames[i]);
						if (code != 0) {
							keyCodes.push_back(code);
						}
					}
				}
				if (!keyCodes.empty()) {
					std::sort(keyCodes.begin(), keyCodes.end());
					keyCodes.erase(std::unique(keyCodes.begin(), keyCodes.end()), keyCodes.end());
					
					std::string normalizedName = ONEngine::UILinkNavigationComponent::KeyCodesToString(keyCodes);
					bool exists = false;
					for (const auto& pin : node->outputs) {
						if (pin.name == normalizedName) exists = true;
					}
					if (!exists) {
						Pin newPin(m_NextId++, normalizedName, PinKind::Output, keyCodes);
						newPin.node = node.get();
						node->outputs.push_back(newPin);
					}
					// 選択状態をクリア
					selectedFlags.assign(keyNames.size(), false);
				}
			}
		}

		// 出力ピン
		int deletePinIndex = -1;
		for (size_t i = 0; i < node->outputs.size(); ++i) {
			auto& pin = node->outputs[i];
			
			if (pin.name != "ChildrenLink") {
				std::string deleteBtnId = "x##DeletePin_" + std::to_string(pin.id.Get());
				if (ImGui::Button(deleteBtnId.c_str())) {
					deletePinIndex = static_cast<int>(i);
				}
				ImGui::SameLine();
			}
			
			ImGui::TextUnformatted(pin.name.c_str());
			ImGui::SameLine(200.0f);
			ed::BeginPin(pin.id, ed::PinKind::Output);
			ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), ">");
			ed::EndPin();
		}

		if (deletePinIndex != -1) {
			ed::PinId idToDelete = node->outputs[deletePinIndex].id;
			m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(), [&](const Link& l) {
				return l.startPinId == idToDelete || l.endPinId == idToDelete;
			}), m_Links.end());
			node->outputs.erase(node->outputs.begin() + deletePinIndex);
		}

		ed::EndNode();
		ed::PopStyleVar();
		ed::PopStyleColor(3);
	}

	// 2. リンクを描画
	for (const auto& link : m_Links) {
		ed::Link(link.id, link.startPinId, link.endPinId);
	}

	// 3. リンクの新規作成
	if (ed::BeginCreate()) {
		ed::PinId inputPinId, outputPinId;
		if (ed::QueryNewLink(&inputPinId, &outputPinId)) {
			if (ed::AcceptNewItem()) {
				m_Links.push_back(Link(m_NextId++, inputPinId, outputPinId));
			}
		}
	}
	ed::EndCreate();

	// 4. リンクとノードの削除
	if (ed::BeginDelete()) {
		ed::LinkId linkId;
		while (ed::QueryDeletedLink(&linkId)) {
			if (ed::AcceptDeletedItem()) {
				auto it = std::remove_if(m_Links.begin(), m_Links.end(),
					[linkId](const Link& l) { return l.id == linkId; });
				m_Links.erase(it, m_Links.end());
			}
		}

		ed::NodeId nodeId;
		while (ed::QueryDeletedNode(&nodeId)) {
			auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(),
				[nodeId](const std::unique_ptr<Node>& n) { return n->id == nodeId; });
			if (it != m_Nodes.end()) {
				if (ed::AcceptDeletedItem()) {
					Node* node = it->get();
					// このノードに接続されているすべてのリンクを削除
					std::vector<ed::PinId> pinIds;
					for (const auto& pin : node->inputs) pinIds.push_back(pin.id);
					for (const auto& pin : node->outputs) pinIds.push_back(pin.id);

					m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(),
						[&pinIds](const Link& l) {
							return std::find(pinIds.begin(), pinIds.end(), l.startPinId) != pinIds.end() ||
								   std::find(pinIds.begin(), pinIds.end(), l.endPinId) != pinIds.end();
						}), m_Links.end());

					// ノードを削除
					m_Nodes.erase(it);
				}
			} else {
				ed::RejectDeletedItem();
			}
		}
	}
	ed::EndDelete();

	// 5. キャンバス上での右クリックによるノード作成 / ノード右クリックメニュー
	ed::Suspend();
	if (ed::ShowBackgroundContextMenu()) {
		ImGui::OpenPopup("Create Node Menu");
	}

	ed::NodeId contextNodeId;
	if (ed::ShowNodeContextMenu(&contextNodeId)) {
		m_ContextMenuNodeId = contextNodeId;
		ImGui::OpenPopup("Node Context Menu");
	}
	ed::Resume();

	ed::Suspend();
	if (ImGui::BeginPopup("Create Node Menu")) {
		ImVec2 mousePos = ImGui::GetMousePosOnOpeningCurrentPopup();

		if (ImGui::MenuItem("Create UIGroup Node")) {
			ONEngine::Guid newGuid = ONEngine::GenerateGuid();
			std::string name = "Group_" + std::to_string(m_Nodes.size());
			auto node = std::make_unique<Node>(m_NextId++, NodeType::Group, name, newGuid);
			node->submitKeys = { "Return", "Space", "GamepadA" };
			node->cancelKeys = { "Escape", "GamepadB" };

			// 入力ピンと出力ピンの作成
			node->inputs.push_back(Pin(m_NextId++, "ParentLink", PinKind::Input));
			node->outputs.push_back(Pin(m_NextId++, "ChildrenLink", PinKind::Output));

			node->inputs[0].node = node.get();
			node->outputs[0].node = node.get();

			ed::SetNodePosition(node->id, mousePos);
			m_Nodes.push_back(std::move(node));
		}

		if (ImGui::MenuItem("Create UIElement Node")) {
			ONEngine::Guid newGuid = ONEngine::GenerateGuid();
			std::string name = "Element_" + std::to_string(m_Nodes.size());
			auto node = std::make_unique<Node>(m_NextId++, NodeType::Element, name, newGuid);

			// 入力ピンと4方向出力ピン of 作成
			node->inputs.push_back(Pin(m_NextId++, "In", PinKind::Input));
			node->inputs[0].node = node.get();

			std::vector<std::string> keys = { "UpArrow", "DownArrow", "LeftArrow", "RightArrow", "Return", "Space" };
			for (const auto& key : keys) {
				int32_t keyCode = ONEngine::UILinkNavigationComponent::ParseKeyCodeString(key);
				Pin pin(m_NextId++, key, PinKind::Output, std::vector<int32_t>{ keyCode });
				pin.node = node.get();
				node->outputs.push_back(pin);
			}

			ed::SetNodePosition(node->id, mousePos);
			m_Nodes.push_back(std::move(node));
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Node Context Menu")) {
		if (ImGui::MenuItem("Delete Node")) {
			ed::DeleteNode(m_ContextMenuNodeId);
		}
		ImGui::EndPopup();
	}
	ed::Resume();

	// Key selector popups
	ed::Suspend();
	for (const auto& node : m_Nodes) {
		if (node->type == NodeType::Element) {
			std::string popupId = "KeySelectorPopup_" + std::to_string(node->id.Get());
			if (ImGui::BeginPopup(popupId.c_str())) {
				const auto& keyNames = ONEngine::UILinkNavigationComponent::GetSupportedKeyNames();
				auto& selectedFlags = m_SelectedKeysMap[node->id.Get()];
				if (selectedFlags.size() != keyNames.size()) {
					selectedFlags.assign(keyNames.size(), false);
				}

				ImGui::Text("Select Keys for Pin:");
				ImGui::Separator();

				// 入力待機用のインタラクティブボタン
				bool isThisNodeListening = m_IsListeningForKey && (m_ListeningNodeId == node->id.Get());
				if (isThisNodeListening) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
					if (ImGui::Button("[Listening... Press a key / Click to Cancel]", ImVec2(250.0f, 0.0f))) {
						m_IsListeningForKey = false;
						m_ListeningNodeId = 0;
					}
					ImGui::PopStyleColor(3);

					// 待機状態時のみキー入力をリアルタイム監視して自動選択し、待機状態を解除する
					for (size_t i = 0; i < keyNames.size(); ++i) {
						const std::string& keyName = keyNames[i];
						int32_t keyCode = ONEngine::UILinkNavigationComponent::ParseKeyCodeString(keyName);
						if (keyCode != 0) {
							bool pressed = false;
							if (keyCode < 256) {
								pressed = ONEngine::Input::TriggerKey(keyCode);
							} else {
								pressed = ONEngine::Input::TriggerGamepad(keyCode - 256);
							}

							if (pressed) {
								// Escapeキーは除外
								if (keyName != "Escape") {
									selectedFlags[i] = !selectedFlags[i]; // トグル選択
									m_IsListeningForKey = false; // 入力されたので待機解除
									m_ListeningNodeId = 0;
									break;
								}
							}
						}
					}
				} else {
					if (ImGui::Button("Press Key to Select", ImVec2(250.0f, 0.0f))) {
						m_IsListeningForKey = true;
						m_ListeningNodeId = node->id.Get();
					}
				}
				ImGui::Separator();

				if (ImGui::BeginTabBar("KeyFilterTabBar")) {
					if (ImGui::BeginTabItem("All")) {
						m_CurrentKeyFilter = KeyFilter::All;
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Keyboard")) {
						m_CurrentKeyFilter = KeyFilter::Keyboard;
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Controller")) {
						m_CurrentKeyFilter = KeyFilter::Controller;
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Mouse")) {
						m_CurrentKeyFilter = KeyFilter::Mouse;
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::Spacing();
				
				ImGui::BeginChild("KeyListChild", ImVec2(250.0f, 250.0f), true);
				if (m_CurrentKeyFilter == KeyFilter::Mouse) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
					ImGui::TextWrapped("Mouse buttons are not used for link navigation.");
					ImGui::TextWrapped("Direct click-to-focus is handled automatically by the system.");
					ImGui::PopStyleColor();
				} else {
					for (size_t i = 0; i < keyNames.size(); ++i) {
						const std::string& keyName = keyNames[i];
						bool isGamepad = (keyName.rfind("Gamepad", 0) == 0);

						if (m_CurrentKeyFilter == KeyFilter::Keyboard && isGamepad) continue;
						if (m_CurrentKeyFilter == KeyFilter::Controller && !isGamepad) continue;

						bool isSelected = selectedFlags[i];
						std::string label = (isSelected ? "[x] " : "[ ] ") + keyName;
						if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
							selectedFlags[i] = !isSelected;
						}
					}
				}
				ImGui::EndChild();

				ImGui::Separator();
				if (ImGui::Button("Close", ImVec2(250.0f, 0.0f))) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		} else if (node->type == NodeType::Group) {
			std::string popupId = "SubmitKeySelectorPopup_" + std::to_string(node->id.Get());
			if (ImGui::BeginPopup(popupId.c_str())) {
				const auto& keyNames = ONEngine::UILinkNavigationComponent::GetSupportedKeyNames();
				auto& selectedFlags = m_SelectedKeysMap[node->id.Get()];
				if (selectedFlags.size() != keyNames.size()) {
					selectedFlags.assign(keyNames.size(), false);
					for (size_t i = 0; i < keyNames.size(); ++i) {
						if (std::find(node->submitKeys.begin(), node->submitKeys.end(), keyNames[i]) != node->submitKeys.end()) {
							selectedFlags[i] = true;
						}
					}
				}

				ImGui::Text("Select Submit Keys:");
				ImGui::Separator();

				// 入力待機用のインタラクティブボタン
				bool isThisNodeListening = m_IsListeningForKey && (m_ListeningNodeId == node->id.Get());
				if (isThisNodeListening) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
					if (ImGui::Button("[Listening... Press a key / Click to Cancel]", ImVec2(250.0f, 0.0f))) {
						m_IsListeningForKey = false;
						m_ListeningNodeId = 0;
					}
					ImGui::PopStyleColor(3);

					// 待機状態時のみキー入力をリアルタイム監視して自動選択し、待機状態を解除する
					for (size_t i = 0; i < keyNames.size(); ++i) {
						const std::string& keyName = keyNames[i];
						int32_t keyCode = ONEngine::UILinkNavigationComponent::ParseKeyCodeString(keyName);
						if (keyCode != 0) {
							bool pressed = false;
							if (keyCode < 256) {
								pressed = ONEngine::Input::TriggerKey(keyCode);
							} else {
								pressed = ONEngine::Input::TriggerGamepad(keyCode - 256);
							}

							if (pressed) {
								// Escapeキーは除外
								if (keyName != "Escape") {
									selectedFlags[i] = !selectedFlags[i]; // トグル選択
									m_IsListeningForKey = false; // 入力されたので待機解除
									m_ListeningNodeId = 0;

									// Update node->submitKeys immediately
									node->submitKeys.clear();
									for (size_t k = 0; k < keyNames.size(); ++k) {
										if (selectedFlags[k]) {
											node->submitKeys.push_back(keyNames[k]);
										}
									}
									break;
								}
							}
						}
					}
				} else {
					if (ImGui::Button("Press Key to Select", ImVec2(250.0f, 0.0f))) {
						m_IsListeningForKey = true;
						m_ListeningNodeId = node->id.Get();
					}
				}
				ImGui::Separator();

				if (ImGui::BeginTabBar("SubmitKeyFilterTabBar")) {
					if (ImGui::BeginTabItem("All")) {
						m_CurrentKeyFilter = KeyFilter::All;
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Keyboard")) {
						m_CurrentKeyFilter = KeyFilter::Keyboard;
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Controller")) {
						m_CurrentKeyFilter = KeyFilter::Controller;
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Mouse")) {
						m_CurrentKeyFilter = KeyFilter::Mouse;
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::Spacing();
				
				ImGui::BeginChild("SubmitKeyListChild", ImVec2(250.0f, 250.0f), true);
				if (m_CurrentKeyFilter == KeyFilter::Mouse) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
					ImGui::TextWrapped("Mouse buttons are not used for submit keys.");
					ImGui::PopStyleColor();
				} else {
					for (size_t i = 0; i < keyNames.size(); ++i) {
						const std::string& keyName = keyNames[i];
						bool isGamepad = (keyName.rfind("Gamepad", 0) == 0);

						if (m_CurrentKeyFilter == KeyFilter::Keyboard && isGamepad) continue;
						if (m_CurrentKeyFilter == KeyFilter::Controller && !isGamepad) continue;

						bool isSelected = selectedFlags[i];
						std::string label = (isSelected ? "[x] " : "[ ] ") + keyName;
						if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
							selectedFlags[i] = !isSelected;
							
							// Update node->submitKeys
							node->submitKeys.clear();
							for (size_t k = 0; k < keyNames.size(); ++k) {
								if (selectedFlags[k]) {
									node->submitKeys.push_back(keyNames[k]);
								}
							}
						}
					}
				}
				ImGui::EndChild();

				ImGui::Separator();
				if (ImGui::Button("Close", ImVec2(250.0f, 0.0f))) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			std::string cancelPopupId = "CancelKeySelectorPopup_" + std::to_string(node->id.Get());
			if (ImGui::BeginPopup(cancelPopupId.c_str())) {
				const auto& keyNames = ONEngine::UILinkNavigationComponent::GetSupportedKeyNames();
				auto& selectedFlags = m_SelectedKeysMap[node->id.Get() + 100000]; // submitKeys と衝突回避のオフセット
				if (selectedFlags.size() != keyNames.size()) {
					selectedFlags.assign(keyNames.size(), false);
					for (size_t i = 0; i < keyNames.size(); ++i) {
						if (std::find(node->cancelKeys.begin(), node->cancelKeys.end(), keyNames[i]) != node->cancelKeys.end()) {
							selectedFlags[i] = true;
						}
					}
				}

				ImGui::Text("Select Cancel Keys:");
				ImGui::Separator();

				// 入力待機用のインタラクティブボタン
				bool isThisNodeListening = m_IsListeningForKey && (m_ListeningNodeId == node->id.Get() + 100000);
				if (isThisNodeListening) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
					if (ImGui::Button("[Listening... Press a key / Click to Cancel]", ImVec2(250.0f, 0.0f))) {
						m_IsListeningForKey = false;
						m_ListeningNodeId = 0;
					}
					ImGui::PopStyleColor(3);

					// 待機状態時のみキー入力をリアルタイム監視して自動選択し、待機状態を解除する
					for (size_t i = 0; i < keyNames.size(); ++i) {
						const std::string& keyName = keyNames[i];
						int32_t keyCode = ONEngine::UILinkNavigationComponent::ParseKeyCodeString(keyName);
						if (keyCode != 0) {
							bool pressed = false;
							if (keyCode < 256) {
								pressed = ONEngine::Input::TriggerKey(keyCode);
							} else {
								pressed = ONEngine::Input::TriggerGamepad(keyCode - 256);
							}

							if (pressed) {
								// Escapeキーは除外
								if (keyName != "Escape") {
									selectedFlags[i] = !selectedFlags[i]; // トグル選択
									m_IsListeningForKey = false; // 入力されたので待機解除
									m_ListeningNodeId = 0;

									// Update node->cancelKeys immediately
									node->cancelKeys.clear();
									for (size_t k = 0; k < keyNames.size(); ++k) {
										if (selectedFlags[k]) {
											node->cancelKeys.push_back(keyNames[k]);
										}
									}
									break;
								}
							}
						}
					}
				} else {
					if (ImGui::Button("Press Key to Select", ImVec2(250.0f, 0.0f))) {
						m_IsListeningForKey = true;
						m_ListeningNodeId = node->id.Get() + 100000;
					}
				}
				ImGui::Separator();

				if (ImGui::BeginTabBar("CancelKeyFilterTabBar")) {
					if (ImGui::BeginTabItem("All")) {
						m_CurrentKeyFilter = KeyFilter::All;
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Keyboard")) {
						m_CurrentKeyFilter = KeyFilter::Keyboard;
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Controller")) {
						m_CurrentKeyFilter = KeyFilter::Controller;
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Mouse")) {
						m_CurrentKeyFilter = KeyFilter::Mouse;
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::Spacing();
				
				ImGui::BeginChild("CancelKeyListChild", ImVec2(250.0f, 250.0f), true);
				if (m_CurrentKeyFilter == KeyFilter::Mouse) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
					ImGui::TextWrapped("Mouse buttons are not used for cancel keys.");
					ImGui::PopStyleColor();
				} else {
					for (size_t i = 0; i < keyNames.size(); ++i) {
						const std::string& keyName = keyNames[i];
						bool isGamepad = (keyName.rfind("Gamepad", 0) == 0);

						if (m_CurrentKeyFilter == KeyFilter::Keyboard && isGamepad) continue;
						if (m_CurrentKeyFilter == KeyFilter::Controller && !isGamepad) continue;

						bool isSelected = selectedFlags[i];
						std::string label = (isSelected ? "[x] " : "[ ] ") + keyName;
						if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
							selectedFlags[i] = !isSelected;
							
							// Update node->cancelKeys
							node->cancelKeys.clear();
							for (size_t k = 0; k < keyNames.size(); ++k) {
								if (selectedFlags[k]) {
									node->cancelKeys.push_back(keyNames[k]);
								}
							}
						}
					}
				}
				ImGui::EndChild();

				ImGui::Separator();
				if (ImGui::Button("Close", ImVec2(250.0f, 0.0f))) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
	}
	ed::Resume();

	ed::End();
	ed::SetCurrentEditor(nullptr);
}

void UIEditorWindow::SavePrefab(const std::string& filepath) {
	std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());

	// ノード座標を取得するため、一時的にエディタコンテキストを設定
	if (m_Editor) {
		ed::SetCurrentEditor(m_Editor);
	}

	// 既存ファイルのコンポーネントを保存するマップ
	// GUID -> コンポーネントリスト の形で保持
	std::unordered_map<std::string, std::vector<json>> existingComponents;
	std::string existingRootGuid = "";
	std::vector<json> existingRootComponents;

	if (std::filesystem::exists(filepath)) {
		try {
			std::ifstream ifs(filepath);
			if (ifs.is_open()) {
				json j;
				ifs >> j;

				// ルートエンティティの情報を取得
				if (j.is_object()) {
					if (j.contains("guid")) {
						existingRootGuid = j["guid"].get<std::string>();
					}
					if (j.contains("components") && j["components"].is_array()) {
						existingRootComponents = j["components"].get<std::vector<json>>();
					}

					// 再帰的にエンティティを走査してコンポーネントを収集するヘルパー
					auto collectComponents = [&](auto& self, const json& entity) -> void {
						if (entity.is_object() && entity.contains("guid")) {
							std::string guidStr = entity["guid"].get<std::string>();
							if (entity.contains("components") && entity["components"].is_array()) {
								existingComponents[guidStr] = entity["components"].get<std::vector<json>>();
							}
						}
						// 子ノードも走査
						if (entity.is_object() && entity.contains("children") && entity["children"].is_array()) {
							for (const auto& child : entity["children"]) {
								self(self, child);
							}
						}
					};

					// 既存の entities 配列（旧フォーマット互換）または children 配列から収集
					if (j.contains("entities") && j["entities"].is_array()) {
						for (const auto& entity : j["entities"]) {
							collectComponents(collectComponents, entity);
						}
					}
					if (j.contains("children") && j["children"].is_array()) {
						for (const auto& child : j["children"]) {
							collectComponents(collectComponents, child);
						}
					}
				}
			}
		} catch (...) {
			// パース失敗時は無視
		}
	}

	// B. 次に、現在アクティブなシーン（ゲームメモリ）上の最新コンポーネント情報で上書きする
	ONEngine::ECSGroup* gameGroup = pEcs_->GetCurrentGroup();
	if (gameGroup) {
		for (auto& entity : gameGroup->GetEntities()) {
			std::string guidStr = entity->GetGuid().ToString();
			
			// このエンティティがUI関連コンポーネントを持っている場合、現在のシーン上の全コンポーネントを最新の状態でJSONシリアライズして退避
			if (entity->GetComponent<UIGroupComponent>() || entity->GetComponent<UIElementComponent>()) {
				std::vector<json> compsJson;
				for (const auto& pair : entity->GetComponents()) {
					IComponent* comp = pair.second;
					if (comp) {
						json compJson = ONEngine::ComponentJsonConverter::ToJson(comp);
						compsJson.push_back(compJson);
					}
				}
				// ディスクから読み込んだ古い情報を、メモリ上の最新情報で完全に上書きする
				existingComponents[guidStr] = compsJson;
			}
		}
	}

	// 1. ヘルパー関数: ノードとその子孫を再帰的にシリアライズする
	auto serializeNode = [&](auto& self, Node* node) -> json {
		json entityJson = json::object();
		entityJson["guid"] = node->guid.ToString();
		entityJson["name"] = node->name;
		entityJson["active"] = true;
		entityJson["prefabName"] = "";

		// エディタ上での位置情報を保存
		if (m_Editor) {
			ImVec2 pos = ed::GetNodePosition(node->id);
			entityJson["editorPosition"] = {
				{ "x", pos.x },
				{ "y", pos.y }
			};
		}

		json components = json::array();
		std::string nodeGuidStr = node->guid.ToString();

		json transformComp;
		bool hasExistingTransform = false;

		// 既存のコンポーネントを分類して退避
		std::vector<json> otherComponents;
		if (existingComponents.count(nodeGuidStr)) {
			for (const auto& comp : existingComponents[nodeGuidStr]) {
				if (comp.is_object() && comp.contains("type")) {
					std::string typeStr = comp["type"].get<std::string>();
					if (typeStr == "Transform") {
						transformComp = comp;
						hasExistingTransform = true;
					} else if (typeStr != "UIGroupComponent" && 
							   typeStr != "UIElementComponent" && 
							   typeStr != "UILinkNavigationComponent") {
						otherComponents.push_back(comp);
					}
				}
			}
		}

		// 1. Transformコンポーネントを先頭に追加
		if (!hasExistingTransform) {
			transformComp = {
				{ "type", "Transform" },
				{ "enable", 1 },
				{ "position", { { "x", 0.0 }, { "y", 0.0 }, { "z", 0.0 } } },
				{ "rotate", { { "w", 1.0 }, { "x", 0.0 }, { "y", 0.0 }, { "z", 0.0 } } },
				{ "scale", { { "x", 1.0 }, { "y", 1.0 }, { "z", 1.0 } } }
			};
		}
		components.push_back(transformComp);

		// 2. 他の非UIコンポーネント（SpriteRenderer, Script等）を追加
		for (const auto& comp : otherComponents) {
			components.push_back(comp);
		}

		// 3. UI関連コンポーネントを追加
		if (node->type == NodeType::Group) {
			std::string currentSelectedGuidStr = "";
			for (const auto& link : m_Links) {
				Pin* startPin = nullptr;
				Pin* endPin = nullptr;
				for (const auto& n : m_Nodes) {
					for (const auto& p : n->outputs) {
						if (p.id == link.startPinId) startPin = const_cast<Pin*>(&p);
					}
					for (const auto& p : n->inputs) {
						if (p.id == link.endPinId) endPin = const_cast<Pin*>(&p);
					}
				}
				if (startPin && endPin && startPin->node == node && endPin->node->type == NodeType::Element && endPin->node->name == "Element_1") {
					currentSelectedGuidStr = endPin->node->guid.ToString();
					break;
				}
			}

			json groupComp = {
				{ "type", "UIGroupComponent" },
				{ "enable", 1 },
				{ "isFocused", node->isFocused },
				{ "isVisible", node->isVisible },
				{ "currentSelected", currentSelectedGuidStr },
				{ "parentGroup", "" },
				{ "submitKeys", node->submitKeys.empty() ? std::vector<std::string>{"Return", "Space", "GamepadA"} : node->submitKeys },
				{ "cancelKeys", node->cancelKeys.empty() ? std::vector<std::string>{"Escape", "GamepadB"} : node->cancelKeys }
			};
			components.push_back(groupComp);
		} else {
			// UIElementComponent
			// 親グループのGUID特定（ピン結線から逆引きする）
			std::string parentGroupGuid = "";
			for (const auto& link : m_Links) {
				Pin* startPin = nullptr;
				Pin* endPin = nullptr;
				for (const auto& n : m_Nodes) {
					for (const auto& p : n->outputs) {
						if (p.id == link.startPinId) startPin = const_cast<Pin*>(&p);
					}
					for (const auto& p : n->inputs) {
						if (p.id == link.endPinId) endPin = const_cast<Pin*>(&p);
					}
				}
				if (startPin && endPin && startPin->node->type == NodeType::Group && endPin->node == node) {
					parentGroupGuid = startPin->node->guid.ToString();
					break;
				}
			}

			json elementComp = {
				{ "type", "UIElementComponent" },
				{ "enable", 1 },
				{ "groupId", parentGroupGuid },
				{ "elementId", node->elementId },
				{ "elementIndex", node->elementIndex }
			};
			components.push_back(elementComp);

			// UILinkNavigationComponent の作成 (結線ピン情報から出力)
			json linksObj = json::object();
			for (const auto& outPin : node->outputs) {
				for (const auto& link : m_Links) {
					if (link.startPinId == outPin.id) {
						// 接続先ノードを特定
						Pin* destPin = nullptr;
						for (const auto& n : m_Nodes) {
							for (const auto& p : n->inputs) {
								if (p.id == link.endPinId) destPin = const_cast<Pin*>(&p);
							}
						}
						if (destPin && destPin->node) {
							for (int32_t keyCode : outPin.keyCodes) {
								std::string keyStr = ONEngine::UILinkNavigationComponent::KeyCodeToString(keyCode);
								linksObj[keyStr] = destPin->node->guid.ToString();
							}
						}
					}
				}
			}

			json navComp = {
				{ "type", "UILinkNavigationComponent" },
				{ "enable", 1 },
				{ "links", linksObj }
			};
			components.push_back(navComp);
		}

		entityJson["components"] = components;

		// このノードを親とする子ノードを収集
		json childrenArray = json::array();
		for (const auto& n : m_Nodes) {
			Node* parentNode = nullptr;
			if (!n->inputs.empty()) {
				ed::PinId inputPinId = n->inputs[0].id;
				for (const auto& link : m_Links) {
					if (link.endPinId == inputPinId) {
						for (const auto& candidate : m_Nodes) {
							for (const auto& p : candidate->outputs) {
								if (p.id == link.startPinId) {
									parentNode = candidate.get();
									break;
								}
							}
							if (parentNode) break;
						}
					}
					if (parentNode) break;
				}
			}
			if (parentNode == node) {
				childrenArray.push_back(self(self, n.get()));
			}
		}

		if (!childrenArray.empty()) {
			entityJson["children"] = childrenArray;
		}

		return entityJson;
	};

	// 2. ルートエンティティの作成
	json rootJson = json::object();
	rootJson["active"] = true;
	rootJson["guid"] = existingRootGuid.empty() ? ONEngine::GenerateGuid().ToString() : existingRootGuid;
	std::string rootName = std::filesystem::path(filepath).stem().string();
	rootJson["name"] = rootName;
	rootJson["prefabName"] = "";

	json rootComponents = json::array();
	bool hasExistingRootTransform = false;
	std::vector<json> otherRootComponents;
	for (const auto& comp : existingRootComponents) {
		if (comp.is_object() && comp.contains("type")) {
			std::string typeStr = comp["type"].get<std::string>();
			if (typeStr == "Transform") {
				rootComponents.push_back(comp);
				hasExistingRootTransform = true;
			} else {
				otherRootComponents.push_back(comp);
			}
		}
	}

	if (!hasExistingRootTransform) {
		json transformComp = {
			{ "type", "Transform" },
			{ "enable", 1 },
			{ "position", { { "x", 0.0 }, { "y", 0.0 }, { "z", 0.0 } } },
			{ "rotate", { { "w", 1.0 }, { "x", 0.0 }, { "y", 0.0 }, { "z", 0.0 } } },
			{ "scale", { { "x", 1.0 }, { "y", 1.0 }, { "z", 1.0 } } }
		};
		rootComponents.insert(rootComponents.begin(), transformComp);
	}

	for (const auto& comp : otherRootComponents) {
		rootComponents.push_back(comp);
	}
	rootJson["components"] = rootComponents;

	// 3. 親ノードを持たないトップレベルのノードをルートの子要素として登録
	json rootChildren = json::array();
	for (const auto& n : m_Nodes) {
		Node* parentNode = nullptr;
		if (!n->inputs.empty()) {
			ed::PinId inputPinId = n->inputs[0].id;
			for (const auto& link : m_Links) {
				if (link.endPinId == inputPinId) {
					for (const auto& candidate : m_Nodes) {
						for (const auto& p : candidate->outputs) {
							if (p.id == link.startPinId) {
								parentNode = candidate.get();
								break;
							}
						}
						if (parentNode) break;
					}
				}
				if (parentNode) break;
			}
		}
		if (!parentNode) {
			rootChildren.push_back(serializeNode(serializeNode, n.get()));
		}
	}

	if (!rootChildren.empty()) {
		rootJson["children"] = rootChildren;
	}

	std::ofstream ofs(filepath);
	if (ofs) {
		ofs << rootJson.dump(4);
		ofs.close();
		ONEngine::Console::Log("[UI Editor] Prefab saved successfully (preserved non-UI components): " + filepath);
	} else {
		ONEngine::Console::LogError("[UI Editor] Failed to save prefab: " + filepath);
	}

	if (m_Editor) {
		ed::SetCurrentEditor(nullptr);
	}
}

void UIEditorWindow::DrawFileDialogs() {
	// 1. LoadPrefabDialog
	if (ImGuiFileDialog::Instance()->Display("LoadPrefabDialog", ImGuiWindowFlags_NoDocking)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string fullPath = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string relative = std::filesystem::relative(fullPath, std::filesystem::current_path()).string();
			std::replace(relative.begin(), relative.end(), '\\', '/');
			m_CurrentPrefabPath = "./" + relative;
			LoadPrefab(m_CurrentPrefabPath);
		}
		ImGuiFileDialog::Instance()->Close();
	}

	// 2. SavePrefabDialog
	if (ImGuiFileDialog::Instance()->Display("SavePrefabDialog", ImGuiWindowFlags_NoDocking)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string fullPath = ImGuiFileDialog::Instance()->GetFilePathName();
			if (!fullPath.ends_with(".prefab")) {
				fullPath += ".prefab";
			}
			std::string relative = std::filesystem::relative(fullPath, std::filesystem::current_path()).string();
			std::replace(relative.begin(), relative.end(), '\\', '/');
			m_CurrentPrefabPath = "./" + relative;
			SavePrefab(m_CurrentPrefabPath);
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

void UIEditorWindow::LoadPrefab(const std::string& filepath) {
	if (!std::filesystem::exists(filepath)) {
		ONEngine::Console::LogError("[UI Editor] Prefab file not found: " + filepath);
		return;
	}

	std::ifstream ifs(filepath);
	if (!ifs.is_open()) return;

	json rootJson;
	try {
		ifs >> rootJson;
	} catch (const std::exception& e) {
		ONEngine::Console::LogError("[UI Editor] Failed to parse prefab JSON: " + std::string(e.what()));
		ifs.close();
		return;
	}
	ifs.close();

	m_Nodes.clear();
	m_Links.clear();
	m_NextId = 1;
	m_SelectedKeysMap.clear();

	std::unordered_map<std::string, Node*> guidToNodeMap;
	std::unordered_map<std::string, json> navLinksMap;

	if (rootJson.contains("entities")) {
		// Old format (flat entities list)
		for (const auto& entityJson : rootJson["entities"]) {
			std::string guidStr = entityJson.value("guid", "");
			std::string name = entityJson.value("name", "UIElement");
			ONEngine::Guid guid = ONEngine::Guid::FromString(guidStr);

			NodeType t = NodeType::Element;
			bool isFocused = false;
			bool isVisible = true;
			std::string elementId = "";
			int elementIndex = 0;
			std::vector<std::string> submitKeys;
			bool hasSubmitKeys = false;
			std::vector<std::string> cancelKeys;
			bool hasCancelKeys = false;

			for (const auto& compJson : entityJson["components"]) {
				std::string compType = compJson.value("type", "");
				if (compType == "UIGroupComponent") {
					t = NodeType::Group;
					isFocused = compJson.value("isFocused", false);
					isVisible = compJson.value("isVisible", true);
					if (compJson.contains("submitKeys") && compJson["submitKeys"].is_array()) {
						submitKeys = compJson["submitKeys"].get<std::vector<std::string>>();
						hasSubmitKeys = true;
					}
					if (compJson.contains("cancelKeys") && compJson["cancelKeys"].is_array()) {
						cancelKeys = compJson["cancelKeys"].get<std::vector<std::string>>();
						hasCancelKeys = true;
					}
				} else if (compType == "UIElementComponent") {
					elementId = compJson.value("elementId", "");
					elementIndex = compJson.value("elementIndex", 0);
				}
			}

			auto node = std::make_unique<Node>(m_NextId++, t, name, guid);
			node->isFocused = isFocused;
			node->isVisible = isVisible;
			node->elementId = elementId;
			node->elementIndex = elementIndex;
			if (t == NodeType::Group) {
				node->submitKeys = hasSubmitKeys ? submitKeys : std::vector<std::string>{"Return", "Space", "GamepadA"};
				node->cancelKeys = hasCancelKeys ? cancelKeys : std::vector<std::string>{"Escape", "GamepadB"};
			}

			if (t == NodeType::Group) {
				node->inputs.push_back(Pin(m_NextId++, "ParentLink", PinKind::Input));
				node->outputs.push_back(Pin(m_NextId++, "ChildrenLink", PinKind::Output));
				node->inputs[0].node = node.get();
				node->outputs[0].node = node.get();
			} else {
				node->inputs.push_back(Pin(m_NextId++, "In", PinKind::Input));
				node->inputs[0].node = node.get();

				// 1. UILinkNavigationComponent からリンクを取得してターゲット毎にグループ化
				std::unordered_map<std::string, std::vector<std::string>> targetToKeys;
				
				for (const auto& compJson : entityJson["components"]) {
					std::string compType = compJson.value("type", "");
					if (compType == "UILinkNavigationComponent") {
						if (compJson.contains("links") && compJson["links"].is_object()) {
							navLinksMap[guidStr] = compJson["links"];
							for (auto it = compJson["links"].begin(); it != compJson["links"].end(); ++it) {
								std::string keyStr = it.key();
								std::string targetGuidStr = it.value().get<std::string>();
								if (!targetGuidStr.empty()) {
									targetToKeys[targetGuidStr].push_back(keyStr);
								}
							}
						}
					}
				}

				// 2. グループごとにピンを生成
				for (auto& pair : targetToKeys) {
					auto& keysList = pair.second;
					std::vector<int32_t> keyCodes;
					for (const auto& k : keysList) {
						int32_t code = ONEngine::UILinkNavigationComponent::ParseKeyCodeString(k);
						if (code != 0) {
							keyCodes.push_back(code);
						}
					}
					std::sort(keyCodes.begin(), keyCodes.end());
					keyCodes.erase(std::unique(keyCodes.begin(), keyCodes.end()), keyCodes.end());
					
					if (!keyCodes.empty()) {
						std::string pinName = ONEngine::UILinkNavigationComponent::KeyCodesToString(keyCodes);
						Pin pin(m_NextId++, pinName, PinKind::Output, keyCodes);
						pin.node = node.get();
						node->outputs.push_back(pin);
					}
				}

				// 3. デフォルトキーの中で、まだリンクに属していないものを単体の未接続ピンとして配置
				std::vector<std::string> defaultKeys = { "UpArrow", "DownArrow", "LeftArrow", "RightArrow", "Return", "Space" };
				for (const auto& dk : defaultKeys) {
					int32_t code = ONEngine::UILinkNavigationComponent::ParseKeyCodeString(dk);
					bool alreadyLinked = false;
					for (const auto& pin : node->outputs) {
						if (std::find(pin.keyCodes.begin(), pin.keyCodes.end(), code) != pin.keyCodes.end()) {
							alreadyLinked = true;
							break;
						}
					}
					if (!alreadyLinked) {
						Pin pin(m_NextId++, dk, PinKind::Output, std::vector<int32_t>{ code });
						pin.node = node.get();
						node->outputs.push_back(pin);
					}
				}
			}

			guidToNodeMap[guidStr] = node.get();
			m_Nodes.push_back(std::move(node));
		}

		// 2. リンク (結線) の再構築
		for (const auto& entityJson : rootJson["entities"]) {
			std::string srcGuidStr = entityJson.value("guid", "");
			Node* srcNode = guidToNodeMap[srcGuidStr];
			if (!srcNode) continue;

			for (const auto& compJson : entityJson["components"]) {
				std::string compType = compJson.value("type", "");
				if (compType == "UIElementComponent") {
					std::string parentGroupGuid = compJson.value("groupId", "");
					if (!parentGroupGuid.empty() && guidToNodeMap.find(parentGroupGuid) != guidToNodeMap.end()) {
						Node* parentNode = guidToNodeMap[parentGroupGuid];
						if (!parentNode->outputs.empty() && !srcNode->inputs.empty()) {
							m_Links.push_back(Link(m_NextId++, parentNode->outputs[0].id, srcNode->inputs[0].id));
						}
					}
				}
			}
		}
	} else {
		// New format (single-root tree)
		auto loadNodeRecursive = [&](auto& self, const json& entityJson, Node* parentNode) -> void {
			std::string guidStr = entityJson.value("guid", "");
			std::string name = entityJson.value("name", "UIElement");
			ONEngine::Guid guid = ONEngine::Guid::FromString(guidStr);

			NodeType t = NodeType::Element;
			bool isFocused = false;
			bool isVisible = true;
			std::string elementId = "";
			int elementIndex = 0;
			std::vector<std::string> submitKeys;
			bool hasSubmitKeys = false;
			std::vector<std::string> cancelKeys;
			bool hasCancelKeys = false;

			if (entityJson.contains("components")) {
				for (const auto& compJson : entityJson["components"]) {
					std::string compType = compJson.value("type", "");
					if (compType == "UIGroupComponent") {
						t = NodeType::Group;
						isFocused = compJson.value("isFocused", false);
						isVisible = compJson.value("isVisible", true);
						if (compJson.contains("submitKeys") && compJson["submitKeys"].is_array()) {
							submitKeys = compJson["submitKeys"].get<std::vector<std::string>>();
							hasSubmitKeys = true;
						}
						if (compJson.contains("cancelKeys") && compJson["cancelKeys"].is_array()) {
							cancelKeys = compJson["cancelKeys"].get<std::vector<std::string>>();
							hasCancelKeys = true;
						}
					} else if (compType == "UIElementComponent") {
						elementId = compJson.value("elementId", "");
						elementIndex = compJson.value("elementIndex", 0);
					} else if (compType == "UILinkNavigationComponent") {
						if (compJson.contains("links") && compJson["links"].is_object()) {
							navLinksMap[guidStr] = compJson["links"];
						}
					}
				}
			}

			auto node = std::make_unique<Node>(m_NextId++, t, name, guid);
			node->isFocused = isFocused;
			node->isVisible = isVisible;
			node->elementId = elementId;
			node->elementIndex = elementIndex;
			if (t == NodeType::Group) {
				node->submitKeys = hasSubmitKeys ? submitKeys : std::vector<std::string>{"Return", "Space", "GamepadA"};
				node->cancelKeys = hasCancelKeys ? cancelKeys : std::vector<std::string>{"Escape", "GamepadB"};
			}

			Node* nodeRaw = node.get();
			guidToNodeMap[guidStr] = nodeRaw;
			m_Nodes.push_back(std::move(node));

			if (t == NodeType::Group) {
				nodeRaw->inputs.push_back(Pin(m_NextId++, "ParentLink", PinKind::Input));
				nodeRaw->outputs.push_back(Pin(m_NextId++, "ChildrenLink", PinKind::Output));
				nodeRaw->inputs[0].node = nodeRaw;
				nodeRaw->outputs[0].node = nodeRaw;
			} else {
				nodeRaw->inputs.push_back(Pin(m_NextId++, "In", PinKind::Input));
				nodeRaw->inputs[0].node = nodeRaw;

				std::unordered_map<std::string, std::vector<std::string>> targetToKeys;
				if (navLinksMap.find(guidStr) != navLinksMap.end()) {
					const auto& linksObj = navLinksMap[guidStr];
					for (auto it = linksObj.begin(); it != linksObj.end(); ++it) {
						std::string keyStr = it.key();
						std::string targetGuidStr = it.value().get<std::string>();
						if (!targetGuidStr.empty()) {
							targetToKeys[targetGuidStr].push_back(keyStr);
						}
					}
				}

				for (auto& pair : targetToKeys) {
					auto& keysList = pair.second;
					std::vector<int32_t> keyCodes;
					for (const auto& k : keysList) {
						int32_t code = ONEngine::UILinkNavigationComponent::ParseKeyCodeString(k);
						if (code != 0) {
							keyCodes.push_back(code);
						}
					}
					std::sort(keyCodes.begin(), keyCodes.end());
					keyCodes.erase(std::unique(keyCodes.begin(), keyCodes.end()), keyCodes.end());

					if (!keyCodes.empty()) {
						std::string pinName = ONEngine::UILinkNavigationComponent::KeyCodesToString(keyCodes);
						Pin pin(m_NextId++, pinName, PinKind::Output, keyCodes);
						pin.node = nodeRaw;
						nodeRaw->outputs.push_back(pin);
					}
				}

				std::vector<std::string> defaultKeys = { "UpArrow", "DownArrow", "LeftArrow", "RightArrow", "Return", "Space" };
				for (const auto& dk : defaultKeys) {
					int32_t code = ONEngine::UILinkNavigationComponent::ParseKeyCodeString(dk);
					bool alreadyLinked = false;
					for (const auto& pin : nodeRaw->outputs) {
						if (std::find(pin.keyCodes.begin(), pin.keyCodes.end(), code) != pin.keyCodes.end()) {
							alreadyLinked = true;
							break;
						}
					}
					if (!alreadyLinked) {
						Pin pin(m_NextId++, dk, PinKind::Output, std::vector<int32_t>{ code });
						pin.node = nodeRaw;
						nodeRaw->outputs.push_back(pin);
					}
				}
			}

			if (parentNode && parentNode->type == NodeType::Group) {
				if (!parentNode->outputs.empty() && !nodeRaw->inputs.empty()) {
					m_Links.push_back(Link(m_NextId++, parentNode->outputs[0].id, nodeRaw->inputs[0].id));
				}
			}

			if (entityJson.contains("children")) {
				for (const auto& childJson : entityJson["children"]) {
					self(self, childJson, nodeRaw);
				}
			}
		};

		if (rootJson.contains("children")) {
			for (const auto& childJson : rootJson["children"]) {
				loadNodeRecursive(loadNodeRecursive, childJson, nullptr);
			}
		}
	}

	// 共通ナビゲーション接続線（リンク）の再構築 (新旧両フォーマットで共通処理として適用)
	for (const auto& pair : navLinksMap) {
		std::string srcGuidStr = pair.first;
		Node* srcNode = guidToNodeMap[srcGuidStr];
		if (!srcNode) continue;

		const auto& linksObj = pair.second;
		for (auto it = linksObj.begin(); it != linksObj.end(); ++it) {
			std::string key = it.key();
			int32_t code = ONEngine::UILinkNavigationComponent::ParseKeyCodeString(key);
			std::string targetGuidStr = it.value().get<std::string>();

			if (guidToNodeMap.find(targetGuidStr) != guidToNodeMap.end()) {
				Node* targetNode = guidToNodeMap[targetGuidStr];
				Pin* srcPin = nullptr;
				for (auto& p : srcNode->outputs) {
					if (std::find(p.keyCodes.begin(), p.keyCodes.end(), code) != p.keyCodes.end()) {
						srcPin = &p;
						break;
					}
				}
				if (srcPin && !targetNode->inputs.empty()) {
					bool linkExists = false;
					for (const auto& l : m_Links) {
						if (l.startPinId == srcPin->id && l.endPinId == targetNode->inputs[0].id) {
							linkExists = true;
							break;
						}
					}
					if (!linkExists) {
						m_Links.push_back(Link(m_NextId++, srcPin->id, targetNode->inputs[0].id));
					}
				}
			}
		}
	}

	// 各ノードの配置を初期位置で整列
	if (m_Editor) {
		ed::SetCurrentEditor(m_Editor);
		ImVec2 initialPos(100.0f, 100.0f);
		for (const auto& node : m_Nodes) {
			ed::SetNodePosition(node->id, initialPos);
			initialPos.x += 250.0f;
			if (initialPos.x > 800.0f) {
				initialPos.x = 100.0f;
				initialPos.y += 300.0f;
			}
		}
		ed::SetCurrentEditor(nullptr);
	}

	ONEngine::Console::Log("[UI Editor] Prefab loaded successfully: " + filepath);
}

void UIEditorWindow::SyncWithEngineScene() {
	// シーンをセーブして同期確認するため、現在の prefab をアクティブな ECSGroup に直接流し込みます
	ONEngine::ECSGroup* gameGroup = pEcs_->GetCurrentGroup();
	if (!gameGroup) return;

	// 同期用に一度保存
	SavePrefab(m_CurrentPrefabPath);

	// prefab内のデータを元に Game グループ上に GameEntity を再構築
	std::ifstream ifs(m_CurrentPrefabPath);
	if (!ifs.is_open()) return;

	json rootJson;
	ifs >> rootJson;
	ifs.close();

	// 既存の古いUIプレハブ由来のUIエンティティを一旦破棄 (簡易削除)
	auto& entities = gameGroup->GetEntities();
	std::vector<GameEntity*> toDelete;
	for (auto& entity : entities) {
		if (entity->GetComponent<UIGroupComponent>() || entity->GetComponent<UIElementComponent>()) {
			toDelete.push_back(entity.get());
		}
	}
	for (auto* entity : toDelete) {
		gameGroup->RemoveEntity(entity);
	}

	// プレハブに含まれるエンティティ群のロード
	if (rootJson.contains("entities")) {
		std::vector<std::pair<GameEntity*, json>> entityLoads;
		for (const auto& entityJson : rootJson["entities"]) {
			ONEngine::Guid guid = ONEngine::Guid::FromString(entityJson["guid"]);
			GameEntity* entity = gameGroup->GenerateEntity(guid, true);
			if (entity) {
				entity->SetName(entityJson.value("name", "UI_Node"));
				
				// 各コンポーネントの追加
				for (const auto& compJson : entityJson["components"]) {
					std::string compType = compJson.value("type", "");
					IComponent* comp = entity->AddComponent(compType);
					if (comp) {
						ONEngine::ComponentJsonConverter::FromJson(compJson, comp);
					}
				}
			}
		}
		ONEngine::Console::Log("[UI Editor] Sync Preview completed on Game ECSGroup!");
	} else {
		// New format (hierarchical tree)
		auto loadTreeEntity = [&](auto& self, const json& entityJson, GameEntity* parent) -> void {
			ONEngine::Guid guid = ONEngine::Guid::FromString(entityJson["guid"]);
			GameEntity* entity = gameGroup->GenerateEntity(guid, true);
			if (entity) {
				entity->SetName(entityJson.value("name", "UI_Node"));
				if (parent) {
					entity->SetParent(parent);
				}
				if (entityJson.contains("components")) {
					for (const auto& compJson : entityJson["components"]) {
						std::string compType = compJson.value("type", "");
						IComponent* comp = entity->AddComponent(compType);
						if (comp) {
							ONEngine::ComponentJsonConverter::FromJson(compJson, comp);
						}
					}
				}
				if (entityJson.contains("children")) {
					for (const auto& childJson : entityJson["children"]) {
						self(self, childJson, entity);
					}
				}
			}
		};
		
		loadTreeEntity(loadTreeEntity, rootJson, nullptr);
		ONEngine::Console::Log("[UI Editor] Sync Preview completed on Game ECSGroup (New single-root tree structure)!");
	}
}
