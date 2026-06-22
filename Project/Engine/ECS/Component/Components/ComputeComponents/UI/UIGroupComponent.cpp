#include "UIGroupComponent.h"

/// external
#include <imgui.h>

/// engine
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"

using namespace ONEngine;

void ComponentDebug::UIGroupComponentDebug(UIGroupComponent* comp) {
	if (!comp) return;
	ImGui::Checkbox("Is Focused", &comp->isFocused);
	ImGui::Checkbox("Is Visible", &comp->isVisible);

	std::string selGuidStr = comp->currentSelectedGuid.ToString();
	ImGui::Text("Current Selected Guid: %s", selGuidStr.c_str());
	if (comp->currentSelected) {
		ImGui::Text("Current Selected Name: %s", comp->currentSelected->GetName().c_str());
	} else {
		ImGui::Text("Current Selected Name: None");
	}

	std::string parentGuidStr = comp->parentGroupGuid.ToString();
	ImGui::Text("Parent Group Guid: %s", parentGuidStr.c_str());
	if (comp->parentGroup) {
		ImGui::Text("Parent Group Name: %s", comp->parentGroup->GetName().c_str());
	} else {
		ImGui::Text("Parent Group Name: None");
	}
}

void ONEngine::from_json(const nlohmann::json& j, UIGroupComponent& c) {
	c.enable = j.value("enable", static_cast<int>(true));
	c.isFocused = j.value("isFocused", false);
	c.isVisible = j.value("isVisible", true);

	std::string selGuidStr = j.value("currentSelected", "");
	c.currentSelectedGuid = selGuidStr.empty() ? Guid::kInvalid : Guid::FromString(selGuidStr);

	std::string parentGuidStr = j.value("parentGroup", "");
	c.parentGroupGuid = parentGuidStr.empty() ? Guid::kInvalid : Guid::FromString(parentGuidStr);
}

void ONEngine::to_json(nlohmann::json& j, const UIGroupComponent& c) {
	j = {
		{ "type", "UIGroupComponent" },
		{ "enable", c.enable },
		{ "isFocused", c.isFocused },
		{ "isVisible", c.isVisible },
		{ "currentSelected", c.currentSelectedGuid.CheckValid() ? c.currentSelectedGuid.ToString() : "" },
		{ "parentGroup", c.parentGroupGuid.CheckValid() ? c.parentGroupGuid.ToString() : "" }
	};
}
