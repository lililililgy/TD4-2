#include "ParticleSystem2D.h"
#include <algorithm>

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

}
