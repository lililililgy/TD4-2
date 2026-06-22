#include "UILinkNavigationComponent.h"

/// external
#include <imgui.h>
#include <unordered_map>
#include <algorithm>
#include <sstream>

/// engine
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"

using namespace ONEngine;

namespace {

const std::unordered_map<std::string, int32_t> g_KeyMap = {
	// Top row
	{"Escape", 0x01}, {"Digit1", 0x02}, {"Digit2", 0x03}, {"Digit3", 0x04}, {"Digit4", 0x05},
	{"Digit5", 0x06}, {"Digit6", 0x07}, {"Digit7", 0x08}, {"Digit8", 0x09}, {"Digit9", 0x0A},
	{"Digit0", 0x0B}, {"Minus", 0x0C}, {"Equals", 0x0D}, {"Backspace", 0x0E},
	// Tab row
	{"Tab", 0x0F}, {"Q", 0x10}, {"W", 0x11}, {"E", 0x12}, {"R", 0x13},
	{"T", 0x14}, {"Y", 0x15}, {"U", 0x16}, {"I", 0x17}, {"O", 0x18},
	{"P", 0x19}, {"LeftBracket", 0x1A}, {"RightBracket", 0x1B}, {"Return", 0x1C}, {"Enter", 0x1C},
	// Modifier row
	{"LeftControl", 0x1D}, {"A", 0x1E}, {"S", 0x1F}, {"D", 0x20}, {"F", 0x21},
	{"G", 0x22}, {"H", 0x23}, {"J", 0x24}, {"K", 0x25}, {"L", 0x26},
	{"Semicolon", 0x27}, {"Apostrophe", 0x28}, {"Grave", 0x29}, {"LeftShift", 0x2A}, {"Backslash", 0x2B},
	// Bottom row
	{"Z", 0x2C}, {"X", 0x2D}, {"C", 0x2E}, {"V", 0x2F}, {"B", 0x30},
	{"N", 0x31}, {"M", 0x32}, {"Comma", 0x33}, {"Period", 0x34}, {"Slash", 0x35},
	{"RightShift", 0x36}, {"KeypadMultiply", 0x37}, {"LeftAlt", 0x38}, {"Space", 0x39}, {"CapsLock", 0x3A},
	// Function keys
	{"F1", 0x3B}, {"F2", 0x3C}, {"F3", 0x3D}, {"F4", 0x3E}, {"F5", 0x3F},
	{"F6", 0x40}, {"F7", 0x41}, {"F8", 0x42}, {"F9", 0x43}, {"F10", 0x44},
	{"F11", 0x57}, {"F12", 0x58},
	// Navigation / Edit
	{"Insert", 0xD2}, {"Delete", 0xD3}, {"Home", 0xC7}, {"End", 0xCF},
	{"PageUp", 0xC9}, {"PageDown", 0xD1},
	{"UpArrow", 0xC8}, {"KEY_UP", 0xC8},
	{"DownArrow", 0xD0}, {"KEY_DOWN", 0xD0},
	{"LeftArrow", 0xCB}, {"KEY_LEFT", 0xCB},
	{"RightArrow", 0xCD}, {"KEY_RIGHT", 0xCD},
	// Numpad
	{"NumLock", 0x45}, {"Keypad7", 0x47}, {"Keypad8", 0x48}, {"Keypad9", 0x49}, {"KeypadSubtract", 0x4A},
	{"Keypad4", 0x4B}, {"Keypad5", 0x4C}, {"Keypad6", 0x4D}, {"KeypadAdd", 0x4E}, {"Keypad1", 0x4F},
	{"Keypad2", 0x50}, {"Keypad3", 0x51}, {"Keypad0", 0x52}, {"KeypadDecimal", 0x53}, {"KeypadEnter", 0x9C},
	{"KeypadDivide", 0xB5}, {"KeypadEquals", 0x8D},
	// Others
	{"RightControl", 0x9D}, {"RightAlt", 0xB8}, {"LeftWindows", 0xDB}, {"RightWindows", 0xDC},
	{"Menu", 0xDD}, {"PrintScreen", 0xB7}, {"ScrollLock", 0x46}, {"Pause", 0xC5}, {"Apps", 0xDD},
	// Media / System
	{"VolumeMute", 0xA0}, {"VolumeDown", 0xAE}, {"VolumeUp", 0xB0}, {"MediaStop", 0xA4},
	{"MediaPlayPause", 0xA2}, {"NextTrack", 0x99}, {"Calculator", 0xA1}, {"WebHome", 0xB2},
	{"Power", 0xDE}, {"Sleep", 0xDF},
	// Gamepad Buttons (flag + 256)
	{"GamepadDPadUp", 257}, {"GamepadDPadDown", 258}, {"GamepadDPadLeft", 260}, {"GamepadDPadRight", 264},
	{"GamepadStart", 272}, {"GamepadBack", 288}, {"GamepadLeftThumb", 320}, {"GamepadRightThumb", 384},
	{"GamepadLeftShoulder", 512}, {"GamepadRightShoulder", 768}, {"GamepadA", 4352}, {"GamepadB", 8448},
	{"GamepadX", 16640}, {"GamepadY", 33024}
};

const std::unordered_map<int32_t, std::string> g_CodeMap = {
	// Top row
	{0x01, "Escape"}, {0x02, "Digit1"}, {0x03, "Digit2"}, {0x04, "Digit3"}, {0x05, "Digit4"},
	{0x06, "Digit5"}, {0x07, "Digit6"}, {0x08, "Digit7"}, {0x09, "Digit8"}, {0x0A, "Digit9"},
	{0x0B, "Digit0"}, {0x0C, "Minus"}, {0x0D, "Equals"}, {0x0E, "Backspace"},
	// Tab row
	{0x0F, "Tab"}, {0x10, "Q"}, {0x11, "W"}, {0x12, "E"}, {0x13, "R"},
	{0x14, "T"}, {0x15, "Y"}, {0x16, "U"}, {0x17, "I"}, {0x18, "O"},
	{0x19, "P"}, {0x1A, "LeftBracket"}, {0x1B, "RightBracket"}, {0x1C, "Return"},
	// Modifier row
	{0x1D, "LeftControl"}, {0x1E, "A"}, {0x1F, "S"}, {0x20, "D"}, {0x21, "F"},
	{0x22, "G"}, {0x23, "H"}, {0x24, "J"}, {0x25, "K"}, {0x26, "L"},
	{0x27, "Semicolon"}, {0x28, "Apostrophe"}, {0x29, "Grave"}, {0x2A, "LeftShift"}, {0x2B, "Backslash"},
	// Bottom row
	{0x2C, "Z"}, {0x2D, "X"}, {0x2E, "C"}, {0x2F, "V"}, {0x30, "B"},
	{0x31, "N"}, {0x32, "M"}, {0x33, "Comma"}, {0x34, "Period"}, {0x35, "Slash"},
	{0x36, "RightShift"}, {0x37, "KeypadMultiply"}, {0x38, "LeftAlt"}, {0x39, "Space"}, {0x3A, "CapsLock"},
	// Function keys
	{0x3B, "F1"}, {0x3C, "F2"}, {0x3D, "F3"}, {0x3E, "F4"}, {0x3F, "F5"},
	{0x40, "F6"}, {0x41, "F7"}, {0x42, "F8"}, {0x43, "F9"}, {0x44, "F10"},
	{0x57, "F11"}, {0x58, "F12"},
	// Navigation / Edit
	{0xD2, "Insert"}, {0xD3, "Delete"}, {0xC7, "Home"}, {0xCF, "End"},
	{0xC9, "PageUp"}, {0xD1, "PageDown"},
	{0xC8, "UpArrow"}, {0xD0, "DownArrow"}, {0xCB, "LeftArrow"}, {0xCD, "RightArrow"},
	// Numpad
	{0x45, "NumLock"}, {0x47, "Keypad7"}, {0x48, "Keypad8"}, {0x49, "Keypad9"}, {0x4A, "KeypadSubtract"},
	{0x4B, "Keypad4"}, {0x4C, "Keypad5"}, {0x4D, "Keypad6"}, {0x4E, "KeypadAdd"}, {0x4F, "Keypad1"},
	{0x50, "Keypad2"}, {0x51, "Keypad3"}, {0x52, "Keypad0"}, {0x53, "KeypadDecimal"}, {0x9C, "KeypadEnter"},
	{0xB5, "KeypadDivide"}, {0x8D, "KeypadEquals"},
	// Others
	{0x9D, "RightControl"}, {0xB8, "RightAlt"}, {0xDB, "LeftWindows"}, {0xDC, "RightWindows"},
	{0xDD, "Menu"}, {0xB7, "PrintScreen"}, {0x46, "ScrollLock"}, {0xC5, "Pause"},
	// Media / System
	{0xA0, "VolumeMute"}, {0xAE, "VolumeDown"}, {0xB0, "VolumeUp"}, {0xA4, "MediaStop"},
	{0xA2, "MediaPlayPause"}, {0x99, "NextTrack"}, {0xA1, "Calculator"}, {0xB2, "WebHome"},
	{0xDE, "Power"}, {0xDF, "Sleep"},
	// Gamepad Buttons (flag + 256)
	{257, "GamepadDPadUp"}, {258, "GamepadDPadDown"}, {260, "GamepadDPadLeft"}, {264, "GamepadDPadRight"},
	{272, "GamepadStart"}, {288, "GamepadBack"}, {320, "GamepadLeftThumb"}, {384, "GamepadRightThumb"},
	{512, "GamepadLeftShoulder"}, {768, "GamepadRightShoulder"}, {4352, "GamepadA"}, {8448, "GamepadB"},
	{16640, "GamepadX"}, {33024, "GamepadY"}
};

} // namespace

