#pragma once

/// externals
#include <nlohmann/json.hpp>

/// engine
#include "Engine/Core/Utility/Utility.h"

#include "Engine/ECS/Component/Components/ComputeComponents/Light/Light.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Effect/Effect.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem/ParticleSystem.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/CustomMeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Primitive/Line2DRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Primitive/Line3DRenderer.h"


namespace ONEngine {
/// /////////////////////////////////////////////////////
/// コンポーネントをJSON形式に変換するコマンド
/// /////////////////////////////////////////////////////
namespace ComponentJsonConverter {
nlohmann::json ToJson(const IComponent* component);
void FromJson(const nlohmann::json& j, IComponent* component);
} /// ComponentJsonConverter


/// //////////////////////////////////////////////////
/// utilities
/// //////////////////////////////////////////////////

// quaternion
void from_json(const nlohmann::json& j, Quaternion& q);
void to_json(nlohmann::json& j, const Quaternion& q);

// color
void from_json(const nlohmann::json& j, Color& c);
void to_json(nlohmann::json& j, const Color& c);


/// //////////////////////////////////////////////////
/// components
/// //////////////////////////////////////////////////

// DirectionalLight
void from_json(const nlohmann::json& j, DirectionalLight& l);
void to_json(nlohmann::json& j, const DirectionalLight& l);

// PointLight
void from_json(const nlohmann::json& j, PointLight& l);
void to_json(nlohmann::json& j, const PointLight& l);

// SpotLight
void from_json(const nlohmann::json& j, SpotLight& l);
void to_json(nlohmann::json& j, const SpotLight& l);

// Effect
void from_json(const nlohmann::json& j, Effect& e);
void to_json(nlohmann::json& j, const Effect& e);
// Effect Member Structs
void from_json(const nlohmann::json& j, Effect::DistanceEmitData& e);
void to_json(nlohmann::json& j, const Effect::DistanceEmitData& e);
void from_json(const nlohmann::json& j, Effect::TimeEmitData& e);
void to_json(nlohmann::json& j, const Effect::TimeEmitData& e);
void from_json(const nlohmann::json& j, EffectMainModule& e);
void to_json(nlohmann::json& j, const EffectMainModule& e);
void from_json(const nlohmann::json& j, EffectEmitShape& e);
void to_json(nlohmann::json& j, const EffectEmitShape& e);

// CustomMeshRenderer
void from_json(const nlohmann::json& j, CustomMeshRenderer& m);
void to_json(nlohmann::json& j, const CustomMeshRenderer& m);

// Line2DRenderer
void from_json(const nlohmann::json& j, Line2DRenderer& l);
void to_json(nlohmann::json& j, const Line2DRenderer& l);

// Line3DRenderer
void from_json(const nlohmann::json& j, Line3DRenderer& l);
void to_json(nlohmann::json& j, const Line3DRenderer& l);

// ParticleSystem
void from_json(const nlohmann::json& j, AnimationCurveKey& k);
void to_json(nlohmann::json& j, const AnimationCurveKey& k);
void from_json(const nlohmann::json& j, AnimationCurve& c);
void to_json(nlohmann::json& j, const AnimationCurve& c);
void from_json(const nlohmann::json& j, MinMaxCurve& m);
void to_json(nlohmann::json& j, const MinMaxCurve& m);
void from_json(const nlohmann::json& j, GradientColorKey& k);
void to_json(nlohmann::json& j, const GradientColorKey& k);
void from_json(const nlohmann::json& j, GradientAlphaKey& k);
void to_json(nlohmann::json& j, const GradientAlphaKey& k);
void from_json(const nlohmann::json& j, ParticleSystemGradient& g);
void to_json(nlohmann::json& j, const ParticleSystemGradient& g);
void from_json(const nlohmann::json& j, MinMaxGradient& m);
void to_json(nlohmann::json& j, const MinMaxGradient& m);

void from_json(const nlohmann::json& j, ParticleSystem& p);
void to_json(nlohmann::json& j, const ParticleSystem& p);
void from_json(const nlohmann::json& j, MinMaxFloat& m);
void to_json(nlohmann::json& j, const MinMaxFloat& m);
void from_json(const nlohmann::json& j, MinMaxColor& m);
void to_json(nlohmann::json& j, const MinMaxColor& m);
void from_json(const nlohmann::json& j, ParticleSystemMain& m);
void to_json(nlohmann::json& j, const ParticleSystemMain& m);
void from_json(const nlohmann::json& j, ParticleSystemEmission& e);
void to_json(nlohmann::json& j, const ParticleSystemEmission& e);
void from_json(const nlohmann::json& j, ParticleSystemEmission::Burst& b);
void to_json(nlohmann::json& j, const ParticleSystemEmission::Burst& b);
void from_json(const nlohmann::json& j, ParticleSystemShape& s);
void to_json(nlohmann::json& j, const ParticleSystemShape& s);
void from_json(const nlohmann::json& j, ParticleSystemRenderer& r);
void to_json(nlohmann::json& j, const ParticleSystemRenderer& r);
void from_json(const nlohmann::json& j, ParticleSystemColorOverLifetime& c);
void to_json(nlohmann::json& j, const ParticleSystemColorOverLifetime& c);
void from_json(const nlohmann::json& j, ParticleSystemSizeOverLifetime& s);
void to_json(nlohmann::json& j, const ParticleSystemSizeOverLifetime& s);
void from_json(const nlohmann::json& j, ParticleSystemVelocityOverLifetime& v);
void to_json(nlohmann::json& j, const ParticleSystemVelocityOverLifetime& v);


} /// ONEngine
