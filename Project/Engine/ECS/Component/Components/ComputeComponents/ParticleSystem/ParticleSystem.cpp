#include "ParticleSystem.h"
#include <algorithm>

namespace ONEngine {

    Color ParticleSystemGradient::Evaluate(float time) const {
        if (colorKeys.empty() && alphaKeys.empty()) return Color::kWhite;

        Color result = Color::kWhite;

        // Color
        if (!colorKeys.empty()) {
            if (time <= colorKeys.front().time) {
                result.r = colorKeys.front().color.r;
                result.g = colorKeys.front().color.g;
                result.b = colorKeys.front().color.b;
            } else if (time >= colorKeys.back().time) {
                result.r = colorKeys.back().color.r;
                result.g = colorKeys.back().color.g;
                result.b = colorKeys.back().color.b;
            } else {
                for (size_t i = 0; i < colorKeys.size() - 1; ++i) {
                    if (time >= colorKeys[i].time && time <= colorKeys[i + 1].time) {
                        float dt = colorKeys[i + 1].time - colorKeys[i].time;
                        if (dt < 0.0001f) {
                            result.r = colorKeys[i].color.r;
                            result.g = colorKeys[i].color.g;
                            result.b = colorKeys[i].color.b;
                        } else {
                            float t = (time - colorKeys[i].time) / dt;
                            float t2 = t * t;
                            float t3 = t2 * t;

                            float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
                            float h10 = t3 - 2.0f * t2 + t;
                            float h01 = -2.0f * t3 + 3.0f * t2;
                            float h11 = t3 - t2;

                            // Apply Hermite spline for R, G, B channels
                            float m0_r = colorKeys[i].outTangent * dt;
                            float m1_r = colorKeys[i + 1].inTangent * dt;
                            result.r = h00 * colorKeys[i].color.r + h10 * m0_r + h01 * colorKeys[i + 1].color.r + h11 * m1_r;

                            float m0_g = colorKeys[i].outTangent * dt;
                            float m1_g = colorKeys[i + 1].inTangent * dt;
                            result.g = h00 * colorKeys[i].color.g + h10 * m0_g + h01 * colorKeys[i + 1].color.g + h11 * m1_g;

                            float m0_b = colorKeys[i].outTangent * dt;
                            float m1_b = colorKeys[i + 1].inTangent * dt;
                            result.b = h00 * colorKeys[i].color.b + h10 * m0_b + h01 * colorKeys[i + 1].color.b + h11 * m1_b;
                        }
                        break;
                    }
                }
            }
        }

        // Alpha
        if (!alphaKeys.empty()) {
            if (time <= alphaKeys.front().time) {
                result.a = alphaKeys.front().alpha;
            } else if (time >= alphaKeys.back().time) {
                result.a = alphaKeys.back().alpha;
            } else {
                for (size_t i = 0; i < alphaKeys.size() - 1; ++i) {
                    if (time >= alphaKeys[i].time && time <= alphaKeys[i + 1].time) {
                        float dt = alphaKeys[i + 1].time - alphaKeys[i].time;
                        if (dt < 0.0001f) {
                            result.a = alphaKeys[i].alpha;
                        } else {
                            float t = (time - alphaKeys[i].time) / dt;
                            float t2 = t * t;
                            float t3 = t2 * t;

                            float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
                            float h10 = t3 - 2.0f * t2 + t;
                            float h01 = -2.0f * t3 + 3.0f * t2;
                            float h11 = t3 - t2;

                            float m0 = alphaKeys[i].outTangent * dt;
                            float m1 = alphaKeys[i + 1].inTangent * dt;
                            result.a = h00 * alphaKeys[i].alpha + h10 * m0 + h01 * alphaKeys[i + 1].alpha + h11 * m1;
                        }
                        break;
                    }
                }
            }
        } else if (!colorKeys.empty()) {
            result.a = 1.0f; 
        }

        return result;
    }

    float AnimationCurve::Evaluate(float time) const {
        if (keys.empty()) return 1.0f;

        if (time <= keys.front().time) return keys.front().value;
        if (time >= keys.back().time) return keys.back().value;

        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (time >= keys[i].time && time <= keys[i + 1].time) {
                float dt = keys[i + 1].time - keys[i].time;
                if (dt < 0.0001f) return keys[i].value;

                float t = (time - keys[i].time) / dt;
                float t2 = t * t;
                float t3 = t2 * t;

                // Hermite spline basis functions
                float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
                float h10 = t3 - 2.0f * t2 + t;
                float h01 = -2.0f * t3 + 3.0f * t2;
                float h11 = t3 - t2;

                float m0 = keys[i].outTangent * dt;
                float m1 = keys[i + 1].inTangent * dt;

                return h00 * keys[i].value + h10 * m0 + h01 * keys[i + 1].value + h11 * m1;
            }
        }

        return 1.0f;
    }

    ParticleSystem::ParticleSystem() {
        // Initialize default state
    }

    void ParticleSystem::Play() {
        isPlaying_ = true;
        isPaused_ = false;
        playbackTime_ = 0.0f;
        aliveCount = 0;
        emitAccumulator = 0.0f;
        std::fill(burstCycleCounts.begin(), burstCycleCounts.end(), 0);
    }

    void ParticleSystem::Stop() {
        isPlaying_ = false;
        isPaused_ = false;
    }

    void ParticleSystem::Clear() {
        playbackTime_ = 0.0f;
        // Also clear GPU buffers in the future
    }

    void ParticleSystem::Pause() {
        isPaused_ = true;
    }

}