int32_t UILinkNavigationComponent::ParseKeyCodeString(const std::string& keyStr) {
	auto it = g_KeyMap.find(keyStr);
	if (it != g_KeyMap.end()) {
		return it->second;
	}

	try {
		if (keyStr.rfind("0x", 0) == 0) {
			return std::stoi(keyStr, nullptr, 16);
		}
		return std::stoi(keyStr);
	} catch (...) {
		return 0; // invalid/unsupported
	}
}

std::string UILinkNavigationComponent::KeyCodeToString(int32_t keyCode) {
	auto it = g_CodeMap.find(keyCode);
	if (it != g_CodeMap.end()) {
		return it->second;
	}

	char buf[32];
	sprintf_s(buf, "0x%02X", keyCode);
	return std::string(buf);
}

std::vector<int32_t> UILinkNavigationComponent::ParseKeyCodesString(const std::string& keysStr) {
	std::vector<int32_t> result;
	std::string current;
	for (char c : keysStr) {
		if (c == ',' || c == ';' || c == ' ' || c == '\t') {
			if (!current.empty()) {
				int32_t code = ParseKeyCodeString(current);
				if (code != 0) {
					result.push_back(code);
				}
				current.clear();
			}
		} else {
			current += c;
		}
	}
	if (!current.empty()) {
		int32_t code = ParseKeyCodeString(current);
		if (code != 0) {
			result.push_back(code);
		}
	}
	// Remove duplicates and sort
	std::sort(result.begin(), result.end());
	result.erase(std::unique(result.begin(), result.end()), result.end());
	return result;
}

