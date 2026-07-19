#define NOMINMAX
#include "ParticleSystem2DUpdateSystem.h"
#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem2D/ParticleSystem2D.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/Core/Utility/Time/Time.h"
#include "Engine/Core/Utility/Tools/Random.h"
#include "Engine/Core/Utility/Tools/Gizmo.h"
#include "Engine/Core/Utility/Tools/Log.h"
#include <numbers>
#include <cmath>
#include <algorithm>
#include <chrono>

namespace ONEngine {

    // --- Helper Functions (Shared logic similar to 3D, adapted for 2D where appropriate) ---

    static float GetMinMaxFloat(const MinMaxFloat& mmf) {
        if (mmf.state == MinMaxState::Constant) return mmf.constant;
        return Random::Float(mmf.minVal, mmf.maxVal);
    }

    static Color GetMinMaxColor(const MinMaxColor& mmc) {
        if (mmc.state == MinMaxState::Constant) return mmc.constant;
        float t = Random::Float(0.0f, 1.0f);
        return Color(
            mmc.minVal.r + (mmc.maxVal.r - mmc.minVal.r) * t,
            mmc.minVal.g + (mmc.maxVal.g - mmc.minVal.g) * t,
            mmc.minVal.b + (mmc.maxVal.b - mmc.minVal.b) * t,
            mmc.minVal.a + (mmc.maxVal.a - mmc.minVal.a) * t
        );
    }

    static float EvaluateMinMaxCurve(const MinMaxCurve& mmc, float time, float randomValue = 0.5f) {
        switch (mmc.state) {
            case MinMaxState::Constant: return mmc.constant;
            case MinMaxState::RandomBetweenTwoConstants: {
                return mmc.constantMin + (mmc.constantMax - mmc.constantMin) * randomValue;
            }
            case MinMaxState::Curve: return mmc.curve.Evaluate(time);
            case MinMaxState::RandomBetweenTwoCurves: {
                float vMin = mmc.curveMin.Evaluate(time);
                float vMax = mmc.curveMax.Evaluate(time);
                return vMin + (vMax - vMin) * randomValue; 
            }
            default: return mmc.constant;
        }
    }

    static Color EvaluateMinMaxGradient(const MinMaxGradient& mmg, float time, float randomValue = 0.5f) {
        switch (mmg.state) {
            case MinMaxState::Constant: return mmg.gradient.Evaluate(time);
            case MinMaxState::Curve: return mmg.gradient.Evaluate(time);
            case MinMaxState::RandomBetweenTwoCurves: {
                Color cMin = mmg.gradientMin.Evaluate(time);
                Color cMax = mmg.gradientMax.Evaluate(time);
                return Color(
                    cMin.r + (cMax.r - cMin.r) * randomValue,
                    cMin.g + (cMax.g - cMin.g) * randomValue,
                    cMin.b + (cMax.b - cMin.b) * randomValue,
                    cMin.a + (cMax.a - cMin.a) * randomValue
                );
            }
            default: return Color::kWhite;
        }
    }

    static Vector3 Vector3Lerp(const Vector3& a, const Vector3& b, float t) {
        return a + (b - a) * t;
    }

