#pragma once

/// engine
#include "../Interface/ECSISystem.h"

namespace ONEngine {

/// ///////////////////////////////////////////////////
/// キー入力を監視し、UIフォーカスの遷移とイベント実行を行うシステム
/// ///////////////////////////////////////////////////
class UIInputNavigationSystem : public ECSISystem {
public:
	UIInputNavigationSystem();
	~UIInputNavigationSystem() override;

	void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;
	void RuntimeUpdate(class ECSGroup* ecs) override;

private:
	void ProcessInputNavigation(class ECSGroup* ecs);
};

} /// namespace ONEngine
