#pragma once

/// std
#include <unordered_map>
#include <string>
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
	/// ----- リンク遷移マップ (キー: KeyCode, 値: 遷移先Entity Guid) ----- ///
	std::unordered_map<int32_t, Guid> links;
};

} /// namespace ONEngine
