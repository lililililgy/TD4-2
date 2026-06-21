#include "UILinkNavigationComponent.h"

/// external
#include <imgui.h>

/// engine
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"

using namespace ONEngine;

namespace {

int32_t ParseKeyCodeString(const std::string& keyStr) {
	if (keyStr == "KEY_UP" || keyStr == "UpArrow") return 0xC8; // UpArrow (DIK_UP = 0xC8)
	if (keyStr == "KEY_DOWN" || keyStr == "DownArrow") return 0xD0; // DownArrow (DIK_DOWN = 0xD0)
	if (keyStr == "KEY_LEFT" || keyStr == "LeftArrow") return 0xCB; // LeftArrow (DIK_LEFT = 0xCB)
	if (keyStr == "KEY_RIGHT" || keyStr == "RightArrow") return 0xCD; // RightArrow (DIK_RIGHT = 0xCD)
	if (keyStr == "Return" || keyStr == "Enter") return 0x1C;
	if (keyStr == "Space") return 0x39;
	if (keyStr == "W") return 0x11;
	if (keyStr == "S") return 0x1F;
	if (keyStr == "A") return 0x1E;
	if (keyStr == "D") return 0x20;

	try {
		if (keyStr.rfind("0x", 0) == 0) {
			return std::stoi(keyStr, nullptr, 16);
		}
		return std::stoi(keyStr);
	} catch (...) {
		return 0; // invalid/unsupported
	}
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

	char buf[32];
	sprintf_s(buf, "0x%02X", keyCode);
	return std::string(buf);
}

} // namespace

void ComponentDebug::UILinkNavigationComponentDebug(UILinkNavigationComponent* comp) {
	if (!comp) return;

	ImGui::Text("Links:");
	for (auto& pair : comp->links) {
		std::string keyName = KeyCodeToString(pair.first);
		std::string targetGuid = pair.second.ToString();
		ImGui::BulletText("%s -> %s", keyName.c_str(), targetGuid.c_str());
	}
}

void ONEngine::from_json(const nlohmann::json& j, UILinkNavigationComponent& c) {
	c.enable = j.value("enable", static_cast<int>(true));
	c.links.clear();

	if (j.contains("links") && j.at("links").is_object()) {
		auto linksObj = j.at("links");
		for (auto it = linksObj.begin(); it != linksObj.end(); ++it) {
			int32_t keyCode = ParseKeyCodeString(it.key());
			std::string targetStr = it.value().get<std::string>();
			Guid targetGuid = targetStr.empty() ? Guid::kInvalid : Guid::FromString(targetStr);
			if (keyCode != 0 && targetGuid.CheckValid()) {
				c.links[keyCode] = targetGuid;
			}
		}
	}
}

void ONEngine::to_json(nlohmann::json& j, const UILinkNavigationComponent& c) {
	nlohmann::json linksObj = nlohmann::json::object();
	for (const auto& pair : c.links) {
		std::string keyStr = KeyCodeToString(pair.first);
		linksObj[keyStr] = pair.second.ToString();
	}

	j = {
		{ "type", "UILinkNavigationComponent" },
		{ "enable", c.enable },
		{ "links", linksObj }
	};
}
