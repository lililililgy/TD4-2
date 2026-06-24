#include "UIHierarchySystem.h"

/// engine
#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIGroupComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIElementComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UILinkNavigationComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Script/Script.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/ECS/Entity/Collection/EntityCollection.h"
#include "Engine/ECS/Entity/Prefab/EntityPrefab.h"

/// std
#include <unordered_map>
#include <unordered_set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace ONEngine;

UIHierarchySystem::UIHierarchySystem() = default;
UIHierarchySystem::~UIHierarchySystem() = default;

void UIHierarchySystem::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
	UpdateUIHierarchy(ecs);
}

void UIHierarchySystem::RuntimeUpdate(ECSGroup* ecs) {
	UpdateUIHierarchy(ecs);
}

namespace {
// プレハブJSONからUIElementのelementIdとguid（古いGUID）を収集するヘルパー
void CollectOldGuids(const json& entityJson, std::unordered_map<std::string, Guid>& outMap) {
	if (entityJson.is_object()) {
		std::string guidStr = entityJson.value("guid", "");
		if (!guidStr.empty() && entityJson.contains("components") && entityJson["components"].is_array()) {
			for (const auto& comp : entityJson["components"]) {
				if (comp.is_object() && comp.value("type", "") == "UIElementComponent") {
					std::string elemId = comp.value("elementId", "");
					if (!elemId.empty()) {
						outMap[elemId] = Guid::FromString(guidStr);
					}
				}
			}
		}
		if (entityJson.contains("children") && entityJson["children"].is_array()) {
			for (const auto& child : entityJson["children"]) {
				CollectOldGuids(child, outMap);
			}
		}
	}
}
} // namespace

