#include "ComponentJsonConverter.h"

/// std
#include <unordered_map>
#include <typeindex>
#include <functional>

#include "Engine/ECS/Component/Collection/ComponentHash.h"

/// engine/compute
#include "Engine/ECS/Component/Components/ComputeComponents/Terrain/Terrain.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Terrain/Grass/GrassField.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Terrain/TerrainCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/VoxelTerrain/VoxelTerrain.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/SphereCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/CircleCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider2D.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Script/Script.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Audio/BGMPlayer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Audio/SEPlayer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem/ParticleSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem2D/ParticleSystem2D.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Variables/Variables.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ShadowCaster/ShadowCaster.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Agent/AgentIntentComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Animator/Animator.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Animation/AnimationPlayer.h"

/// engine/renderer
#include "Engine/ECS/Component/Components/RendererComponents/Skybox/Skybox.h"
#include "Engine/ECS/Component/Components/RendererComponents/Sprite/SpriteRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/MeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/DissolveMeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/SkinMesh/SkinMeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/ScreenPostEffectTag/ScreenPostEffectTag.h"


using namespace ONEngine;


/// //////////////////////////////////////////////////
/// ComponentJsonConverter
/// //////////////////////////////////////////////////

namespace {
	class JsonConverter {
	public:

		JsonConverter() {

			/// compute
			Register<Transform>();
			Register<Variables>();
			Register<DirectionalLight>();
			Register<PointLight>();
			Register<SpotLight>();
			Register<BGMPlayer>();
			Register<SEPlayer>();
			Register<Effect>();
			Register<ParticleSystem>();
			Register<ParticleSystem2D>();
			Register<Script>();
			Register<Terrain>();
			Register<GrassField>();
			Register<TerrainCollider>();
			Register<CameraComponent>();
			Register<ShadowCaster>();
			Register<VoxelTerrain>();
			Register<AgentIntentComponent>();
			Register<Animator>();
			Register<AnimationPlayer>();

			/// renderer
			Register<SpriteRenderer>();
			Register<CustomMeshRenderer>();
			Register<MeshRenderer>();
			Register<DissolveMeshRenderer>();
			Register<SkinMeshRenderer>();
			Register<Line2DRenderer>();
			Register<Line3DRenderer>();
			Register<ScreenPostEffectTag>();
			Register<Skybox>();

			/// collision
			Register<SphereCollider>();
			Register<BoxCollider>();
			Register<CircleCollider>();
			Register<BoxCollider2D>();

			/// UI
			Register<UIGroupComponent>();
			Register<UIElementComponent>();
			Register<UILinkNavigationComponent>();
		}

		template <typename T>
		void Register() {
			std::string typeName = GetComponentTypeName<T>();

			converters_[typeName] = [](const IComponent* component) {
				return *static_cast<const T*>(component);
				};

			fromJsonConverters_[typeName] = [this](IComponent* component, const nlohmann::json& j) {
				uint32_t id = component->id;
 				*static_cast<T*>(component) = j.get<T>();
				component->id = id;
				};
		}

		using Converter = std::function<nlohmann::json(const IComponent*)>;
		using FromJsonConverter = std::function<void(IComponent*, const nlohmann::json&)>;
		std::unordered_map<std::string, Converter> converters_;
		std::unordered_map<std::string, FromJsonConverter> fromJsonConverters_;

	};

	JsonConverter jsonConverter;
}

nlohmann::json ComponentJsonConverter::ToJson(const IComponent* component) {
	std::string name = GetComponentTypeName(component);

	auto itr = jsonConverter.converters_.find(name);
	if (itr == jsonConverter.converters_.end()) {
		return nlohmann::json{};
	}

	return itr->second(component);
}

void ComponentJsonConverter::FromJson(const nlohmann::json& j, IComponent* component) {
	std::string name = GetComponentTypeName(component);

	auto itr = jsonConverter.fromJsonConverters_.find(name);
	if (itr == jsonConverter.fromJsonConverters_.end()) {
		Console::Log("ComponentJsonConverter: " + name + "の変換関数が登録されていません。");
		return;
	}

	itr->second(component, j);
}

