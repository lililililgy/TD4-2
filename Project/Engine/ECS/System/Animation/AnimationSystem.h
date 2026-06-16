#pragma once

/// engine
#include "../Interface/ECSISystem.h"

namespace ONEngine {

/// ///////////////////////////////////////////////////
/// アニメーションを更新するシステム
/// ///////////////////////////////////////////////////
class AnimationSystem : public ECSISystem {
public:
    AnimationSystem() = default;
    ~AnimationSystem() override = default;

    void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;
    void RuntimeUpdate(class ECSGroup* ecs) override;

private:
    void Update(class ECSGroup* ecs, float deltaTime);
};

} /// namespace ONEngine
