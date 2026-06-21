#pragma once

/// engine
#include "../Interface/ECSISystem.h"

namespace ONEngine {

/// ///////////////////////////////////////////////////
/// UIの階層と表示・フォーカス状態の変更を監視し制御するシステム
/// ///////////////////////////////////////////////////
class UIHierarchySystem : public ECSISystem {
public:
	UIHierarchySystem();
	~UIHierarchySystem() override;

	void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;
	void RuntimeUpdate(class ECSGroup* ecs) override;

private:
	void UpdateUIHierarchy(class ECSGroup* ecs);
};

} /// namespace ONEngine