void ONEngine::from_json(const nlohmann::json& j, Quaternion& q) {
	q.x = j.at("x").get<float>();
	q.y = j.at("y").get<float>();
	q.z = j.at("z").get<float>();
	q.w = j.at("w").get<float>();
}

void ONEngine::to_json(nlohmann::json& j, const Quaternion& q) {
	j = nlohmann::json{ { "x", q.x }, { "y", q.y }, { "z", q.z }, { "w", q.w } };
}

void ONEngine::from_json(const nlohmann::json& j, Color& c) {
	c.r = j.at("r").get<float>();
	c.g = j.at("g").get<float>();
	c.b = j.at("b").get<float>();
	c.a = j.at("a").get<float>();
}

void ONEngine::to_json(nlohmann::json& j, const Color& c) {
	j = nlohmann::json{ { "r", c.r }, { "g", c.g }, { "b", c.b }, { "a", c.a } };
}


void ONEngine::from_json(const nlohmann::json& j, DirectionalLight& l) {
	l.SetIntensity(j.at("intensity").get<float>());
	l.SetDirection(j.at("direction").get<Vector3>());
	l.SetColor(j.at("color").get<Vector4>());
}

void ONEngine::to_json(nlohmann::json& j, const DirectionalLight& l) {
	j = nlohmann::json{
		{ "type", "DirectionalLight" },
		{ "enable", l.enable },
		{ "intensity", l.GetIntensity() },
		{ "direction", l.GetDirection() },
		{ "color", l.GetColor() }
	};
}

void ONEngine::from_json(const nlohmann::json& j, PointLight& l) {
	l.SetIntensity(j.at("intensity").get<float>());
	l.SetRadius(j.at("radius").get<float>());
	l.SetColor(j.at("color").get<Vector4>());
}

void ONEngine::to_json(nlohmann::json& j, const PointLight& l) {
	j = nlohmann::json{
		{ "type", "PointLight" },
		{ "enable", l.enable },
		{ "intensity", l.GetIntensity() },
		{ "radius", l.GetRadius() },
		{ "color", l.GetColor() }
	};
}

void ONEngine::from_json(const nlohmann::json& j, SpotLight& l) {
	l.SetIntensity(j.at("intensity").get<float>());
	l.SetDirection(j.at("direction").get<Vector3>());
	l.SetRadius(j.at("radius").get<float>());
	l.SetColor(j.at("color").get<Vector4>());
	l.SetInnerAngle(j.at("innerAngle").get<float>());
	l.SetOuterAngle(j.at("outerAngle").get<float>());
}

void ONEngine::to_json(nlohmann::json& j, const SpotLight& l) {
	j = nlohmann::json{
		{ "type", "SpotLight" },
		{ "enable", l.enable },
		{ "intensity", l.GetIntensity() },
		{ "direction", l.GetDirection() },
		{ "radius", l.GetRadius() },
		{ "color", l.GetColor() },
		{ "innerAngle", l.GetInnerAngle() },
		{ "outerAngle", l.GetOuterAngle() }
	};
}

void ONEngine::from_json(const nlohmann::json& j, Effect& e) {
	e.enable = j.at("enable").get<int>();
	e.SetIsCreateParticle(j.at("isCreateParticle").get<bool>());
	e.SetMeshPath(j.at("meshPath").get<std::string>());
	e.SetTexturePath(j.at("texturePath").get<std::string>());
	e.SetMainModule(j.at("mainModule").get<EffectMainModule>());
	e.SetEmitShape(j.at("emitShape").get<EffectEmitShape>());
	e.SetEmitType(static_cast<Effect::EmitType>(j.at("emitType").get<int>()));
	e.SetEmitTypeDistance(j.at("distanceEmitData").get<Effect::DistanceEmitData>());
	e.SetEmitTypeTime(j.at("timeEmitData").get<Effect::TimeEmitData>());
	e.SetMaxEffectCount(j.at("maxEffectCount").get<size_t>());
	e.SetEmitInstanceCount(j.at("emitInstanceCount").get<size_t>());
	e.SetBlendMode(static_cast<Effect::BlendMode>(j.at("blendMode").get<int>()));
}

