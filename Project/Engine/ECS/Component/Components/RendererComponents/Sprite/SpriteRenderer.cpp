#include "SpriteRenderer.h"

/// external
#include <imgui.h>
#include <format>

/// engine
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/Editor/Commands/ComponentEditCommands/ComponentJsonConverter.h"
#include "Engine/Asset/Collection/AssetCollection.h"

/// editor
#include "Engine/Editor/Math/ImGuiMath.h"
#include "Engine/Editor/Math/AssetDebugger.h"

#include "Engine/Core/DirectX12/Manager/DxManager.h"

using namespace ONEngine;

/// /////////////////////////////////////////////////////////////
/// デバッグ用のSpriteRenderer
/// /////////////////////////////////////////////////////////////

void ComponentDebug::SpriteDebug(SpriteRenderer* sr, Asset::AssetCollection* assetCollection) {
	if (!sr) {
		return;
	}

	float indentValue = 1.8f;
	ImGui::Indent(indentValue);

	Editor::ImMathf::MaterialEdit("Material", &sr->material_, assetCollection, false);

	bool isPerfect = sr->IsPixelPerfect();
	if (ImGui::Checkbox("Pixel Perfect", &isPerfect)) {
		sr->SetPixelPerfect(isPerfect);
	}

	ImGui::Unindent(indentValue);
}


void ONEngine::to_json(nlohmann::json& j, const SpriteRenderer& sr) {
	j = nlohmann::json{
		{ "type", "SpriteRenderer" },
		{ "enable", sr.enable },
		{ "material", sr.material_ },
		{ "isPixelPerfect", sr.isPixelPerfect_ }
	};
}

void ONEngine::from_json(const nlohmann::json& j, SpriteRenderer& sr) {
	sr.enable = j.value("enable", static_cast<int>(true));
	sr.material_ = j.value("material", Asset::Material{});
	sr.isPixelPerfect_ = j.value("isPixelPerfect", false);
}


/// /////////////////////////////////////////////////////////////
/// SpriteRenderer
/// /////////////////////////////////////////////////////////////

SpriteRenderer::SpriteRenderer() {
	gpuMaterial_.baseColor = Vector4::White;
	gpuMaterial_.entityId = 0;
	gpuMaterial_.baseTextureId = 0;
	gpuMaterial_.uvTransform = UVTransform();
	gpuMaterial_.postEffectFlags = 0;

	material_.baseColor = Vector4::White;
	material_.postEffectFlags = 0;
}
SpriteRenderer::~SpriteRenderer() {}

void SpriteRenderer::RenderingSetup(Asset::AssetCollection* assetCollection) {

	gpuMaterial_.baseColor = material_.baseColor;
	gpuMaterial_.postEffectFlags = material_.postEffectFlags;
	/// base texture
	if (material_.HasBaseTexture()) {
		int32_t textureIndex = assetCollection->GetTextureIndexFromGuid(material_.GetBaseTextureGuid());
		if (textureIndex != -1) {
			gpuMaterial_.baseTextureId = textureIndex;
		} else {
			gpuMaterial_.baseTextureId = 0;
			Console::LogWarning(std::format("[SpriteRenderer] Texture not found in AssetCollection. GUID: {}. Falling back to default (Index 0).", material_.GetBaseTextureGuid().ToString()));
		}
	} else {
		gpuMaterial_.baseTextureId = 0;
	}

	gpuMaterial_.uvTransform = material_.uvTransform;
	gpuMaterial_.entityId = owner_ ? static_cast<int32_t>(owner_->GetId()) : 0;
}


void SpriteRenderer::SetColor(const Vector4& color) {
	material_.baseColor = color;
}

void SpriteRenderer::SetUVTransform(const UVTransform& uvTransform) {
	material_.uvTransform = uvTransform;
}

void SpriteRenderer::SetPixelPerfect(bool enable) {
	isPixelPerfect_ = enable;
}

const Vector4& SpriteRenderer::GetColor() const {
	return gpuMaterial_.baseColor;
}

const GPUMaterial& SpriteRenderer::GetGpuMaterial() const {
	return gpuMaterial_;
}

const UVTransform& SpriteRenderer::GetUVTransform() const {
	return material_.uvTransform;
}

bool SpriteRenderer::IsPixelPerfect() const {
	return isPixelPerfect_;
}

Vector2 SpriteRenderer::GetTextureSize(Asset::AssetCollection* assetCollection) const {
	if (material_.HasBaseTexture()) {
		Asset::Texture* texture = assetCollection->GetTextureFromGuid(material_.GetBaseTextureGuid());
		if (texture) {
			return texture->GetTextureSize();
		}
	}
	return Vector2(0.0f, 0.0f);
}


/// ===================================================
/// csで使用するための関数群
/// ===================================================

Vector4 MonoInternalMethods::InternalGetColor(uint64_t nativeHandle) {
	SpriteRenderer* sr = reinterpret_cast<SpriteRenderer*>(nativeHandle);
	if (sr) {
		return sr->GetColor();
	}

	Console::LogError("MonoInternalMethods::InternalGetColor() | native handle is invalid");

	return Vector4();
}

void MonoInternalMethods::InternalSetColor(uint64_t nativeHandle, Vector4 color) {
	SpriteRenderer* sr = reinterpret_cast<SpriteRenderer*>(nativeHandle);
	if (sr) {
		sr->SetColor(color);
		return;
	}

	Console::LogError("MonoInternalMethods::InternalSetColor() | native handle is invalid");
}

Vector2 MonoInternalMethods::InternalGetTextureSize(uint64_t nativeHandle) {
	SpriteRenderer* sr = reinterpret_cast<SpriteRenderer*>(nativeHandle);
	if (sr) {
		auto* assetCollection = Asset::AssetCollection::GetInstance();
		return sr->GetTextureSize(assetCollection);
	}

	Console::LogError("MonoInternalMethods::InternalGetTextureSize() | native handle is invalid");
	return Vector2(0.0f, 0.0f);
}

bool MonoInternalMethods::InternalGetPixelPerfect(uint64_t nativeHandle) {
	SpriteRenderer* sr = reinterpret_cast<SpriteRenderer*>(nativeHandle);
	if (sr) {
		return sr->IsPixelPerfect();
	}
	Console::LogError("MonoInternalMethods::InternalGetPixelPerfect() | native handle is invalid");
	return false;
}

void MonoInternalMethods::InternalSetPixelPerfect(uint64_t nativeHandle, bool enable) {
	SpriteRenderer* sr = reinterpret_cast<SpriteRenderer*>(nativeHandle);
	if (sr) {
		sr->SetPixelPerfect(enable);
		return;
	}
	Console::LogError("MonoInternalMethods::InternalSetPixelPerfect() | native handle is invalid");
}
