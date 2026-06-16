#pragma once

#include "../Interface/ECSISystem.h"

namespace ONEngine {

    class ParticleSystemUpdateSystem : public ECSISystem {
    public:
        ParticleSystemUpdateSystem();
        ~ParticleSystemUpdateSystem() override = default;

        void RuntimeUpdate(class ECSGroup* ecs) override;
        void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;

    private:
        void DrawGizmos(class ECSGroup* ecs);
        void UpdateSingleSystem(class ParticleSystem* ps, class GameEntity* entity, float dt);
    };

}