void ONEngine::to_json(nlohmann::json& j, const Effect& e) {
	j = nlohmann::json{
		{ "type", "Effect" },
		{ "enable", e.enable },
		{ "isCreateParticle", e.IsCreateParticle() },
		{ "meshPath", e.GetMeshPath() },
		{ "texturePath", e.GetTexturePath() },
		{ "mainModule", e.GetMainModule() },
		{ "emitShape", e.GetEmitShape() },
		{ "emitType", e.GetEmitType() },
		{ "distanceEmitData", e.GetDistanceEmitData() },
		{ "timeEmitData", e.GetTimeEmitData() },
		{ "maxEffectCount", e.GetMaxEffectCount() },
		{ "emitInstanceCount", e.GetEmitInstanceCount() },
		{ "blendMode", static_cast<int>(e.GetBlendMode()) },
	};
}

void ONEngine::from_json(const nlohmann::json& j, Effect::DistanceEmitData& e) {
	e.emitDistance = j.at("emitDistance").get<float>();
	e.emitInterval = j.at("emitInterval").get<float>();
}

void ONEngine::to_json(nlohmann::json& j, const Effect::DistanceEmitData& e) {
	j = nlohmann::json{
		{ "emitDistance", e.emitDistance },
		{ "emitInterval", e.emitInterval }
	};
}

void ONEngine::from_json(const nlohmann::json& j, Effect::TimeEmitData& e) {
	e.emitTime = j.at("emitTime").get<float>();
	e.emitInterval = j.at("emitInterval").get<float>();
}

void ONEngine::to_json(nlohmann::json& j, const Effect::TimeEmitData& e) {
	j = nlohmann::json{
		{ "emitTime", e.emitTime },
		{ "emitInterval", e.emitInterval }
	};
}

void ONEngine::from_json(const nlohmann::json& j, EffectMainModule& e) {
	e.SetLifeLeftTime(j.at("lifeLeftTime").get<float>());
	e.SetSpeedStartData(j.at("startSpeed").get<std::pair<float, float>>());
	e.SetSizeStartData(j.at("startSize").get<std::pair<Vector3, Vector3>>());
	e.SetRotateStartData(j.at("startRotate").get<std::pair<Vector3, Vector3>>());
	e.SetColorStartData(j.at("startColor").get<std::pair<Color, Color>>());
	e.SetGravityModifier(j.at("gravityModifier").get<float>());
}

void ONEngine::to_json(nlohmann::json& j, const EffectMainModule& e) {
	j = nlohmann::json{
		{ "lifeLeftTime", e.GetLifeLeftTime() },
		{ "startSpeed", e.GetSpeedStartData() },
		{ "startSize", e.GetSizeStartData() },
		{ "startRotate", e.GetRotateStartData() },
		{ "startColor", e.GetColorStartData() },
		{ "gravityModifier", e.GetGravityModifier() }
	};
}

void ONEngine::from_json(const nlohmann::json& j, EffectEmitShape& e) {
	int type = j.at("type").get<int>();
	switch (type) {
	case static_cast<int>(EffectEmitShape::ShapeType::Sphere):
		e.SetSphere(j.at("sphere").get<Sphere>());
		break;
	case static_cast<int>(EffectEmitShape::ShapeType::Cube):
		e.SetCube(j.at("cube").get<Cube>());
		break;
	case static_cast<int>(EffectEmitShape::ShapeType::Cone):
		e.SetCone(j.at("cone").get<Cone>());
		break;
	default:
		break;
	}
}

void ONEngine::to_json(nlohmann::json& j, const EffectEmitShape& e) {
	j = nlohmann::json{
		{ "type", static_cast<int>(e.GetType()) },
		{ "sphere", e.GetSphere() },
		{ "cube", e.GetCube() },
		{ "cone", e.GetCone() }
	};
}


void ONEngine::from_json(const nlohmann::json& j, CustomMeshRenderer& m) {
	m.enable = j.at("enable").get<int>();
	m.SetTexturePath(j.at("texturePath").get<std::string>());
	m.SetColor(j.at("color").get<Color>());
}
void ONEngine::to_json(nlohmann::json& j, const CustomMeshRenderer& m) {
	j = nlohmann::json{
		{ "type", "CustomMeshRenderer" },
		{ "enable", m.enable },
		{ "texturePath", m.GetTexturePath() },
		{ "color", m.GetColor() }
	};
}

