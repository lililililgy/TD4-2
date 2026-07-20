#pragma once

#include <vector>
#include <string>
#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Color.h"

namespace ONEngine {

    // --- Common Utilities ---

    enum class MinMaxState : uint8_t {
        Constant,
        RandomBetweenTwoConstants,
        Curve,
        RandomBetweenTwoCurves
    };

    struct MinMaxFloat {
        MinMaxState state;
        float constant;
        float minVal;
        float maxVal;
        // AnimationCurve curve; // Future

        MinMaxFloat() : state(MinMaxState::Constant), constant(0.0f), minVal(0.0f), maxVal(1.0f) {}
        MinMaxFloat(float c) : state(MinMaxState::Constant), constant(c), minVal(0.0f), maxVal(1.0f) {}
        MinMaxFloat(float min, float max) : state(MinMaxState::RandomBetweenTwoConstants), constant(0.0f), minVal(min), maxVal(max) {}
    };

    struct MinMaxColor {
        MinMaxState state;
        Color constant;
        Color minVal;
        Color maxVal;

        MinMaxColor() : state(MinMaxState::Constant), constant(Color::kWhite), minVal(Color::kWhite), maxVal(Color::kWhite) {}
        MinMaxColor(const Color& c) : state(MinMaxState::Constant), constant(c), minVal(Color::kWhite), maxVal(Color::kWhite) {}
        MinMaxColor(const Color& min, const Color& max) : state(MinMaxState::RandomBetweenTwoConstants), constant(Color::kWhite), minVal(min), maxVal(max) {}
    };

    struct GradientColorKey {
        Color color;
        float time;
        float inTangent = 0.0f;
        float outTangent = 0.0f;
    };
    struct GradientAlphaKey {
        float alpha;
        float time;
        float inTangent = 0.0f;
        float outTangent = 0.0f;
    };
    struct ParticleSystemGradient {
        std::vector<GradientColorKey> colorKeys;
        std::vector<GradientAlphaKey> alphaKeys;

        Color Evaluate(float time) const;
    };

    struct AnimationCurveKey {
        float time;
        float value;
        float inTangent = 0.0f;
        float outTangent = 0.0f;
    };
    struct AnimationCurve {
        std::vector<AnimationCurveKey> keys;
        float Evaluate(float time) const;
    };

    struct MinMaxGradient {
        MinMaxState state = MinMaxState::Constant;
        ParticleSystemGradient gradient;
        ParticleSystemGradient gradientMin;
        ParticleSystemGradient gradientMax;
    };

    struct MinMaxCurve {
        MinMaxState state = MinMaxState::Constant;
        float constant = 1.0f;
        float constantMin = 0.0f;
        float constantMax = 1.0f;
        AnimationCurve curve;
        AnimationCurve curveMin;
        AnimationCurve curveMax;
    };

    // --- Modules ---

    enum class SimulationSpace : uint8_t {
        Local,
        World
    };

    struct ParticleSystemMain {
        float duration = 5.0f;
        bool looping = true;
        bool prewarm = false;
        MinMaxFloat startDelay = MinMaxFloat(0.0f);
        MinMaxFloat startLifetime = MinMaxFloat(5.0f);
        MinMaxFloat startSpeed = MinMaxFloat(5.0f);
        MinMaxFloat startSize = MinMaxFloat(1.0f);
        MinMaxFloat startRotation = MinMaxFloat(0.0f);
        MinMaxColor startColor = MinMaxColor(Color::kWhite);
        float gravityModifier = 0.0f;
        SimulationSpace simulationSpace = SimulationSpace::World;
        int maxParticles = 1000;
        bool playOnAwake = true;
    };

    struct ParticleSystemEmission {
        bool enabled = true;
        float rateOverTime = 10.0f;
        float rateOverDistance = 0.0f;

        struct Burst {
            float time = 0.0f;
            int count = 30;
            int cycles = 1;
            float interval = 0.01f;
            float probability = 1.0f;
        };
        std::vector<Burst> bursts;
    };

    enum class ParticleSystemShapeType : uint8_t {
        Sphere,
        Hemisphere,
        Cone,
        Box,
        Circle,
        Edge
    };

    struct ParticleSystemShape {
        bool enabled = true;
        ParticleSystemShapeType type = ParticleSystemShapeType::Sphere;
        float radius = 1.0f;
        float radiusThickness = 1.0f; // 0 to 1
        float arc = 360.0f;
        float angle = 25.0f; // For Cone
        Vector3 boxScale = { 1.0f, 1.0f, 1.0f };
    };

    struct ParticleSystemColorOverLifetime {
        bool enabled = false;
        MinMaxGradient color;
    };

    struct ParticleSystemSizeOverLifetime {
        bool enabled = false;
        MinMaxCurve size;
    };

    struct ParticleSystemVelocityOverLifetime {
        bool enabled = false;
        MinMaxCurve x;
        MinMaxCurve y;
        MinMaxCurve z;
        MinMaxCurve speedModifier;
        SimulationSpace space = SimulationSpace::Local;

        ParticleSystemVelocityOverLifetime() {
            speedModifier.constant = 1.0f;
        }
    };

    struct ParticleSystemRotationOverLifetime {
        bool enabled = false;
        MinMaxCurve angularVelocity;
    };

    enum class FlipMode : uint8_t {
        None,
        X,
        Y,
        Both
    };

    struct ParticleSystemRenderer {
        enum class RenderMode : uint8_t {
            Billboard,
            StretchedBillboard,
            HorizontalBillboard,
            VerticalBillboard,
            Mesh
        };
        RenderMode renderMode = RenderMode::Billboard;

        enum class RenderAlignment : uint8_t {
            View,
            Velocity
        };
        RenderAlignment alignment = RenderAlignment::View;

        enum class BlendMode : uint8_t {
            Normal,
            Add,
            Subtract,
            Multiply,
            Screen,
            None
        };
        BlendMode blendMode = BlendMode::Normal;

        float speedScale = 0.0f;
        float lengthScale = 2.0f;
        FlipMode flipMode = FlipMode::None;

        std::string materialGuid; // Reference to material asset
        std::string meshGuid;     // For Mesh mode
    };

    struct ParticleSystemTextureSheetAnimation {
        bool enabled = false;
        int tilesX = 1;
        int tilesY = 1;
        float fps = 12.0f;
    };

}
