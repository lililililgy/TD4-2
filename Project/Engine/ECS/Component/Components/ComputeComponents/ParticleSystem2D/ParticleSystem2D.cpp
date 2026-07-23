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

    void ParticleSystem2D::Emit(int count) {
        if (count <= 0) return;

        const int requestCapacity = std::max(0, main.maxParticles - pendingEmitCount_);
        pendingEmitCount_ += std::min(count, requestCapacity);
    }

    int ParticleSystem2D::ConsumePendingEmitCount() {
        const int count = pendingEmitCount_;
        pendingEmitCount_ = 0;
        return count;
    }

    void InternalEmitParticleSystem2D(uint64_t nativeHandle, int32_t count) {
        auto* particleSystem = reinterpret_cast<ParticleSystem2D*>(nativeHandle);
        if (!particleSystem || count <= 0) return;

        particleSystem->Emit(count);
    }

}