std::string UILinkNavigationComponent::KeyCodesToString(const std::vector<int32_t>& keyCodes) {
	std::string result;
	for (size_t i = 0; i < keyCodes.size(); ++i) {
		if (i > 0) result += ", ";
		result += KeyCodeToString(keyCodes[i]);
	}
	return result;
}

const std::vector<std::string>& UILinkNavigationComponent::GetSupportedKeyNames() {
	static std::vector<std::string> names;
	if (names.empty()) {
		for (const auto& pair : g_CodeMap) {
			names.push_back(pair.second);
		}
		std::sort(names.begin(), names.end());
	}
	return names;
}

void ComponentDebug::UILinkNavigationComponentDebug(UILinkNavigationComponent* comp) {
	if (!comp) return;

	ImGui::Text("Links:");
	for (auto& pair : comp->links) {
		std::string keyName = UILinkNavigationComponent::KeyCodeToString(pair.first);
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
			int32_t keyCode = UILinkNavigationComponent::ParseKeyCodeString(it.key());
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
		std::string keyStr = UILinkNavigationComponent::KeyCodeToString(pair.first);
		linksObj[keyStr] = pair.second.ToString();
	}

	j = {
		{ "type", "UILinkNavigationComponent" },
		{ "enable", c.enable },
		{ "links", linksObj }
	};
}