void ONEngine::from_json(const nlohmann::json& j, Line2DRenderer& l) {
	l.enable = j.at("enable").get<int>();
}

void ONEngine::to_json(nlohmann::json& j, const Line2DRenderer& l) {
	j = nlohmann::json{
		{ "type", "Line2DRenderer" },
		{ "enable", l.enable }
	};
}

void ONEngine::from_json(const nlohmann::json& j, Line3DRenderer& l) {
	if (!j.contains("enable")) {
		l.enable = j.at("enable").get<int>();
	}
}

void ONEngine::to_json(nlohmann::json& j, const Line3DRenderer& l) {
	j = nlohmann::json{
		{ "type", "Line3DRenderer" },
		{ "enable", l.enable }
	};
}

// ParticleSystem

void ONEngine::from_json(const nlohmann::json& j, AnimationCurveKey& k) {
	k.time = j.value("time", 0.0f);
	k.value = j.value("value", 1.0f);
	k.inTangent = j.value("inTangent", 0.0f);
	k.outTangent = j.value("outTangent", 0.0f);
}
void ONEngine::to_json(nlohmann::json& j, const AnimationCurveKey& k) {
	j = nlohmann::json{
		{"time", k.time},
		{"value", k.value},
		{"inTangent", k.inTangent},
		{"outTangent", k.outTangent}
	};
}

void ONEngine::from_json(const nlohmann::json& j, AnimationCurve& c) {
	c.keys = j.value("keys", std::vector<AnimationCurveKey>());
}
void ONEngine::to_json(nlohmann::json& j, const AnimationCurve& c) {
	j = nlohmann::json{ {"keys", c.keys} };
}

void ONEngine::from_json(const nlohmann::json& j, MinMaxCurve& m) {
	m.state = static_cast<MinMaxState>(j.value("state", static_cast<uint8_t>(MinMaxState::Constant)));
	m.constant = j.value("constant", 1.0f);
	m.curve = j.value("curve", AnimationCurve());
	m.curveMin = j.value("curveMin", AnimationCurve());
	m.curveMax = j.value("curveMax", AnimationCurve());
}
void ONEngine::to_json(nlohmann::json& j, const MinMaxCurve& m) {
	j = nlohmann::json{
		{"state", static_cast<uint8_t>(m.state)},
		{"constant", m.constant},
		{"curve", m.curve},
		{"curveMin", m.curveMin},
		{"curveMax", m.curveMax}
	};
}

void ONEngine::from_json(const nlohmann::json& j, GradientColorKey& k) {
	k.color = j.value("color", Color::kWhite);
	k.time = j.value("time", 0.0f);
	k.inTangent = j.value("inTangent", 0.0f);
	k.outTangent = j.value("outTangent", 0.0f);
}
void ONEngine::to_json(nlohmann::json& j, const GradientColorKey& k) {
	j = nlohmann::json{
		{"color", k.color},
		{"time", k.time},
		{"inTangent", k.inTangent},
		{"outTangent", k.outTangent}
	};
}

void ONEngine::from_json(const nlohmann::json& j, GradientAlphaKey& k) {
	k.alpha = j.value("alpha", 1.0f);
	k.time = j.value("time", 0.0f);
	k.inTangent = j.value("inTangent", 0.0f);
	k.outTangent = j.value("outTangent", 0.0f);
}
void ONEngine::to_json(nlohmann::json& j, const GradientAlphaKey& k) {
	j = nlohmann::json{
		{"alpha", k.alpha},
		{"time", k.time},
		{"inTangent", k.inTangent},
		{"outTangent", k.outTangent}
	};
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystemGradient& g) {
	g.colorKeys = j.value("colorKeys", std::vector<GradientColorKey>());
	g.alphaKeys = j.value("alphaKeys", std::vector<GradientAlphaKey>());
}
void ONEngine::to_json(nlohmann::json& j, const ParticleSystemGradient& g) {
	j = nlohmann::json{ {"colorKeys", g.colorKeys}, {"alphaKeys", g.alphaKeys} };
}

