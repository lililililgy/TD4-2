#pragma once

#include "../Interface/ECSISystem.h"

namespace ONEngine {

    class ParticleSystem2DUpdateSystem : public ECSISystem {
    public:
        ParticleSystem2DUpdateSystem();
        ~ParticleSystem2DUpdateSystem() override = default;

        void RuntimeUpdate(class ECSGroup* ecs) override;
        void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;

    private:
        void DrawGizmos(class ECSGroup* ecs);
        void UpdateSingleSystem(class ParticleSystem2D* ps, class GameEntity* entity, float dt);
    };

}
