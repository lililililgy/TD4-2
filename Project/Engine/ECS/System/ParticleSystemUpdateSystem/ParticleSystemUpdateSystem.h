#pragma once

#include "../Interface/ECSISystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem/ParticleSystem.h"
#include <vector>

namespace ONEngine {

    struct GhostParticleSystem {
        std::vector<Particle> particles;
        size_t aliveCount = 0;
        ParticleSystemMain main;
        ParticleSystemRenderer renderer;
        ParticleSystemColorOverLifetime colorOverLifetime;
        ParticleSystemSizeOverLifetime sizeOverLifetime;
        ParticleSystemVelocityOverLifetime velocityOverLifetime;
        Matrix4x4 finalWorldMat;
    };

    class ParticleSystemUpdateSystem : public ECSISystem {
    public:
        ParticleSystemUpdateSystem();
        ~ParticleSystemUpdateSystem() override = default;

        void RuntimeUpdate(class ECSGroup* ecs) override;
        void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;

        void RegisterGhost(const class ParticleSystem* ps, const Matrix4x4& worldMat);
        void UpdateGhosts(float dt);
        const std::vector<GhostParticleSystem>& GetGhosts() const { return ghostSystems_; }
        std::vector<GhostParticleSystem>& GetGhosts() { return ghostSystems_; }
        void ClearGhosts() { ghostSystems_.clear(); }

    private:
        void DrawGizmos(class ECSGroup* ecs);
        void UpdateSingleSystem(class ParticleSystem* ps, class GameEntity* entity, float dt);

        std::vector<GhostParticleSystem> ghostSystems_;
    };

}