void ONEngine::from_json(const nlohmann::json& j, MinMaxGradient& m) {
	m.state = static_cast<MinMaxState>(j.value("state", static_cast<uint8_t>(MinMaxState::Constant)));
	m.gradient = j.value("gradient", ParticleSystemGradient());
	m.gradientMin = j.value("gradientMin", ParticleSystemGradient());
	m.gradientMax = j.value("gradientMax", ParticleSystemGradient());
}
void ONEngine::to_json(nlohmann::json& j, const MinMaxGradient& m) {
	j = nlohmann::json{
		{"state", static_cast<uint8_t>(m.state)},
		{"gradient", m.gradient},
		{"gradientMin", m.gradientMin},
		{"gradientMax", m.gradientMax}
	};
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystem& p) {
	p.enable = j.value("enable", 1);
	if (j.contains("main")) p.main = j.at("main").get<ParticleSystemMain>();
	if (j.contains("emission")) p.emission = j.at("emission").get<ParticleSystemEmission>();
	if (j.contains("shape")) p.shape = j.at("shape").get<ParticleSystemShape>();
	if (j.contains("colorOverLifetime")) p.colorOverLifetime = j.at("colorOverLifetime").get<ParticleSystemColorOverLifetime>();
	if (j.contains("sizeOverLifetime")) p.sizeOverLifetime = j.at("sizeOverLifetime").get<ParticleSystemSizeOverLifetime>();
	if (j.contains("velocityOverLifetime")) p.velocityOverLifetime = j.at("velocityOverLifetime").get<ParticleSystemVelocityOverLifetime>();
	if (j.contains("renderer")) p.renderer = j.at("renderer").get<ParticleSystemRenderer>();
}

void ONEngine::to_json(nlohmann::json& j, const ParticleSystem& p) {
	j = nlohmann::json{
		{ "type", "ParticleSystem" },
		{ "enable", p.enable },
		{ "main", p.main },
		{ "emission", p.emission },
		{ "shape", p.shape },
		{ "colorOverLifetime", p.colorOverLifetime },
		{ "sizeOverLifetime", p.sizeOverLifetime },
		{ "velocityOverLifetime", p.velocityOverLifetime },
		{ "renderer", p.renderer },
	};
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystem2D& p) {
	p.enable = j.value("enable", 1);
	if (j.contains("main")) p.main = j.at("main").get<ParticleSystemMain>();
	if (j.contains("emission")) p.emission = j.at("emission").get<ParticleSystemEmission>();
	if (j.contains("shape")) p.shape = j.at("shape").get<ParticleSystemShape>();
	if (j.contains("colorOverLifetime")) p.colorOverLifetime = j.at("colorOverLifetime").get<ParticleSystemColorOverLifetime>();
	if (j.contains("sizeOverLifetime")) p.sizeOverLifetime = j.at("sizeOverLifetime").get<ParticleSystemSizeOverLifetime>();
	if (j.contains("velocityOverLifetime")) p.velocityOverLifetime = j.at("velocityOverLifetime").get<ParticleSystemVelocityOverLifetime>();
	if (j.contains("renderer")) p.renderer = j.at("renderer").get<ParticleSystemRenderer>();
	if (j.contains("textureSheetAnimation")) {
		auto& ta = j.at("textureSheetAnimation");
		p.textureSheetAnimation.enabled = ta.value("enabled", false);
		p.textureSheetAnimation.tilesX = ta.value("tilesX", 1);
		p.textureSheetAnimation.tilesY = ta.value("tilesY", 1);
		p.textureSheetAnimation.fps = ta.value("fps", 12.0f);
	}
}

void ONEngine::to_json(nlohmann::json& j, const ParticleSystem2D& p) {
	j = nlohmann::json{
		{ "type", "ParticleSystem2D" },
		{ "enable", p.enable },
		{ "main", p.main },
		{ "emission", p.emission },
		{ "shape", p.shape },
		{ "colorOverLifetime", p.colorOverLifetime },
		{ "sizeOverLifetime", p.sizeOverLifetime },
		{ "velocityOverLifetime", p.velocityOverLifetime },
		{ "renderer", p.renderer },
		{ "textureSheetAnimation", nlohmann::json{{ "enabled", p.textureSheetAnimation.enabled }, { "tilesX", p.textureSheetAnimation.tilesX }, { "tilesY", p.textureSheetAnimation.tilesY }, { "fps", p.textureSheetAnimation.fps }} },
	};
}

