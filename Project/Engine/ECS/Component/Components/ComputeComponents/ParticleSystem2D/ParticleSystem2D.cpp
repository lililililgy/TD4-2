#include "ParticleSystem2D.h"
#include <algorithm>
#include "Engine/ECS/System/ParticleSystem2DUpdateSystem/ParticleSystem2DUpdateSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"

namespace ONEngine {

    ParticleSystem2D::ParticleSystem2D() {
        // Initialize default state
    }

    void ParticleSystem2D::Play() {
        isPlaying_ = true;
        isPaused_ = false;
        playbackTime_ = 0.0f;
        aliveCount = 0;
        emitAccumulator = 0.0f;
        std::fill(burstCycleCounts.begin(), burstCycleCounts.end(), 0);
    }

    void ParticleSystem2D::Stop() {
        isPlaying_ = false;
        isPaused_ = false;
    }

    void ParticleSystem2D::Clear() {
        playbackTime_ = 0.0f;
    }

    void ParticleSystem2D::Pause() {
        isPaused_ = true;
    }

    ParticleSystem2D::~ParticleSystem2D() {
        if (aliveCount > 0 && GetOwner()) {
            if (auto* transform = GetOwner()->GetTransform()) {
                ParticleSystem2DUpdateSystem::RegisterGhost(this, transform->matWorld);
            }
        }
    }

}
