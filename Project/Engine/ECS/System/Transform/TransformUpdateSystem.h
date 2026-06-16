#pragma once

/// engine
#include "../Interface/ECSISystem.h"

/// ///////////////////////////////////////////////////
/// Transformの行列を更新するシステム
/// ///////////////////////////////////////////////////
namespace ONEngine {

class TransformUpdateSystem : public ECSISystem {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================
	
	TransformUpdateSystem() = default;
	~TransformUpdateSystem() override = default;
	
	void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;
	void RuntimeUpdate(class ECSGroup* ecs) override;

	void Update(class ECSGroup* ecs);
};


} /// ONEngine
