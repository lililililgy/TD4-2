#include "UIHierarchySystem.h"

/// engine
#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIGroupComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIElementComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Script/Script.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"

using namespace ONEngine;

UIHierarchySystem::UIHierarchySystem() = default;
UIHierarchySystem::~UIHierarchySystem() = default;

void UIHierarchySystem::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
	UpdateUIHierarchy(ecs);
}

void UIHierarchySystem::RuntimeUpdate(ECSGroup* ecs) {
	UpdateUIHierarchy(ecs);
}

void UIHierarchySystem::UpdateUIHierarchy(ECSGroup* ecs) {
	// 1. UIElementComponent のグループ参照 (GUID) を解決する
	ComponentArray<UIElementComponent>* elementArray = ecs->GetComponentArray<UIElementComponent>();
	if (elementArray) {
		for (auto& elem : elementArray->GetUsedComponents()) {
			if (elem->groupId.CheckValid() && !elem->groupEntity) {
				elem->groupEntity = ecs->GetEntityFromGuid(elem->groupId);
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
		}
		if (groupComp->parentGroupGuid.CheckValid() && !groupComp->parentGroup) {
			groupComp->parentGroup = ecs->GetEntityFromGuid(groupComp->parentGroupGuid);
		}

		// 表示・非表示 (isVisible) の状態変化を反映
		if (groupComp->isVisible != groupComp->wasVisible) {
			groupOwner->active = groupComp->isVisible;

			// グループ内の全要素の active 状態を同期
			if (elementArray) {
				for (auto& elem : elementArray->GetUsedComponents()) {
					if (elem->groupId == groupOwner->GetGuid()) {
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
					if (elem->groupId == groupOwner->GetGuid()) {
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
}
