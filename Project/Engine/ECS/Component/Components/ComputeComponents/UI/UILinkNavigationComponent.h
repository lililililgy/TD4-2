#pragma once

/// std
#include <unordered_map>
#include <string>
#include <vector>
#include <nlohmann/json_fwd.hpp>

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Asset/Guid/Guid.h"

namespace ONEngine {

class UILinkNavigationComponent;

/// Json変換
void from_json(const nlohmann::json& j, UILinkNavigationComponent& c);
void to_json(nlohmann::json& j, const UILinkNavigationComponent& c);

namespace ComponentDebug {
void UILinkNavigationComponentDebug(UILinkNavigationComponent* comp);
}

/// ///////////////////////////////////////////////////
/// リンク指定型のナビゲーションデータ（結線情報）
/// ///////////////////////////////////////////////////
class UILinkNavigationComponent : public IComponent {
	friend void ComponentDebug::UILinkNavigationComponentDebug(UILinkNavigationComponent* comp);
	friend void from_json(const nlohmann::json& j, UILinkNavigationComponent& c);
	friend void to_json(nlohmann::json& j, const UILinkNavigationComponent& c);

public:
	UILinkNavigationComponent() {
		Reset();
	}

	void Reset() override {
		links.clear();
	}

public:
	static int32_t ParseKeyCodeString(const std::string& keyStr);
	static std::string KeyCodeToString(int32_t keyCode);
	static std::vector<int32_t> ParseKeyCodesString(const std::string& keysStr);
	static std::string KeyCodesToString(const std::vector<int32_t>& keyCodes);
	static const std::vector<std::string>& GetSupportedKeyNames();

public:
	/// ----- リンク遷移マップ (キー: KeyCode, 値: 遷移先Entity Guid) ----- ///
	std::unordered_map<int32_t, Guid> links;
};

} /// namespace ONEngine
