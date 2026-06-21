#include "UIEditorWindow.h"

/// external
#include <imgui.h>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

/// engine
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIGroupComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIElementComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UILinkNavigationComponent.h"
#include "Engine/Editor/Commands/ComponentEditCommands/ComponentJsonConverter.h"
#include "Engine/ECS/Component/Components/Interface/IComponent.h"
#include "InspectorWindow.h"

using namespace Editor;
using namespace ONEngine;
using json = nlohmann::json;

namespace {

int32_t ParseKeyCodeString(const std::string& keyStr) {
	if (keyStr == "UpArrow") return 0xC8;
	if (keyStr == "DownArrow") return 0xD0;
	if (keyStr == "LeftArrow") return 0xCB;
	if (keyStr == "RightArrow") return 0xCD;
	if (keyStr == "Return") return 0x1C;
	if (keyStr == "Space") return 0x39;
	if (keyStr == "W") return 0x11;
	if (keyStr == "S") return 0x1F;
	if (keyStr == "A") return 0x1E;
	if (keyStr == "D") return 0x20;
	return 0;
}

std::string KeyCodeToString(int32_t keyCode) {
	if (keyCode == 0xC8) return "UpArrow";
	if (keyCode == 0xD0) return "DownArrow";
	if (keyCode == 0xCB) return "LeftArrow";
	if (keyCode == 0xCD) return "RightArrow";
	if (keyCode == 0x1C) return "Return";
	if (keyCode == 0x39) return "Space";
	if (keyCode == 0x11) return "W";
	if (keyCode == 0x1F) return "S";
	if (keyCode == 0x1E) return "A";
	if (keyCode == 0x20) return "D";
	return std::to_string(keyCode);
}

} // namespace

UIEditorWindow::UIEditorWindow(
	const std::string& title,
	ONEngine::EntityComponentSystem* ecs,
	ONEngine::Asset::AssetCollection* assetCollection,
	InspectorWindow* inspector)
	: windowName_(title), pEcs_(ecs), pAssetCollection_(assetCollection), pInspector_(inspector) {

	ed::Config config;
	config.SettingsFile = nullptr; // 自動セーブ無効
	m_Editor = ed::CreateEditor(&config);

	m_CurrentPrefabPath = "./Assets/UI/TitleMenu.prefab";
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
}

