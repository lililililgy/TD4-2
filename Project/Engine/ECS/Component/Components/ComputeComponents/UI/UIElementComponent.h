#pragma once

/// std
#include <string>
#include <nlohmann/json_fwd.hpp>

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Asset/Guid/Guid.h"

namespace ONEngine {

class UIElementComponent;

/// Json変換
void from_json(const nlohmann::json& j, UIElementComponent& c);
void to_json(nlohmann::json& j, const UIElementComponent& c);

namespace ComponentDebug {
void UIElementComponentDebug(UIElementComponent* comp);
}

/// ///////////////////////////////////////////////////
/// 個々のUI要素（ボタンやテキストなど）の基本情報コンポーネント
/// ///////////////////////////////////////////////////
class UIElementComponent : public IComponent {
	friend void ComponentDebug::UIElementComponentDebug(UIElementComponent* comp);
	friend void from_json(const nlohmann::json& j, UIElementComponent& c);
	friend void to_json(nlohmann::json& j, const UIElementComponent& c);

public:
	struct BatchData {
		uint32_t compId;
		int32_t groupIdId;
		// elementId and elementIndex can be read from JSON or directly from variables
		int32_t elementIndex;
	};

public:
	UIElementComponent() {
		Reset();
	}

	void Reset() override {
		groupId = Guid::kInvalid;
		groupEntity = nullptr;
		elementId = "";
		elementIndex = 0;
	}

public:
	/// ----- 参照データ ----- ///
	Guid groupId;
	GameEntity* groupEntity = nullptr;

	/// ----- 識別データ ----- ///
	std::string elementId;
	int elementIndex = 0;
};

} /// namespace ONEngine
