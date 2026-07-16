#pragma once

/// std
#include <vector>
#include <string>
#include <nlohmann/json_fwd.hpp>

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Asset/Guid/Guid.h"

namespace ONEngine {

class UIGroupComponent;

/// Json変換
void from_json(const nlohmann::json& j, UIGroupComponent& c);
void to_json(nlohmann::json& j, const UIGroupComponent& c);

namespace ComponentDebug {
void UIGroupComponentDebug(UIGroupComponent* comp);
void UIGroupComponentInspectorDebug(UIGroupComponent* comp);
}

/// ///////////////////////////////////////////////////
/// UIのグループ（画面やメニュー）を管理するコンポーネント
/// ///////////////////////////////////////////////////
class UIGroupComponent : public IComponent {
	friend void ComponentDebug::UIGroupComponentDebug(UIGroupComponent* comp);
	friend void ComponentDebug::UIGroupComponentInspectorDebug(UIGroupComponent* comp);
	friend void from_json(const nlohmann::json& j, UIGroupComponent& c);
	friend void to_json(nlohmann::json& j, const UIGroupComponent& c);

public:
#pragma pack(push, 4)
	struct BatchData {
		uint32_t compId;
		int32_t enable;
		int32_t currentSelectedId;
		uint8_t isFocused;
		uint8_t isVisible;
		uint8_t padding[2];
		int32_t parentGroupId;
	};
#pragma pack(pop)

public:
	UIGroupComponent() {
		Reset();
	}

	void Reset() override {
		currentSelected = nullptr;
		isFocused = false;
		isVisible = true;
		parentGroup = nullptr;
		currentSelectedGuid = Guid::kInvalid;
		parentGroupGuid = Guid::kInvalid;
		submitKeys = { "Return", "Space", "GamepadA" }; // デフォルト決定キー
		cancelKeys = { "Escape", "GamepadB" }; // デフォルトキャンセルキー
	}

public:
	/// ----- ランタイム参照型データ ----- ///
	GameEntity* currentSelected = nullptr;
	GameEntity* parentGroup = nullptr;

	/// ----- シリアライズ/参照解決用データ ----- ///
	Guid currentSelectedGuid;
	Guid parentGroupGuid;

	/// ----- 状態フラグ ----- ///
	bool isFocused = false;
	bool isVisible = true;

	bool wasFocused = false;
	bool wasVisible = true;

	/// ----- 決定キー設定 ----- ///
	std::vector<std::string> submitKeys;

	/// ----- キャンセルキー設定 ----- ///
	std::vector<std::string> cancelKeys;
};

} /// namespace ONEngine