void UIHierarchySystem::UpdateUIHierarchy(ECSGroup* ecs) {
	// 1. UIElementComponent のグループ参照 (GUID) を解決する
	ComponentArray<UIElementComponent>* elementArray = ecs->GetComponentArray<UIElementComponent>();
	if (elementArray) {
		for (auto& elem : elementArray->GetUsedComponents()) {
			if (elem->groupId.CheckValid() && !elem->groupEntity) {
				elem->groupEntity = ecs->GetEntityFromGuid(elem->groupId);
				if (elem->groupEntity) {
					ONEngine::Console::Log(std::format("[UI Hierarchy] Resolved element '{}' parent group via GUID: '{}'", 
						elem->GetOwner() ? elem->GetOwner()->GetName() : "Unknown", elem->groupEntity->GetName()));
				}
			}

			// フォールバック: GUIDでグループが解決できなかった場合（プレハブインスタンス化時のGUID書き換えなど）、シーン階層（親子関係）から親のUIGroupComponentを持つEntityを探す
			if (!elem->groupEntity && elem->GetOwner()) {
				GameEntity* parent = elem->GetOwner()->GetParent();
				while (parent) {
					if (parent->GetComponent<UIGroupComponent>()) {
						elem->groupEntity = parent;
						// groupIdも新しいインスタンスのGUIDに修復
						elem->groupId = parent->GetGuid();
						ONEngine::Console::Log(std::format("[UI Hierarchy] Resolved element '{}' parent group via Scene Hierarchy: '{}' (Auto-fixed groupId)", 
							elem->GetOwner()->GetName(), parent->GetName()));
						break;
					}
					parent = parent->GetParent();
				}
			}
		}
	}

	// 2. UIGroupComponent の参照解決と状態の監視
	ComponentArray<UIGroupComponent>* groupArray = ecs->GetComponentArray<UIGroupComponent>();
	if (!groupArray) return;

	for (auto& groupComp : groupArray->GetUsedComponents()) {
		GameEntity* groupOwner = groupComp->GetOwner();
		if (!groupOwner) continue;

		// GUID参照の解決
		if (groupComp->currentSelectedGuid.CheckValid() && !groupComp->currentSelected) {
			groupComp->currentSelected = ecs->GetEntityFromGuid(groupComp->currentSelectedGuid);
			if (groupComp->currentSelected) {
				ONEngine::Console::Log(std::format("[UI Hierarchy] Resolved currentSelected: '{}' (GUID: {}) for Group '{}'", 
					groupComp->currentSelected->GetName(), groupComp->currentSelectedGuid.ToString(), groupOwner->GetName()));
			}
		}
		if (groupComp->parentGroupGuid.CheckValid() && !groupComp->parentGroup) {
			groupComp->parentGroup = ecs->GetEntityFromGuid(groupComp->parentGroupGuid);
			if (groupComp->parentGroup) {
				ONEngine::Console::Log(std::format("[UI Hierarchy] Resolved parentGroup: '{}' for Group '{}'", 
					groupComp->parentGroup->GetName(), groupOwner->GetName()));
			}
		}

		// フォールバック: 初期選択が指定されていない場合、グループ内の elementIndex が最も小さい要素を自動設定する
		if (!groupComp->currentSelected && elementArray) {
			GameEntity* bestElement = nullptr;
			int32_t minIndex = INT32_MAX;

			for (auto& elem : elementArray->GetUsedComponents()) {
				if (elem->groupEntity == groupOwner) {
					if (elem->elementIndex < minIndex) {
						minIndex = elem->elementIndex;
						bestElement = elem->GetOwner();
					}
				}
			}

			if (bestElement) {
				groupComp->currentSelected = bestElement;
				groupComp->currentSelectedGuid = bestElement->GetGuid();
				ONEngine::Console::Log(std::format("[UI Hierarchy] Fallback: No currentSelected specified for Group '{}'. Automatically selected element with lowest index: '{}' (Index: {})", 
					groupOwner->GetName(), bestElement->GetName(), minIndex));
			} else {
				// 毎フレームの警告ログのスパムを防ぐため、初回のみまたは警告を一度だけにする対策として、ここでは出力しない（または警告レベルを下げる）
				static std::unordered_set<Guid> warnedGroups;
				if (!warnedGroups.count(groupOwner->GetGuid())) {
					ONEngine::Console::LogWarning(std::format("[UI Hierarchy] Fallback failed: No elements found belonging to Group '{}'", groupOwner->GetName()));
					warnedGroups.insert(groupOwner->GetGuid());
				}
			}
		}

		// 表示・非表示 (isVisible) の状態変化を反映
		if (groupComp->isVisible != groupComp->wasVisible) {
			groupOwner->active = groupComp->isVisible;

			// グループ内の全要素の active 状態を同期
			if (elementArray) {
				for (auto& elem : elementArray->GetUsedComponents()) {
					if (elem->groupEntity == groupOwner) {
						if (GameEntity* elemOwner = elem->GetOwner()) {
							elemOwner->active = groupComp->isVisible;
						}
					}
				}
			}
			groupComp->wasVisible = groupComp->isVisible;
		}

		// フォーカス (isFocused) の状態変化を反映
		if (groupComp->isFocused != groupComp->wasFocused) {
			// グループ内の全要素のスクリプト有効状態を同期
			if (elementArray) {
				for (auto& elem : elementArray->GetUsedComponents()) {
					if (elem->groupEntity == groupOwner) {
						if (GameEntity* elemOwner = elem->GetOwner()) {
							if (Script* script = elemOwner->GetComponent<Script>()) {
								for (auto& data : script->GetScriptDataList()) {
									script->SetEnable(data.scriptName, groupComp->isFocused);
								}
							}
						}
					}
				}
			}

			// グループ管理エンティティ自身のスクリプト状態も同期
			if (Script* groupScript = groupOwner->GetComponent<Script>()) {
				for (auto& data : groupScript->GetScriptDataList()) {
					groupScript->SetEnable(data.scriptName, groupComp->isFocused);
				}
			}

			groupComp->wasFocused = groupComp->isFocused;
		}
	}

	// 3. プレハブインスタンス化時の GUID リマップ (親子関係解決後)
	for (auto& groupComp : groupArray->GetUsedComponents()) {
		GameEntity* groupOwner = groupComp->GetOwner();
		if (!groupOwner) continue;

		static std::unordered_set<Guid> remappedGroups;
		if (remappedGroups.count(groupOwner->GetGuid())) continue;

		std::string prefabName = groupOwner->GetPrefabName();
		if (prefabName.empty()) {
			// 親グループのPrefab名も探す
			GameEntity* p = groupOwner->GetParent();
			while (p) {
				prefabName = p->GetPrefabName();
				if (!prefabName.empty()) break;
				p = p->GetParent();
			}
		}

		// フォールバック: それでも空の場合、ヒエラルキーの最上位（ルート）エンティティの名前からPrefab名を推測する
		if (prefabName.empty()) {
			GameEntity* root = groupOwner;
			while (root->GetParent()) {
				root = root->GetParent();
			}
			if (root) {
				std::string rootName = root->GetName();
				// クローン名の除去
				size_t clonePos = rootName.find("(Clone)");
				if (clonePos != std::string::npos) {
					rootName = rootName.substr(0, clonePos);
				}
				prefabName = rootName + ".prefab";
				ONEngine::Console::Log(std::format("[UI Hierarchy] Guessing prefab name from root entity name: '{}' -> '{}'", root->GetName(), prefabName));
			}
		}

		if (prefabName.empty()) continue;

		// プレハブを取得
		auto* collection = ecs->GetEntityCollection();
		auto* prefab = collection ? collection->GetPrefab(prefabName) : nullptr;
		if (!prefab) continue;

		// A. プレハブ内の elementId -> 旧Guid
		std::unordered_map<std::string, Guid> elementIdToOldGuid;
		CollectOldGuids(prefab->GetJson(), elementIdToOldGuid);

		// B. 現在のシーンインスタンス内の elementId -> 新Guid
		std::unordered_map<std::string, Guid> elementIdToNewGuid;
		if (elementArray) {
			for (auto& elem : elementArray->GetUsedComponents()) {
				if (elem->groupEntity == groupOwner) {
					if (GameEntity* owner = elem->GetOwner()) {
						elementIdToNewGuid[elem->elementId] = owner->GetGuid();
					}
				}
			}
		}

		// C. 旧Guid -> 新Guid の対応マップ作成
		std::unordered_map<Guid, Guid> oldToNewMap;
		for (const auto& pair : elementIdToOldGuid) {
			const std::string& elemId = pair.first;
			Guid oldGuid = pair.second;
			if (elementIdToNewGuid.count(elemId)) {
				oldToNewMap[oldGuid] = elementIdToNewGuid[elemId];
			}
		}

		// D. UILinkNavigationComponent の links をリマップして書き換える
		bool remappedAny = false;
		if (elementArray) {
			for (auto& elem : elementArray->GetUsedComponents()) {
				if (elem->groupEntity == groupOwner) {
					if (GameEntity* owner = elem->GetOwner()) {
						if (auto* nav = owner->GetComponent<UILinkNavigationComponent>()) {
							bool updated = false;
							for (auto& linkPair : nav->links) {
								Guid oldTarget = linkPair.second;
								if (oldToNewMap.count(oldTarget)) {
									linkPair.second = oldToNewMap[oldTarget];
									updated = true;
								}
							}
							if (updated) {
								remappedAny = true;
								ONEngine::Console::Log(std::format("[UI Hierarchy] Remapped navigation links for element '{}' in Group '{}'", owner->GetName(), groupOwner->GetName()));
							}
						}
					}
				}
			}
		}

		if (remappedAny) {
			remappedGroups.insert(groupOwner->GetGuid());
		}
	}
}
