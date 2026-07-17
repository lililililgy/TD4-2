#pragma once

#include "../Interface/ECSISystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem2D/ParticleSystem2D.h"
#include <vector>

namespace ONEngine {

    struct GhostParticleSystem2D {
        std::vector<Particle2D> particles;
        size_t aliveCount = 0;
        ParticleSystemMain main;
        ParticleSystemRenderer renderer;
        ParticleSystemColorOverLifetime colorOverLifetime;
        ParticleSystemSizeOverLifetime sizeOverLifetime;
        ParticleSystemVelocityOverLifetime velocityOverLifetime;
        ParticleSystemTextureSheetAnimation textureSheetAnimation;
        Matrix4x4 finalWorldMat;
    };

    class ParticleSystem2DUpdateSystem : public ECSISystem {
    public:
        ParticleSystem2DUpdateSystem();
        ~ParticleSystem2DUpdateSystem() override = default;

        void RuntimeUpdate(class ECSGroup* ecs) override;
        void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;

        static void RegisterGhost(const class ParticleSystem2D* ps, const Matrix4x4& worldMat);
        static void UpdateGhosts(float dt);
        static const std::vector<GhostParticleSystem2D>& GetGhosts() { return ghostSystems_; }
        static void ClearGhosts() { ghostSystems_.clear(); }

    private:
        void DrawGizmos(class ECSGroup* ecs);
        void UpdateSingleSystem(class ParticleSystem2D* ps, class GameEntity* entity, float dt);

        static std::vector<GhostParticleSystem2D> ghostSystems_;
    };

}
