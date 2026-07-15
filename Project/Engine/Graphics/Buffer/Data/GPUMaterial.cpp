#include "GPUMaterial.h"

using namespace ONEngine;

/// engine
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/Editor/Commands/ComponentEditCommands/ComponentJsonConverter.h"
#include "Engine/Asset/Collection/AssetCollection.h"



/// ------------------------------------------------
/// GPUMaterial Json変換
/// ------------------------------------------------

void ONEngine::to_json(nlohmann::json& j, const GPUMaterial& material) {
	j = nlohmann::json{
		{ "uvTransform", material.uvTransform },
		{ "baseColor", material.baseColor },
		{ "outlineColor", material.outlineColor },
		{ "postEffectFlags", material.postEffectFlags },
		{ "entityId", material.entityId },
		{ "baseTextureId", material.baseTextureId },
		{ "normalTextureId", material.normalTextureId },
		{ "bloomIntensity", material.bloomIntensity },
		{ "bloomThreshold", material.bloomThreshold }
	};
}

void ONEngine::from_json(const nlohmann::json& j, GPUMaterial& material) {
	material.uvTransform     = j.value("uvTransform", UVTransform{});
	material.baseColor       = j.value("baseColor", Vector4::White);
	material.outlineColor   = j.value("outlineColor", Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	material.postEffectFlags = j.value("postEffectFlags", PostEffectFlags_None);
	material.entityId        = j.value("entityId", 0);
	material.baseTextureId   = j.value("baseTextureId", -1);
	material.normalTextureId = j.value("normalTextureId", -1);
	material.bloomIntensity  = j.value("bloomIntensity", 1.0f);
	material.bloomThreshold  = j.value("bloomThreshold", 0.8f);
}
