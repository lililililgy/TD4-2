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
	ONEngine::Console::Log(std::format("[UI Navigation] Focus Change: Group='{}', OldSelected='{}', NewSelected='{}'", 
		groupOwner ? groupOwner->GetName() : "None", 
		oldSelected ? oldSelected->GetName() : "None", 
		newSelected ? newSelected->GetName() : "None"));

	// 1. 古い選択要素のスクリプトへ通知
	if (oldSelected) {
		if (Script* scriptComp = oldSelected->GetComponent<Script>()) {
			for (const auto& name : scriptComp->GetScriptNames()) {
				MonoObject* obj = MonoScriptEngine::GetInstance().GetMonoBehaviorFromCS(groupName, oldSelected->GetId(), name);
				if (obj) {
					ONEngine::Console::Log(std::format("[UI Navigation]   -> Calling OnDeselect() on '{}' (Script: '{}')", oldSelected->GetName(), name));
					InvokeScriptMethod(obj, "OnDeselect");
				} else {
					ONEngine::Console::LogWarning(std::format("[UI Navigation]   -> OnDeselect: Failed to get MonoObject for '{}' (Script: '{}')", oldSelected->GetName(), name));
				}
			}
		}
	}

	// 2. 新しい選択要素のスクリプトへ通知
	if (newSelected) {
		if (Script* scriptComp = newSelected->GetComponent<Script>()) {
			for (const auto& name : scriptComp->GetScriptNames()) {
				MonoObject* obj = MonoScriptEngine::GetInstance().GetMonoBehaviorFromCS(groupName, newSelected->GetId(), name);
				if (obj) {
					ONEngine::Console::Log(std::format("[UI Navigation]   -> Calling OnSelect() on '{}' (Script: '{}')", newSelected->GetName(), name));
					InvokeScriptMethod(obj, "OnSelect");
				} else {
					ONEngine::Console::LogWarning(std::format("[UI Navigation]   -> OnSelect: Failed to get MonoObject for '{}' (Script: '{}')", newSelected->GetName(), name));
				}
			}
		} else {
			ONEngine::Console::Log(std::format("[UI Navigation]   -> NewSelected '{}' has no Script component attached.", newSelected->GetName()));
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
				if (obj) {
					ONEngine::Console::Log(std::format("[UI Navigation]   -> Calling OnUISelect(elementId='{}') on Group '{}' (Script: '{}')", elementId, groupOwner->GetName(), name));
					InvokeScriptMethod(obj, "OnUISelect", args);
				} else {
					ONEngine::Console::LogWarning(std::format("[UI Navigation]   -> OnUISelect: Failed to get MonoObject for Group '{}' (Script: '{}')", groupOwner->GetName(), name));
				}
			}
		}
	}
}