void ONEngine::from_json(const nlohmann::json& j, MinMaxFloat& m) {
	m.state = static_cast<MinMaxState>(j.value("state", static_cast<uint8_t>(MinMaxState::Constant)));
	m.constant = j.value("constant", 0.0f);
	m.minVal = j.value("min", 0.0f);
	m.maxVal = j.value("max", 1.0f);
}

void ONEngine::to_json(nlohmann::json& j, const MinMaxFloat& m) {
	j = nlohmann::json{
		{ "state", static_cast<uint8_t>(m.state) },
		{ "constant", m.constant },
		{ "min", m.minVal },
		{ "max", m.maxVal }
	};
}

void ONEngine::from_json(const nlohmann::json& j, MinMaxColor& m) {
	m.state = static_cast<MinMaxState>(j.value("state", static_cast<uint8_t>(MinMaxState::Constant)));
	m.constant = j.value("constant", Color::kWhite);
	m.minVal = j.value("min", Color::kWhite);
	m.maxVal = j.value("max", Color::kWhite);
}

void ONEngine::to_json(nlohmann::json& j, const MinMaxColor& m) {
	j = nlohmann::json{
		{ "state", static_cast<uint8_t>(m.state) },
		{ "constant", m.constant },
		{ "min", m.minVal },
		{ "max", m.maxVal }
	};
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystemMain& m) {
	m.duration = j.value("duration", 5.0f);
	m.looping = j.value("looping", true);
	m.prewarm = j.value("prewarm", false);
	m.startDelay = j.value("startDelay", MinMaxFloat(0.0f));
	m.startLifetime = j.value("startLifetime", MinMaxFloat(5.0f));
	m.startSpeed = j.value("startSpeed", MinMaxFloat(5.0f));
	m.startSize = j.value("startSize", MinMaxFloat(1.0f));
	m.startRotation = j.value("startRotation", MinMaxFloat(0.0f));
	m.startColor = j.value("startColor", MinMaxColor(Color::kWhite));
	m.gravityModifier = j.value("gravityModifier", 0.0f);
	m.simulationSpace = static_cast<SimulationSpace>(j.value("simulationSpace", static_cast<uint8_t>(SimulationSpace::World)));
	m.maxParticles = j.value("maxParticles", 1000);
	m.playOnAwake = j.value("playOnAwake", true);
}

void ONEngine::to_json(nlohmann::json& j, const ParticleSystemMain& m) {
	j = nlohmann::json{
		{ "duration", m.duration },
		{ "looping", m.looping },
		{ "prewarm", m.prewarm },
		{ "startDelay", m.startDelay },
		{ "startLifetime", m.startLifetime },
		{ "startSpeed", m.startSpeed },
		{ "startSize", m.startSize },
		{ "startRotation", m.startRotation },
		{ "startColor", m.startColor },
		{ "gravityModifier", m.gravityModifier },
		{ "simulationSpace", static_cast<uint8_t>(m.simulationSpace) },
		{ "maxParticles", m.maxParticles },
		{ "playOnAwake", m.playOnAwake }
	};
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystemEmission& e) {
	e.enabled = j.value("enabled", true);
	e.rateOverTime = j.value("rateOverTime", 10.0f);
	e.rateOverDistance = j.value("rateOverDistance", 0.0f);
	e.bursts = j.value("bursts", std::vector<ParticleSystemEmission::Burst>());
}

void ONEngine::to_json(nlohmann::json& j, const ParticleSystemEmission& e) {
	j = nlohmann::json{
		{ "enabled", e.enabled },
		{ "rateOverTime", e.rateOverTime },
		{ "rateOverDistance", e.rateOverDistance },
		{ "bursts", e.bursts }
	};
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystemEmission::Burst& b) {
	b.time = j.value("time", 0.0f);
	b.count = j.value("count", 30);
	b.cycles = j.value("cycles", 1);
	b.interval = j.value("interval", 0.01f);
	b.probability = j.value("probability", 1.0f);
}