    // 2D shape evaluation (generates position & direction on the XY plane)
    static void EvaluateShape2D(const ParticleSystemShape& shape, Vector3& outPos, Vector3& outDir) {
        if (!shape.enabled) {
            outPos = Vector3::Zero;
            outDir = Vector3(0, 1, 0); // Default direction is Y-up for 2D
            return;
        }

        switch (shape.type) {
            case ParticleSystemShapeType::Sphere:
            case ParticleSystemShapeType::Circle: {
                float theta = Random::Float(0.0f, shape.arc) * 3.14159f / 180.0f;
                float r = std::sqrt(Random::Float(0.0f, 1.0f)); 
                float t = Random::Float(1.0f - shape.radiusThickness, 1.0f);
                r *= t * shape.radius;
                outPos = Vector3(r * std::cos(theta), r * std::sin(theta), 0.0f);
                
                if (outPos.LengthSquared() > 0.0001f) {
                    outDir = outPos.Normalize();
                } else {
                    outDir = Vector3(0, 1, 0);
                }
                break;
            }
            case ParticleSystemShapeType::Hemisphere: {
                // XY plane half circle (Y >= 0)
                float arc = std::min(shape.arc, 180.0f);
                float theta = Random::Float(0.0f, arc) * 3.14159f / 180.0f;
                float r = std::sqrt(Random::Float(0.0f, 1.0f)); 
                float t = Random::Float(1.0f - shape.radiusThickness, 1.0f);
                r *= t * shape.radius;
                outPos = Vector3(r * std::cos(theta), r * std::sin(theta), 0.0f);
                
                if (outPos.LengthSquared() > 0.0001f) {
                    outDir = outPos.Normalize();
                } else {
                    outDir = Vector3(0, 1, 0);
                }
                break;
            }
            case ParticleSystemShapeType::Box: {
                outPos = Vector3(
                    Random::Float(-shape.boxScale.x * 0.5f, shape.boxScale.x * 0.5f),
                    Random::Float(-shape.boxScale.y * 0.5f, shape.boxScale.y * 0.5f),
                    0.0f
                );
                outDir = Vector3(0, 1, 0);
                break;
            }
            case ParticleSystemShapeType::Cone: {
                // 2D Cone on XY plane (Y-up centered)
                float angleRad = shape.angle * 3.14159f / 180.0f;
                float halfArcRad = (shape.arc * 0.5f) * 3.14159f / 180.0f;
                float theta = 3.14159f * 0.5f + Random::Float(-halfArcRad, halfArcRad);
                float spreadTheta = theta + Random::Float(-angleRad * 0.5f, angleRad * 0.5f);
                
                outDir = Vector3(std::cos(spreadTheta), std::sin(spreadTheta), 0.0f).Normalize();
                
                float t = Random::Float(1.0f - shape.radiusThickness, 1.0f);
                float r = shape.radius * t;
                outPos = Vector3(r * std::cos(theta), r * std::sin(theta), 0.0f);
                break;
            }
            case ParticleSystemShapeType::Edge: {
                outPos = Vector3(Random::Float(-shape.radius, shape.radius), 0, 0);
                outDir = Vector3(0, 1, 0);
                break;
            }
            default:
                outPos = Vector3::Zero;
                outDir = Vector3(0, 1, 0);
                break;
        }
    }

    // --- System Methods ---

    ParticleSystem2DUpdateSystem::ParticleSystem2DUpdateSystem() : ECSISystem() {}

    void ParticleSystem2DUpdateSystem::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
        DrawGizmos(ecs);

        float dt = Time::UnscaledDeltaTime();
        if (dt <= 0.0f || dt > 0.1f) dt = 1.0f / 60.0f;

