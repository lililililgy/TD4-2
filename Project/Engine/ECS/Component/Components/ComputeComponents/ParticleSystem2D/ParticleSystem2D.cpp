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
        pendingEmitCount_ = 0;
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

    void ParticleSystem2D::SetBoxShape(const Vector3& boxScale) {
        shape.enabled = true;
        shape.type = ParticleSystemShapeType::Box;
        shape.boxScale = Vector3(
            std::max(0.0f, boxScale.x),
            std::max(0.0f, boxScale.y),
            std::max(0.0f, boxScale.z));
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

    void InternalStopParticleSystem2D(uint64_t nativeHandle) {
        auto* particleSystem = reinterpret_cast<ParticleSystem2D*>(nativeHandle);
        if (!particleSystem) return;

        particleSystem->Stop();
    }

    void InternalSetParticleSystem2DBoxShape(uint64_t nativeHandle, float x, float y, float z) {
        auto* particleSystem = reinterpret_cast<ParticleSystem2D*>(nativeHandle);
        if (!particleSystem) return;

        particleSystem->SetBoxShape(Vector3(x, y, z));
    }

}