void ONEngine::to_json(nlohmann::json& j, const ParticleSystemEmission::Burst& b) {
	j = nlohmann::json{
		{ "time", b.time },
		{ "count", b.count },
		{ "cycles", b.cycles },
		{ "interval", b.interval },
		{ "probability", b.probability }
	};
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystemShape& s) {
	s.enabled = j.value("enabled", true);
	s.type = static_cast<ParticleSystemShapeType>(j.value("type", static_cast<uint8_t>(ParticleSystemShapeType::Sphere)));
	s.radius = j.value("radius", 1.0f);
	s.radiusThickness = j.value("radiusThickness", 1.0f);
	s.arc = j.value("arc", 360.0f);
	s.angle = j.value("angle", 25.0f);
	s.boxScale = j.value("boxScale", Vector3(1.0f, 1.0f, 1.0f));
}

void ONEngine::to_json(nlohmann::json& j, const ParticleSystemShape& s) {
	j = nlohmann::json{
		{ "enabled", s.enabled },
		{ "type", static_cast<uint8_t>(s.type) },
		{ "radius", s.radius },
		{ "radiusThickness", s.radiusThickness },
		{ "arc", s.arc },
		{ "angle", s.angle },
		{ "boxScale", s.boxScale }
	};
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystemRenderer& r) {
	r.renderMode = static_cast<ParticleSystemRenderer::RenderMode>(j.value("renderMode", static_cast<uint8_t>(ParticleSystemRenderer::RenderMode::Billboard)));
	r.blendMode = static_cast<ParticleSystemRenderer::BlendMode>(j.value("blendMode", static_cast<uint8_t>(ParticleSystemRenderer::BlendMode::Normal)));
	r.flipMode = static_cast<FlipMode>(j.value("flipMode", static_cast<uint8_t>(FlipMode::None)));
	r.materialGuid = j.value("materialGuid", "");
	r.meshGuid = j.value("meshGuid", "");
}

void ONEngine::to_json(nlohmann::json& j, const ParticleSystemRenderer& r) {
	j = nlohmann::json{
		{ "renderMode", static_cast<uint8_t>(r.renderMode) },
		{ "blendMode", static_cast<uint8_t>(r.blendMode) },
		{ "flipMode", static_cast<uint8_t>(r.flipMode) },
		{ "materialGuid", r.materialGuid },
		{ "meshGuid", r.meshGuid }
	};
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystemColorOverLifetime& c) {
	c.enabled = j.value("enabled", false);
	c.color = j.value("color", MinMaxGradient());
}

void ONEngine::to_json(nlohmann::json& j, const ParticleSystemColorOverLifetime& c) {
	j = nlohmann::json{ { "enabled", c.enabled }, { "color", c.color } };
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystemSizeOverLifetime& s) {
	s.enabled = j.value("enabled", false);
	s.size = j.value("size", MinMaxCurve());
}

void ONEngine::to_json(nlohmann::json& j, const ParticleSystemSizeOverLifetime& s) {
	j = nlohmann::json{ { "enabled", s.enabled }, { "size", s.size } };
}

void ONEngine::from_json(const nlohmann::json& j, ParticleSystemVelocityOverLifetime& v) {
	v.enabled = j.value("enabled", false);
	v.x = j.value("x", MinMaxCurve());
	v.y = j.value("y", MinMaxCurve());
	v.z = j.value("z", MinMaxCurve());
	v.speedModifier = j.value("speedModifier", MinMaxCurve());
	v.space = static_cast<SimulationSpace>(j.value("space", static_cast<uint8_t>(SimulationSpace::Local)));
}

void ONEngine::to_json(nlohmann::json& j, const ParticleSystemVelocityOverLifetime& v) {
	j = nlohmann::json{
		{ "enabled", v.enabled },
		{ "x", v.x },
		{ "y", v.y },
		{ "z", v.z },
		{ "speedModifier", v.speedModifier },
		{ "space", static_cast<uint8_t>(v.space) }
	};
}

