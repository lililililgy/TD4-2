#include "UIInputNavigationSystem.h"

/// engine
#include "Engine/Core/Utility/Input/Input.h"
#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIGroupComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIElementComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UILinkNavigationComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Script/Script.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/Script/MonoScriptEngine.h"

/// externals
#include <mono/jit/jit.h>

using namespace ONEngine;

namespace {

void InvokeScriptMethod(MonoObject* obj, const std::string& methodName, void** args = nullptr) {
	if (!obj) return;
	MonoClass* klass = mono_object_get_class(obj);
	int paramCount = (args != nullptr) ? 1 : 0;
	MonoMethod* method = mono_class_get_method_from_name(klass, methodName.c_str(), paramCount);
	if (method) {
		MonoObject* exc = nullptr;
		MonoScriptEngineUtils::SafeInvoke(method, obj, args, &exc);
		if (exc) {
			MonoScriptEngineUtils::HandleException(exc);
		}
	}
}

void NotifyFocusChange(ECSGroup* ecs, GameEntity* groupOwner, GameEntity* oldSelected, GameEntity* newSelected) {
	std::string groupName = ecs->GetGroupName();

	// 1. 古い選択要素のスクリプトへ通知
	if (oldSelected) {
		if (Script* scriptComp = oldSelected->GetComponent<Script>()) {
			for (const auto& name : scriptComp->GetScriptNames()) {
				MonoObject* obj = MonoScriptEngine::GetInstance().GetMonoBehaviorFromCS(groupName, oldSelected->GetId(), name);
				InvokeScriptMethod(obj, "OnDeselect");
			}
		}
	}

	// 2. 新しい選択要素のスクリプトへ通知
	if (newSelected) {
		if (Script* scriptComp = newSelected->GetComponent<Script>()) {
			for (const auto& name : scriptComp->GetScriptNames()) {
				MonoObject* obj = MonoScriptEngine::GetInstance().GetMonoBehaviorFromCS(groupName, newSelected->GetId(), name);
				InvokeScriptMethod(obj, "OnSelect");
			}
		}
	}

	// 3. 親グループの統括スクリプトへ通知 (一括制御)
	if (groupOwner) {
		if (Script* groupScriptComp = groupOwner->GetComponent<Script>()) {
			std::string elementId = "";
			if (newSelected) {
				if (auto* elemComp = newSelected->GetComponent<UIElementComponent>()) {
					elementId = elemComp->elementId;
				}
			}

			MonoString* monoStr = mono_string_new(mono_domain_get(), elementId.c_str());
			void* args[1];
			args[0] = monoStr;

			for (const auto& name : groupScriptComp->GetScriptNames()) {
				MonoObject* obj = MonoScriptEngine::GetInstance().GetMonoBehaviorFromCS(groupName, groupOwner->GetId(), name);
				InvokeScriptMethod(obj, "OnUISelect", args);
			}
		}
	}
}

void NotifySubmit(ECSGroup* ecs, GameEntity* groupOwner, GameEntity* selected) {
	if (!selected) return;
	std::string groupName = ecs->GetGroupName();

	// 1. 個別スクリプトの OnSubmit 呼び出し
	if (Script* scriptComp = selected->GetComponent<Script>()) {
		for (const auto& name : scriptComp->GetScriptNames()) {
			MonoObject* obj = MonoScriptEngine::GetInstance().GetMonoBehaviorFromCS(groupName, selected->GetId(), name);
			InvokeScriptMethod(obj, "OnSubmit");
		}
	}

	// 2. グループ統括スクリプトの OnUISubmit 呼び出し
	if (groupOwner) {
		if (Script* groupScriptComp = groupOwner->GetComponent<Script>()) {
			std::string elementId = "";
			if (auto* elemComp = selected->GetComponent<UIElementComponent>()) {
				elementId = elemComp->elementId;
			}

			MonoString* monoStr = mono_string_new(mono_domain_get(), elementId.c_str());
			void* args[1];
			args[0] = monoStr;

			for (const auto& name : groupScriptComp->GetScriptNames()) {
				MonoObject* obj = MonoScriptEngine::GetInstance().GetMonoBehaviorFromCS(groupName, groupOwner->GetId(), name);
				InvokeScriptMethod(obj, "OnUISubmit", args);
			}
		}
	}
}

} // namespace

UIInputNavigationSystem::UIInputNavigationSystem() = default;
UIInputNavigationSystem::~UIInputNavigationSystem() = default;

void UIInputNavigationSystem::OutsideOfRuntimeUpdate(ECSGroup*) {
	// エディット中はキーボードによるUIナビゲーションは行わない
}

void UIInputNavigationSystem::RuntimeUpdate(ECSGroup* ecs) {
	ProcessInputNavigation(ecs);
}

void UIInputNavigationSystem::ProcessInputNavigation(ECSGroup* ecs) {
	ComponentArray<UIGroupComponent>* groupArray = ecs->GetComponentArray<UIGroupComponent>();
	if (!groupArray) return;

	for (auto& groupComp : groupArray->GetUsedComponents()) {
		if (!groupComp->isFocused || !groupComp->isVisible) continue;

		GameEntity* selected = groupComp->currentSelected;
		if (!selected) continue;

		// 1. 決定・送信キーのチェック
		// DIK_RETURN (0x1C) または DIK_SPACE (0x39)
		if (Input::TriggerKey(0x1C) || Input::TriggerKey(0x39) || Input::TriggerGamepad(0)) { // Gamepad A Button as 0
			NotifySubmit(ecs, groupComp->GetOwner(), selected);
			continue;
		}

		// 2. ナビゲーションリンクの遷移チェック
		if (UILinkNavigationComponent* nav = selected->GetComponent<UILinkNavigationComponent>()) {
			for (const auto& pair : nav->links) {
				int32_t keyCode = pair.first;
				const Guid& targetGuid = pair.second;

				// キー入力またはゲームパッドボタンが押されたかチェック
				bool triggered = false;
				if (keyCode < 256) {
					triggered = Input::TriggerKey(keyCode);
				} else {
					// ゲームパッドや特殊コード
					triggered = Input::TriggerGamepad(keyCode - 256);
				}

				if (triggered) {
					if (GameEntity* target = ecs->GetEntityFromGuid(targetGuid)) {
						// 選択の更新
						groupComp->currentSelected = target;
						groupComp->currentSelectedGuid = targetGuid;

						// イベント通知
						NotifyFocusChange(ecs, groupComp->GetOwner(), selected, target);
						break; // 一度の入力につき1つの遷移のみ
					}
				}
			}
		}
	}
}