void NotifySubmit(ECSGroup* ecs, GameEntity* groupOwner, GameEntity* selected) {
	if (!selected) return;
	std::string groupName = ecs->GetGroupName();
	ONEngine::Console::Log(std::format("[UI Navigation] Submit Event: Element='{}' in Group='{}'", selected->GetName(), groupOwner ? groupOwner->GetName() : "None"));

	// 1. 個別スクリプトの OnSubmit 呼び出し
	if (Script* scriptComp = selected->GetComponent<Script>()) {
		for (const auto& name : scriptComp->GetScriptNames()) {
			MonoObject* obj = MonoScriptEngine::GetInstance().GetMonoBehaviorFromCS(groupName, selected->GetId(), name);
			if (obj) {
				ONEngine::Console::Log(std::format("[UI Navigation]   -> Calling OnSubmit() on '{}' (Script: '{}')", selected->GetName(), name));
				InvokeScriptMethod(obj, "OnSubmit");
			} else {
				ONEngine::Console::LogWarning(std::format("[UI Navigation]   -> OnSubmit: Failed to get MonoObject for '{}' (Script: '{}')", selected->GetName(), name));
			}
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
				if (obj) {
					ONEngine::Console::Log(std::format("[UI Navigation]   -> Calling OnUISubmit(elementId='{}') on Group '{}' (Script: '{}')", elementId, groupOwner->GetName(), name));
					InvokeScriptMethod(obj, "OnUISubmit", args);
				} else {
					ONEngine::Console::LogWarning(std::format("[UI Navigation]   -> OnUISubmit: Failed to get MonoObject for Group '{}' (Script: '{}')", groupOwner->GetName(), name));
				}
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

#include <unordered_map>
#include <unordered_set>

namespace {
// 各グループの前回選択されていた要素のGUIDを追跡するマップ
// (グループGUID -> 選択要素GUID)
std::unordered_map<Guid, Guid> g_LastSelectedMap;
}

void UIInputNavigationSystem::ProcessInputNavigation(ECSGroup* ecs) {
	ComponentArray<UIGroupComponent>* groupArray = ecs->GetComponentArray<UIGroupComponent>();
	if (!groupArray) return;

	for (auto& groupComp : groupArray->GetUsedComponents()) {
		GameEntity* groupOwner = groupComp->GetOwner();
		Guid groupGuid = groupOwner ? groupOwner->GetGuid() : Guid::kInvalid;

		if (!groupComp->isFocused || !groupComp->isVisible) {
			if (groupGuid.CheckValid()) {
				if (g_LastSelectedMap.count(groupGuid)) {
					ONEngine::Console::Log(std::format("[UI Navigation] Group '{}' lost focus or became invisible.", groupOwner ? groupOwner->GetName() : "Unknown"));
					g_LastSelectedMap.erase(groupGuid);
				}
			}
			continue;
		}

		GameEntity* selected = groupComp->currentSelected;
		if (!selected) {
			// 毎フレームログが出るとうるさいので、状態変化時のみ警告を出すなどの工夫ができますが、ここでは選択なしの状態を通知
			static std::unordered_set<Guid> warnedGroups;
			if (groupGuid.CheckValid() && !warnedGroups.count(groupGuid)) {
				ONEngine::Console::LogWarning(std::format("[UI Navigation] Group '{}' is active but has no currentSelected!", groupOwner ? groupOwner->GetName() : "Unknown"));
				warnedGroups.insert(groupGuid);
			}
			continue;
		}

		// 初回フォーカス時、または外部から直接選択要素が切り替えられた場合の OnSelect 自動発火
		if (groupGuid.CheckValid()) {
			auto it = g_LastSelectedMap.find(groupGuid);
			if (it == g_LastSelectedMap.end()) {
				ONEngine::Console::Log(std::format("[UI Navigation] Group '{}' gained focus. Initial selection: '{}' (GUID: {})", 
					groupOwner->GetName(), selected->GetName(), selected->GetGuid().ToString()));
				g_LastSelectedMap[groupGuid] = selected->GetGuid();
				NotifyFocusChange(ecs, groupOwner, nullptr, selected);
			} else if (it->second != selected->GetGuid()) {
				ONEngine::Console::Log(std::format("[UI Navigation] Group '{}' selection changed externally to '{}'", 
					groupOwner->GetName(), selected->GetName()));
				GameEntity* oldSelected = ecs->GetEntityFromGuid(it->second);
				it->second = selected->GetGuid();
				NotifyFocusChange(ecs, groupOwner, oldSelected, selected);
			}
		}

		// 1. 決定・送信キーのチェック
		// UIGroupComponent の submitKeys に登録されているキーが押されたかチェック
		bool submitTriggered = false;
		int32_t triggeredSubmitKey = 0;

		for (const auto& keyName : groupComp->submitKeys) {
			int32_t keyCode = UILinkNavigationComponent::ParseKeyCodeString(keyName);
			if (keyCode != 0) {
				bool pressed = false;
				if (keyCode < 256) {
					pressed = Input::TriggerKey(keyCode);
				} else {
					pressed = Input::TriggerGamepad(keyCode - 256);
				}

				if (pressed) {
					submitTriggered = true;
					triggeredSubmitKey = keyCode;
					break;
				}
			}
		}

		if (submitTriggered) {
			// もし選択中のエレメントが、そのキーに対応する明示的なナビゲーションリンクを持っているなら、
			// ここでの決定（Submit）処理はスキップして、後ろのナビゲーションリンク遷移に任せる
			bool hasExplicitLink = false;
			if (UILinkNavigationComponent* nav = selected->GetComponent<UILinkNavigationComponent>()) {
				if (nav->links.count(triggeredSubmitKey)) {
					hasExplicitLink = true;
				}
			}

			if (!hasExplicitLink) {
				ONEngine::Console::Log(std::format("[UI Navigation] Group Submit Key Triggered on element '{}'", selected->GetName()));
				NotifySubmit(ecs, groupOwner, selected);
				continue;
			}
		}

		// 2. ナビゲーションリンク of 遷移チェック
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
					std::string keyName = UILinkNavigationComponent::KeyCodeToString(keyCode);
					ONEngine::Console::Log(std::format("[UI Navigation] Input Detected: Key='{}' ({}) on element '{}'", 
						keyName, keyCode, selected->GetName()));

					if (GameEntity* target = ecs->GetEntityFromGuid(targetGuid)) {
						ONEngine::Console::Log(std::format("[UI Navigation] Transitioning focus: '{}' -> '{}'", 
							selected->GetName(), target->GetName()));

						// 選択の更新
						groupComp->currentSelected = target;
						groupComp->currentSelectedGuid = targetGuid;

						// 追跡マップを更新して二重発火を防止
						if (groupGuid.CheckValid()) {
							g_LastSelectedMap[groupGuid] = targetGuid;
						}

						// イベント通知
						NotifyFocusChange(ecs, groupOwner, selected, target);
						break; // 一度の入力につき1つの遷移のみ
					} else {
						ONEngine::Console::LogError(std::format("[UI Navigation] Transition target GUID '{}' not found in active scene!", 
							targetGuid.ToString()));
					}
				}
			}
		}
	}
}
