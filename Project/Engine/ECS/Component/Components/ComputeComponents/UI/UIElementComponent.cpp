#include "UIElementComponent.h"

/// external
#include <imgui.h>

/// engine
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"

using namespace ONEngine;

void ComponentDebug::UIElementComponentDebug(UIElementComponent* comp) {
	if (!comp) return;

	std::string groupGuidStr = comp->groupId.ToString();
	ImGui::Text("Group Guid: %s", groupGuidStr.c_str());
	if (comp->groupEntity) {
		ImGui::Text("Group Entity Name: %s", comp->groupEntity->GetName().c_str());
	} else {
		ImGui::Text("Group Entity Name: None");
	}

	char elementIdBuf[256];
	strncpy_s(elementIdBuf, comp->elementId.c_str(), sizeof(elementIdBuf));
	if (ImGui::InputText("Element ID", elementIdBuf, sizeof(elementIdBuf))) {
		comp->elementId = elementIdBuf;
	}

	ImGui::InputInt("Element Index", &comp->elementIndex);
}

void ONEngine::from_json(const nlohmann::json& j, UIElementComponent& c) {
	c.enable = j.value("enable", static_cast<int>(true));
	
	std::string groupGuidStr = j.value("groupId", "");
	c.groupId = groupGuidStr.empty() ? Guid::kInvalid : Guid::FromString(groupGuidStr);

	c.elementId = j.value("elementId", "");
	c.elementIndex = j.value("elementIndex", 0);
}

void ONEngine::to_json(nlohmann::json& j, const UIElementComponent& c) {
	j = {
		{ "type", "UIElementComponent" },
		{ "enable", c.enable },
		{ "groupId", c.groupId.CheckValid() ? c.groupId.ToString() : "" },
		{ "elementId", c.elementId },
		{ "elementIndex", c.elementIndex }
	};
}