        auto& entities = ecs->GetEntities();
        for (auto& entityPtr : entities) {
            GameEntity* entity = entityPtr.get();
            if (!entity || !entity->active) continue;

            auto* ps = entity->GetComponent<ParticleSystem2D>();
            if (!ps || !ps->enable || !ps->isEditorPreview_ || ps->isEditorPaused_) continue;

            UpdateSingleSystem(ps, entity, dt);
        }
    }

    void ParticleSystem2DUpdateSystem::RuntimeUpdate(ECSGroup* ecs) {
        DrawGizmos(ecs);

        float dt = Time::DeltaTime();
        if (dt <= 0.0f) return;

        UpdateGhosts(dt);

        auto& entities = ecs->GetEntities();
        for (auto& entityPtr : entities) {
            GameEntity* entity = entityPtr.get();
            if (!entity || !entity->active) continue;

            auto* ps = entity->GetComponent<ParticleSystem2D>();
            if (!ps || !ps->enable) continue;

            UpdateSingleSystem(ps, entity, dt);
        }
    }

    void ParticleSystem2DUpdateSystem::UpdateSingleSystem(ParticleSystem2D* ps, GameEntity* entity, float dt) {
        Transform* transform = entity->GetTransform();
        if (!transform) return;

        if (ps->main.playOnAwake && ps->GetTime() == 0.0f && !ps->IsPlaying() && !ps->IsPaused()) {
            ps->Play();
        }

        if (ps->IsPlaying() && !ps->IsPaused()) {
            ps->UpdateTime(dt);
        }

        bool isSimulating = ps->IsPlaying() && !ps->IsPaused();

        if (ps->particles.size() != static_cast<size_t>(ps->main.maxParticles)) {
            ps->particles.resize(ps->main.maxParticles);
            if (ps->aliveCount > ps->particles.size()) ps->aliveCount = ps->particles.size();
        }

        if (ps->burstCycleCounts.size() != ps->emission.bursts.size()) {
            ps->burstCycleCounts.resize(ps->emission.bursts.size(), 0);
        }

        float currentPlaybackTime = ps->GetTime();

        // --- Emission ---
        if (isSimulating && ps->emission.enabled && currentPlaybackTime > GetMinMaxFloat(ps->main.startDelay)) {
            ps->emitAccumulator += ps->emission.rateOverTime * dt;
            int emitCount = static_cast<int>(ps->emitAccumulator);
            ps->emitAccumulator -= static_cast<float>(emitCount);

            for (int i = 0; i < emitCount; ++i) {
                if (ps->aliveCount >= ps->particles.size()) break;

                Particle2D& p = ps->particles[ps->aliveCount++];
                Vector3 shapePos, shapeDir;
                EvaluateShape2D(ps->shape, shapePos, shapeDir);

                Matrix4x4 currentMat = transform->matWorld;
                Matrix4x4 emitMat = currentMat;
                if (ps->hasPreviousWorldMat) {
                    float t_emit = (float)i / (float)(emitCount > 1 ? emitCount - 1 : 1);
                    Vector3 prevPos(ps->previousWorldMat.m[3][0], ps->previousWorldMat.m[3][1], ps->previousWorldMat.m[3][2]);
                    Vector3 currPos(currentMat.m[3][0], currentMat.m[3][1], currentMat.m[3][2]);
                    Vector3 lerpedPos = Vector3Lerp(prevPos, currPos, t_emit);
                    emitMat.m[3][0] = lerpedPos.x;
                    emitMat.m[3][1] = lerpedPos.y;
                    emitMat.m[3][2] = lerpedPos.z;
                }

                if (ps->main.simulationSpace == SimulationSpace::World) {
                    p.position = Matrix4x4::Transform(shapePos, emitMat);
                    p.velocity = Matrix4x4::TransformNormal(shapeDir, emitMat).Normalize() * GetMinMaxFloat(ps->main.startSpeed);
                    p.simulationSpace = 0; // World
                } else {
                    p.position = shapePos;
                    p.velocity = shapeDir * GetMinMaxFloat(ps->main.startSpeed);
                    p.simulationSpace = 1; // Local
                }

                // Force 2D alignment (Z = 0)
                p.position.z = 0.0f;
                p.velocity.z = 0.0f;

                p.baseVelocity = p.velocity;
                p.startLifetime = GetMinMaxFloat(ps->main.startLifetime);
                p.remainingLifetime = p.startLifetime;
                p.startColor = GetMinMaxColor(ps->main.startColor);
                p.color = p.startColor;
                p.startSize = GetMinMaxFloat(ps->main.startSize);
                p.size = p.startSize;
                p.rotation = GetMinMaxFloat(ps->main.startRotation);
                p.randomValue = Random::Float(0.0f, 1.0f);
            }

            for (size_t i = 0; i < ps->emission.bursts.size(); ++i) {
                auto& burst = ps->emission.bursts[i];
                int& cycleCount = ps->burstCycleCounts[i];
                
                float burstTime = burst.time;
                if (currentPlaybackTime >= burstTime && cycleCount < burst.cycles) {
                    float nextBurstTime = burstTime + static_cast<float>(cycleCount) * burst.interval;
                    if (currentPlaybackTime >= nextBurstTime) {
                        if (Random::Float(0.0f, 1.0f) <= burst.probability) {
                            int burstEmitCount = burst.count;
                            for (int e = 0; e < burstEmitCount; ++e) {
                                if (ps->aliveCount >= ps->particles.size()) break;
                                Particle2D& p = ps->particles[ps->aliveCount++];
                                Vector3 shapePos, shapeDir;
                                EvaluateShape2D(ps->shape, shapePos, shapeDir);
                                
                                Matrix4x4 worldMat = transform->matWorld;
                                if (ps->main.simulationSpace == SimulationSpace::World) {
                                    p.position = Matrix4x4::Transform(shapePos, worldMat);
                                    p.velocity = Matrix4x4::TransformNormal(shapeDir, worldMat).Normalize() * GetMinMaxFloat(ps->main.startSpeed);
                                    p.simulationSpace = 0; // World
                                } else {
                                    p.position = shapePos;
                                    p.velocity = shapeDir * GetMinMaxFloat(ps->main.startSpeed);
                                    p.simulationSpace = 1; // Local
                                }

                                // Force 2D alignment (Z = 0)
                                p.position.z = 0.0f;
                                p.velocity.z = 0.0f;

                                p.baseVelocity = p.velocity;
                                p.startLifetime = GetMinMaxFloat(ps->main.startLifetime);
                                p.remainingLifetime = p.startLifetime;
                                p.startColor = GetMinMaxColor(ps->main.startColor);
                                p.color = p.startColor;
                                p.startSize = GetMinMaxFloat(ps->main.startSize);
                                p.size = p.startSize;
                                p.rotation = GetMinMaxFloat(ps->main.startRotation);
                                p.randomValue = Random::Float(0.0f, 1.0f);
                            }
                        }
                        cycleCount++;
                    }
                }
            }
        }

        if (isSimulating && currentPlaybackTime > ps->main.duration) {
            if (ps->main.looping) {
                ps->ResetTime(currentPlaybackTime - ps->main.duration);
                std::fill(ps->burstCycleCounts.begin(), ps->burstCycleCounts.end(), 0);
            } else {
                ps->Stop();
            }
        }

        // --- Update ---
        if (!ps->IsPaused() && (ps->IsPlaying() || ps->aliveCount > 0)) {
            Vector3 gravity = Vector3(0.0f, -9.81f, 0.0f) * ps->main.gravityModifier;
            
            for (size_t i = 0; i < ps->aliveCount; ) {
                Particle2D& p = ps->particles[i];
                p.remainingLifetime -= dt;

                if (p.remainingLifetime <= 0.0f) {
                    if (ps->aliveCount > 1) {
                        p = ps->particles[ps->aliveCount - 1];
                    }
                    ps->aliveCount--;
                } else {
                    float normalizedTime = 1.0f - (p.remainingLifetime / p.startLifetime);
                    normalizedTime = std::clamp(normalizedTime, 0.0f, 1.0f);

                    if (ps->velocityOverLifetime.enabled) {
                        Vector3 linearVelocity(
                            EvaluateMinMaxCurve(ps->velocityOverLifetime.x, normalizedTime, p.randomValue),
                            EvaluateMinMaxCurve(ps->velocityOverLifetime.y, normalizedTime, p.randomValue),
                            0.0f // Force Z = 0 in 2D
                        );
                        float speedMultiplier = EvaluateMinMaxCurve(ps->velocityOverLifetime.speedModifier, normalizedTime, p.randomValue);

                        if (ps->velocityOverLifetime.space == SimulationSpace::World) {
                            p.velocity = p.baseVelocity * speedMultiplier + linearVelocity;
                        } else {
                            p.velocity = p.baseVelocity * speedMultiplier + Matrix4x4::TransformNormal(linearVelocity, transform->matWorld);
                        }
                    }

                    if (p.simulationSpace == 1) { // Local
                        Vector3 localGravity = Matrix4x4::TransformNormal(gravity, Matrix4x4::MakeInverse(transform->matWorld));
                        localGravity.z = 0.0f; // Force 2D
                        p.velocity += localGravity * dt;
                    } else {
                        p.velocity += gravity * dt;
                    }

                    // Enforce 2D velocity & position
                    p.velocity.z = 0.0f;
                    p.position += p.velocity * dt;
                    p.position.z = 0.0f;

                    if (ps->colorOverLifetime.enabled) {
                        Color overLifeColor = EvaluateMinMaxGradient(ps->colorOverLifetime.color, normalizedTime, p.randomValue);
                        p.color.r = p.startColor.r * overLifeColor.r;
                        p.color.g = p.startColor.g * overLifeColor.g;
                        p.color.b = p.startColor.b * overLifeColor.b;
                        p.color.a = p.startColor.a * overLifeColor.a;
                    }
                    if (ps->sizeOverLifetime.enabled) {
                        p.size = p.startSize * EvaluateMinMaxCurve(ps->sizeOverLifetime.size, normalizedTime, p.randomValue);
                    }
                    if (ps->rotationOverLifetime.enabled) {
                        float angularVelocityDeg = EvaluateMinMaxCurve(ps->rotationOverLifetime.angularVelocity, normalizedTime, p.randomValue);
                        float angularVelocityRad = angularVelocityDeg * (3.14159265f / 180.0f);
                        p.rotation += angularVelocityRad * dt;
                    }
                    i++;
                }
            }
        }
        ps->previousWorldMat = transform->matWorld;
        ps->hasPreviousWorldMat = true;
    }

    void ParticleSystem2DUpdateSystem::DrawGizmos(ECSGroup* ecs) {
        if (!ecs) return;
        auto& entities = ecs->GetEntities();
        for (auto& entityPtr : entities) {
            GameEntity* entity = entityPtr.get();
            if (!entity || !entity->active) continue;

            auto* ps = entity->GetComponent<ParticleSystem2D>();
            if (!ps || !ps->enable) continue;

            auto* transform = entity->GetTransform();
            if (!transform) continue;

            const auto& shape = ps->shape;
            if (!shape.enabled) continue;

            Matrix4x4 worldMat = transform->matWorld;
            Vector4 color = Vector4(1.0f, 0.0f, 0.0f, 0.5f); // Red color for 2D gizmos

            float outerRadius = shape.radius;
            float innerRadius = shape.radius * (1.0f - shape.radiusThickness);

            auto DrawCircle2D = [&](const Matrix4x4& m, float r, float arc) {
                if (r <= 0.0f) return;
                const int segments = 32;
                float arcRad = arc * 3.14159f / 180.0f;
                for (int i = 0; i < segments; i++) {
                    float t1 = (float)i / segments * arcRad;
                    float t2 = (float)(i + 1) / segments * arcRad;
                    Vector3 p1 = { r * std::cos(t1), r * std::sin(t1), 0.0f };
                    Vector3 p2 = { r * std::cos(t2), r * std::sin(t2), 0.0f };
                    Gizmo::DrawLine(Matrix4x4::Transform(p1, m), Matrix4x4::Transform(p2, m), color);
                }
            };

            switch (shape.type) {
                case ParticleSystemShapeType::Sphere:
                case ParticleSystemShapeType::Circle:
                    DrawCircle2D(worldMat, outerRadius, shape.arc);
                    if (shape.radiusThickness < 1.0f) {
                        DrawCircle2D(worldMat, innerRadius, shape.arc);
                    }
                    break;
                case ParticleSystemShapeType::Hemisphere:
                    DrawCircle2D(worldMat, outerRadius, std::min(shape.arc, 180.0f));
                    if (shape.radiusThickness < 1.0f) {
                        DrawCircle2D(worldMat, innerRadius, std::min(shape.arc, 180.0f));
                    }
                    break;
                case ParticleSystemShapeType::Box:
                {
                    Vector3 h = shape.boxScale * 0.5f;
                    Vector3 v[4] = {
                        {-h.x, -h.y, 0.0f}, {h.x, -h.y, 0.0f},
                        {h.x, h.y, 0.0f}, {-h.x, h.y, 0.0f}
                    };
                    for (int i = 0; i < 4; i++) v[i] = Matrix4x4::Transform(v[i], worldMat);
                    for (int i = 0; i < 4; i++) {
                        Gizmo::DrawLine(v[i], v[(i + 1) % 4], color);
                    }
                }
                break;
                case ParticleSystemShapeType::Cone:
                {
                    // Draw 2D Cone sector
                    float halfArcRad = (shape.arc * 0.5f) * 3.14159f / 180.0f;
                    
                    float t = 3.14159f * 0.5f; // Center Y-up
                    Vector3 pCenter = { 0, 0, 0 };
                    
                    Vector3 pLeft = Vector3(std::cos(t - halfArcRad), std::sin(t - halfArcRad), 0.0f) * outerRadius;
                    Vector3 pRight = Vector3(std::cos(t + halfArcRad), std::sin(t + halfArcRad), 0.0f) * outerRadius;
                    
                    Gizmo::DrawLine(Matrix4x4::Transform(pCenter, worldMat), Matrix4x4::Transform(pLeft, worldMat), color);
                    Gizmo::DrawLine(Matrix4x4::Transform(pCenter, worldMat), Matrix4x4::Transform(pRight, worldMat), color);
                    
                    // Outer arc
                    const int segments = 16;
                    for (int i = 0; i < segments; i++) {
                        float t1 = (t - halfArcRad) + (float)i / segments * (shape.arc * 3.14159f / 180.0f);
                        float t2 = (t - halfArcRad) + (float)(i + 1) / segments * (shape.arc * 3.14159f / 180.0f);
                        Vector3 p1 = Vector3(std::cos(t1), std::sin(t1), 0.0f) * outerRadius;
                        Vector3 p2 = Vector3(std::cos(t2), std::sin(t2), 0.0f) * outerRadius;
                        Gizmo::DrawLine(Matrix4x4::Transform(p1, worldMat), Matrix4x4::Transform(p2, worldMat), color);
                    }
                }
                break;
                case ParticleSystemShapeType::Edge:
                    Gizmo::DrawLine(Matrix4x4::Transform({ -shape.radius, 0, 0 }, worldMat), Matrix4x4::Transform({ shape.radius, 0, 0 }, worldMat), color);
                    break;
            }
        }
    }

    void ParticleSystem2DUpdateSystem::RegisterGhost(const ParticleSystem2D* ps, const Matrix4x4& worldMat) {
        if (!ps || ps->aliveCount == 0) return;

        GhostParticleSystem2D ghost;
        ghost.particles = ps->particles;
        ghost.aliveCount = ps->aliveCount;
        ghost.main = ps->main;
        ghost.renderer = ps->renderer;
        ghost.colorOverLifetime = ps->colorOverLifetime;
        ghost.sizeOverLifetime = ps->sizeOverLifetime;
        ghost.velocityOverLifetime = ps->velocityOverLifetime;
        ghost.rotationOverLifetime = ps->rotationOverLifetime;
        ghost.textureSheetAnimation = ps->textureSheetAnimation;
        ghost.finalWorldMat = worldMat;

        ghostSystems_.push_back(std::move(ghost));
    }

    void ParticleSystem2DUpdateSystem::UpdateGhosts(float dt) {
        Vector3 gravity = Vector3(0.0f, -9.81f, 0.0f);

        for (auto it = ghostSystems_.begin(); it != ghostSystems_.end(); ) {
            GhostParticleSystem2D& ghost = *it;
            Vector3 finalGravity = gravity * ghost.main.gravityModifier;

            for (size_t i = 0; i < ghost.aliveCount; ) {
                Particle2D& p = ghost.particles[i];
                p.remainingLifetime -= dt;

                if (p.remainingLifetime <= 0.0f) {
                    if (ghost.aliveCount > 1) {
                        p = ghost.particles[ghost.aliveCount - 1];
                    }
                    ghost.aliveCount--;
                } else {
                    float normalizedTime = 1.0f - (p.remainingLifetime / p.startLifetime);
                    normalizedTime = std::clamp(normalizedTime, 0.0f, 1.0f);

                    if (ghost.velocityOverLifetime.enabled) {
                        Vector3 linearVelocity(
                            EvaluateMinMaxCurve(ghost.velocityOverLifetime.x, normalizedTime, p.randomValue),
                            EvaluateMinMaxCurve(ghost.velocityOverLifetime.y, normalizedTime, p.randomValue),
                            0.0f
                        );
                        float speedMultiplier = EvaluateMinMaxCurve(ghost.velocityOverLifetime.speedModifier, normalizedTime, p.randomValue);

                        if (ghost.velocityOverLifetime.space == SimulationSpace::World) {
                            p.velocity = p.baseVelocity * speedMultiplier + linearVelocity;
                        } else {
                            p.velocity = p.baseVelocity * speedMultiplier + Matrix4x4::TransformNormal(linearVelocity, ghost.finalWorldMat);
                        }
                    }

                    if (p.simulationSpace == 1) { // Local
                        Vector3 localGravity = Matrix4x4::TransformNormal(finalGravity, Matrix4x4::MakeInverse(ghost.finalWorldMat));
                        localGravity.z = 0.0f;
                        p.velocity += localGravity * dt;
                    } else {
                        p.velocity += finalGravity * dt;
                    }

                    p.velocity.z = 0.0f;
                    p.position += p.velocity * dt;
                    p.position.z = 0.0f;

                    if (ghost.colorOverLifetime.enabled) {
                        Color overLifeColor = EvaluateMinMaxGradient(ghost.colorOverLifetime.color, normalizedTime, p.randomValue);
                        p.color.r = p.startColor.r * overLifeColor.r;
                        p.color.g = p.startColor.g * overLifeColor.g;
                        p.color.b = p.startColor.b * overLifeColor.b;
                        p.color.a = p.startColor.a * overLifeColor.a;
                    }
                    if (ghost.sizeOverLifetime.enabled) {
                        p.size = p.startSize * EvaluateMinMaxCurve(ghost.sizeOverLifetime.size, normalizedTime, p.randomValue);
                    }
                    if (ghost.rotationOverLifetime.enabled) {
                        float angularVelocityDeg = EvaluateMinMaxCurve(ghost.rotationOverLifetime.angularVelocity, normalizedTime, p.randomValue);
                        float angularVelocityRad = angularVelocityDeg * (3.14159265f / 180.0f);
                        p.rotation += angularVelocityRad * dt;
                    }
                    i++;
                }
            }

            if (ghost.aliveCount == 0) {
                it = ghostSystems_.erase(it);
            } else {
                ++it;
            }
        }
    }

}
