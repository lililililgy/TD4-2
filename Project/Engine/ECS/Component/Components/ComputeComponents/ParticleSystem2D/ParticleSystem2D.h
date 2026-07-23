#pragma once

#include "../../Interface/IComponent.h"
#include "../ParticleSystem/ParticleSystemData.h"
#include "Engine/Core/Utility/Math/Matrix4x4.h"
#include "Engine/Core/Utility/Math/Vector2.h"

namespace ONEngine {

    // CPU-side 2D particle state (Uses Vector3 for compatibility with default shader bindings, but keeps Z=0 or layout consistent)
    struct Particle2D {
        Vector3 position;
        Vector3 velocity;
        Color color;
        float startLifetime = 0.0f;
        float remainingLifetime = 0.0f;
        Vector2 size = { 1.0f, 1.0f };
        float rotation = 0.0f;
        float pad0 = 0.0f;
        Color startColor;
        Vector2 startSize = { 1.0f, 1.0f };
        float randomValue = 0.0f;
        uint32_t simulationSpace = 0; // 0: World, 1: Local
        Vector3 baseVelocity;
        float pad1 = 0.0f;
    };
    static_assert(sizeof(Particle2D) == 112, "Particle2D layout must match HLSL Particle struct size (112 bytes)");

    class ParticleSystem2D : public IComponent {
    public:
        ParticleSystem2D();
        ~ParticleSystem2D() override = default;

        // --- Controls ---
        void Play();
        void Stop();
        void Clear();
        void Pause();
        void Emit(int count);
        int ConsumePendingEmitCount();

        void UpdateTime(float dt) { playbackTime_ += dt; }
        void ResetTime(float t = 0.0f) { playbackTime_ = t; }

        // --- Getters ---
        bool IsPlaying() const { return isPlaying_; }
        bool IsPaused() const { return isPaused_; }
        float GetTime() const { return playbackTime_; }

        // --- Editor Preview ---
        bool isEditorPreview_ = false;
        float editorPlaybackTime_ = 0.0f;
        bool isEditorPaused_ = false;

        // --- Modules ---
        ParticleSystemMain main;
        ParticleSystemEmission emission;
        ParticleSystemShape shape;
        ParticleSystemColorOverLifetime colorOverLifetime;
        ParticleSystemSizeOverLifetime sizeOverLifetime;
        ParticleSystemVelocityOverLifetime velocityOverLifetime;
        ParticleSystemRotationOverLifetime rotationOverLifetime;
        ParticleSystemRenderer renderer;
        ParticleSystemTextureSheetAnimation textureSheetAnimation;
        bool inheritEmitterRotation = false;

        // --- CPU Simulation State ---
        std::vector<Particle2D> particles;
        size_t aliveCount = 0;
        float emitAccumulator = 0.0f;
        std::vector<int> burstCycleCounts; // Track how many times a burst has fired

        Matrix4x4 previousWorldMat;
        bool hasPreviousWorldMat = false;

    private:
        bool isPlaying_ = false;
        bool isPaused_ = false;
        float playbackTime_ = 0.0f;
        int pendingEmitCount_ = 0;
    };

    void InternalEmitParticleSystem2D(uint64_t nativeHandle, int32_t count);

}
