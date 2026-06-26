#include "UIGroupComponent.h"

/// external
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <algorithm>

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

	// 決定キー（submitKeys）の編集UI
	ImGui::Separator();
	ImGui::Text("Submit Keys (Decision Keys):");
	
	// カンマ区切りの文字列として編集できるようにする
	std::string keysStr = "";
	for (size_t i = 0; i < comp->submitKeys.size(); ++i) {
		keysStr += comp->submitKeys[i];
		if (i + 1 < comp->submitKeys.size()) {
			keysStr += ", ";
		}
	}
	
	char buf[256];
	strncpy(buf, keysStr.c_str(), sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';
	
	if (ImGui::InputText("Keys (comma separated)", buf, sizeof(buf))) {
		std::vector<std::string> newKeys;
		std::string raw(buf);
		size_t pos = 0;
		while ((pos = raw.find(",")) != std::string::npos) {
			std::string token = raw.substr(0, pos);
			// 前後の空白トリム
			token.erase(0, token.find_first_not_of(" \t"));
			token.erase(token.find_last_not_of(" \t") + 1);
			if (!token.empty()) {
				newKeys.push_back(token);
			}
			raw.erase(0, pos + 1);
		}
		// 最後のトークン
		raw.erase(0, raw.find_first_not_of(" \t"));
		raw.erase(raw.find_last_not_of(" \t") + 1);
		if (!raw.empty()) {
			newKeys.push_back(raw);
		}
		comp->submitKeys = newKeys;
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

	// submitKeys の読み込み
	if (j.contains("submitKeys") && j["submitKeys"].is_array()) {
		c.submitKeys = j["submitKeys"].get<std::vector<std::string>>();
	} else {
		c.submitKeys = { "Return", "Space", "GamepadA" }; // デフォルト値
	}
}

void ONEngine::to_json(nlohmann::json& j, const UIGroupComponent& c) {
	j = {
		{ "type", "UIGroupComponent" },
		{ "enable", c.enable },
		{ "isFocused", c.isFocused },
		{ "isVisible", c.isVisible },
		{ "currentSelected", c.currentSelectedGuid.CheckValid() ? c.currentSelectedGuid.ToString() : "" },
		{ "parentGroup", c.parentGroupGuid.CheckValid() ? c.parentGroupGuid.ToString() : "" },
		{ "submitKeys", c.submitKeys }
	};
}