void UIEditorWindow::DrawToolbar() {
	ImGui::Text("Prefab File Path: ");
	ImGui::SameLine();
	char pathBuf[256];
	strncpy_s(pathBuf, m_CurrentPrefabPath.c_str(), sizeof(pathBuf));
	if (ImGui::InputText("##prefabPath", pathBuf, sizeof(pathBuf))) {
		m_CurrentPrefabPath = pathBuf;
	}

	ImGui::SameLine();
	if (ImGui::Button("Load")) {
		LoadPrefab(m_CurrentPrefabPath);
	}

	ImGui::SameLine();
	if (ImGui::Button("Save")) {
		SavePrefab(m_CurrentPrefabPath);
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
		ImGui::Separator();

		// 入力ピン
		for (const auto& pin : node->inputs) {
			ed::BeginPin(pin.id, ed::PinKind::Input);
			ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), ">");
			ed::EndPin();
			ImGui::SameLine();
			ImGui::TextUnformatted(pin.name.c_str());
		}

		// コンテンツ部
		if (node->type == NodeType::Group) {
			ImGui::Checkbox("Is Focused", &node->isFocused);
			ImGui::Checkbox("Is Visible", &node->isVisible);
		} else {
			char idBuf[64];
			strncpy_s(idBuf, node->elementId.c_str(), sizeof(idBuf));
			if (ImGui::InputText("ID", idBuf, sizeof(idBuf))) {
				node->elementId = idBuf;
			}
			ImGui::InputInt("Idx", &node->elementIndex);
		}

		// 出力ピン
		for (const auto& pin : node->outputs) {
			ImGui::TextUnformatted(pin.name.c_str());
			
			float posX = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(">").x - 4.0f;
			if (posX > ImGui::GetCursorPosX()) {
				ImGui::SameLine(posX);
			} else {
				ImGui::SameLine();
			}

			ed::BeginPin(pin.id, ed::PinKind::Output);
			ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), ">");
			ed::EndPin();
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

	// 4. リンクの削除
	if (ed::BeginDelete()) {
		ed::LinkId linkId;
		while (ed::QueryDeletedLink(&linkId)) {
			if (ed::AcceptDeletedItem()) {
				auto it = std::remove_if(m_Links.begin(), m_Links.end(),
					[linkId](const Link& l) { return l.id == linkId; });
				m_Links.erase(it, m_Links.end());
			}
		}
	}
	ed::EndDelete();

	// 5. キャンバス上での右クリックによるノード作成
	ed::Suspend();
	if (ed::ShowBackgroundContextMenu()) {
		ImGui::OpenPopup("Create Node Menu");
	}
	ed::Resume();

	ed::Suspend();
	if (ImGui::BeginPopup("Create Node Menu")) {
		ImVec2 mousePos = ImGui::GetMousePosOnOpeningCurrentPopup();

		if (ImGui::MenuItem("Create UIGroup Node")) {
			ONEngine::Guid newGuid = ONEngine::GenerateGuid();
			std::string name = "Group_" + std::to_string(m_Nodes.size());
			auto node = std::make_unique<Node>(m_NextId++, NodeType::Group, name, newGuid);

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

			// 入力ピンと4方向出力ピンの作成
			node->inputs.push_back(Pin(m_NextId++, "In", PinKind::Input));
			node->inputs[0].node = node.get();

			std::vector<std::string> keys = { "UpArrow", "DownArrow", "LeftArrow", "RightArrow", "Return", "Space" };
			for (const auto& key : keys) {
				int32_t keyCode = ParseKeyCodeString(key);
				Pin pin(m_NextId++, key, PinKind::Output, keyCode);
				pin.node = node.get();
				node->outputs.push_back(pin);
			}

			ed::SetNodePosition(node->id, mousePos);
			m_Nodes.push_back(std::move(node));
		}

		ImGui::EndPopup();
	}
	ed::Resume();

	ed::End();
	ed::SetCurrentEditor(nullptr);
}

