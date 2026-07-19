#pragma once

#include "../Interface/ECSISystem.h"

namespace ONEngine {

class Rigidbody2DUpdateSystem : public ECSISystem {
public:
	Rigidbody2DUpdateSystem() = default;
	~Rigidbody2DUpdateSystem() override = default;

	void RuntimeUpdate(class ECSGroup* ecs) override;
};

} /// namespace ONEngine