void UIEditorWindow::SavePrefab(const std::string& filepath) {
	std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());

	json rootJson = json::object();
	json entitiesArray = json::array();

	// 1. 各ノードをGameEntityのJSON形式でシリアライズ
	for (const auto& node : m_Nodes) {
		json entityJson = json::object();
		entityJson["guid"] = node->guid.ToString();
		entityJson["name"] = node->name;
		entityJson["active"] = true;
		entityJson["prefabName"] = "";

		json components = json::array();

		// デフォルトの Transform コンポーネント
		json transformComp = {
			{ "type", "Transform" },
			{ "enable", 1 },
			{ "position", { { "x", 0.0 }, { "y", 0.0 }, { "z", 0.0 } } },
			{ "rotate", { { "w", 1.0 }, { "x", 0.0 }, { "y", 0.0 }, { "z", 0.0 } } },
			{ "scale", { { "x", 1.0 }, { "y", 1.0 }, { "z", 1.0 } } }
		};
		components.push_back(transformComp);

		if (node->type == NodeType::Group) {
			json groupComp = {
				{ "type", "UIGroupComponent" },
				{ "enable", 1 },
				{ "isFocused", node->isFocused },
				{ "isVisible", node->isVisible },
				{ "currentSelected", "" },
				{ "parentGroup", "" }
			};
			components.push_back(groupComp);
		} else {
			// UIElementComponent
			// 親グループのGUID特定（ピン結線から逆引きする）
			std::string parentGroupGuid = "";
			for (const auto& link : m_Links) {
				// 出力ピンがGroupノードのChildrenLinkで、入力ピンがこのElementのInピンの時
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

				if (startPin && endPin && startPin->node->type == NodeType::Group && endPin->node == node.get()) {
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
							std::string keyStr = KeyCodeToString(outPin.keyCode);
							linksObj[keyStr] = destPin->node->guid.ToString();
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
		entitiesArray.push_back(entityJson);
	}

	rootJson["entities"] = entitiesArray;

	std::ofstream ofs(filepath);
	if (ofs) {
		ofs << rootJson.dump(4);
		ofs.close();
		ONEngine::Console::Log("[UI Editor] Prefab saved successfully: " + filepath);
	} else {
		ONEngine::Console::LogError("[UI Editor] Failed to save prefab: " + filepath);
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
	ifs >> rootJson;
	ifs.close();

	m_Nodes.clear();
	m_Links.clear();
	m_NextId = 1;

	if (!rootJson.contains("entities")) return;

	// 1. ノードの再構築
	std::unordered_map<std::string, Node*> guidToNodeMap;
	for (const auto& entityJson : rootJson["entities"]) {
		std::string guidStr = entityJson.value("guid", "");
		std::string name = entityJson.value("name", "UIElement");
		ONEngine::Guid guid = ONEngine::Guid::FromString(guidStr);

		NodeType t = NodeType::Element;
		bool isFocused = false;
		bool isVisible = true;
		std::string elementId = "";
		int elementIndex = 0;

		for (const auto& compJson : entityJson["components"]) {
			std::string compType = compJson.value("type", "");
			if (compType == "UIGroupComponent") {
				t = NodeType::Group;
				isFocused = compJson.value("isFocused", false);
				isVisible = compJson.value("isVisible", true);
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
			node->inputs.push_back(Pin(m_NextId++, "ParentLink", PinKind::Input));
			node->outputs.push_back(Pin(m_NextId++, "ChildrenLink", PinKind::Output));
			node->inputs[0].node = node.get();
			node->outputs[0].node = node.get();
		} else {
			node->inputs.push_back(Pin(m_NextId++, "In", PinKind::Input));
			node->inputs[0].node = node.get();

			std::vector<std::string> keys = { "UpArrow", "DownArrow", "LeftArrow", "RightArrow", "Return", "Space" };
			for (const auto& key : keys) {
				int32_t keyCode = ParseKeyCodeString(key);
				Pin pin(m_NextId++, key, PinKind::Output, keyCode);
				pin.node = node.get();
				node->outputs.push_back(pin);
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
					// Group の ChildrenLink -> Element の In ピンへの接続を作成
					m_Links.push_back(Link(m_NextId++, parentNode->outputs[0].id, srcNode->inputs[0].id));
				}
			} else if (compType == "UILinkNavigationComponent") {
				if (compJson.contains("links") && compJson.at("links").is_object()) {
					auto linksObj = compJson.at("links");
					for (auto it = linksObj.begin(); it != linksObj.end(); ++it) {
						std::string key = it.key();
						std::string targetGuidStr = it.value().get<std::string>();

						if (guidToNodeMap.find(targetGuidStr) != guidToNodeMap.end()) {
							Node* targetNode = guidToNodeMap[targetGuidStr];
							// Element の各方向キー出力ピン -> 遷移先ノードの In ピンへの接続を作成
							Pin* srcPin = nullptr;
							for (auto& p : srcNode->outputs) {
								if (p.name == key) {
									srcPin = &p;
									break;
								}
							}
							if (srcPin) {
								m_Links.push_back(Link(m_NextId++, srcPin->id, targetNode->inputs[0].id));
							}
						}
					}
				}
			}
		}
	}

	// 各ノードの配置を初期位置で整列
	ImVec2 initialPos(100.0f, 100.0f);
	for (const auto& node : m_Nodes) {
		ed::SetNodePosition(node->id, initialPos);
		initialPos.x += 250.0f;
		if (initialPos.x > 800.0f) {
			initialPos.x = 100.0f;
			initialPos.y += 300.0f;
		}
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
	// SceneIO に類似したロード処理
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
	}
}
