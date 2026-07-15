#include "ScreenPostEffectTag.h"

/// std
#include <vector>
#include <string>
#include <array>
#include <algorithm>

/// editor
#include "Engine/Editor/Commands/ImGuiCommand/ImGuiCommand.h"

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Array/ComponentArray.h"


/// external
#include <imgui.h>


using namespace ONEngine;

namespace {

	const std::vector<std::string> gFlagNames = {
		"Grayscale",
		"Radial Blur",
		"Fisheye",
		"Water Distortion",
		"Water Depth Fog & Vignette",
		"Water Color Grading & Absorption",
		"Water Caustics & Light Shafts",
		"Pixelate"
	};

} /// namespace

void ScreenPostEffectTag::SetPostEffectEnable(PostEffectType type, bool isEnable) {
	flags_.flags[static_cast<size_t>(type)] = isEnable;
}

bool ScreenPostEffectTag::GetPostEffectEnable(PostEffectType type) const {
	return flags_.flags[static_cast<size_t>(type)];
}

void ScreenPostEffectTag::SetFisheyeStrength(float strength) {
	flags_.fisheyeStrength = strength;
}

float ScreenPostEffectTag::GetFisheyeStrength() const {
	return flags_.fisheyeStrength;
}

void ScreenPostEffectTag::SetFisheyeScale(float scale) {
	flags_.fisheyeScale = scale;
}

float ScreenPostEffectTag::GetFisheyeScale() const {
	return flags_.fisheyeScale;
}

// Wave Distortion
void ScreenPostEffectTag::SetWaterDistortionStrength(float strength) { flags_.distortionStrength = strength; }
float ScreenPostEffectTag::GetWaterDistortionStrength() const { return flags_.distortionStrength; }
void ScreenPostEffectTag::SetWaterDistortionSpeed(float speed) { flags_.distortionSpeed = speed; }
float ScreenPostEffectTag::GetWaterDistortionSpeed() const { return flags_.distortionSpeed; }
void ScreenPostEffectTag::SetWaterDistortionFrequency(float freq) { flags_.distortionFrequency = freq; }
float ScreenPostEffectTag::GetWaterDistortionFrequency() const { return flags_.distortionFrequency; }

// Depth Fog & Vignette
void ScreenPostEffectTag::SetWaterFogColor(const Vector3& color) { flags_.fogColor = color; }
Vector3 ScreenPostEffectTag::GetWaterFogColor() const { return flags_.fogColor; }
void ScreenPostEffectTag::SetWaterFogDensity(float density) { flags_.fogDensity = density; }
float ScreenPostEffectTag::GetWaterFogDensity() const { return flags_.fogDensity; }
void ScreenPostEffectTag::SetWaterFogWaterSurfaceY(float y) { flags_.fogWaterSurfaceY = y; }
float ScreenPostEffectTag::GetWaterFogWaterSurfaceY() const { return flags_.fogWaterSurfaceY; }
void ScreenPostEffectTag::SetWaterVignetteStrength(float strength) { flags_.vignetteStrength = strength; }
float ScreenPostEffectTag::GetWaterVignetteStrength() const { return flags_.vignetteStrength; }

// Color Grading & Absorption
void ScreenPostEffectTag::SetWaterAbsorptionCoefficients(const Vector3& coeffs) { flags_.absorptionCoefficients = coeffs; }
Vector3 ScreenPostEffectTag::GetWaterAbsorptionCoefficients() const { return flags_.absorptionCoefficients; }
void ScreenPostEffectTag::SetWaterContrast(float contrast) { flags_.contrast = contrast; }
float ScreenPostEffectTag::GetWaterContrast() const { return flags_.contrast; }
void ScreenPostEffectTag::SetWaterSaturation(float sat) { flags_.saturation = sat; }
float ScreenPostEffectTag::GetWaterSaturation() const { return flags_.saturation; }
void ScreenPostEffectTag::SetWaterColorFilter(const Vector3& filter) { flags_.colorFilter = filter; }
Vector3 ScreenPostEffectTag::GetWaterColorFilter() const { return flags_.colorFilter; }

// Caustics & Light Shafts
void ScreenPostEffectTag::SetWaterCausticsScale(float scale) { flags_.causticsScale = scale; }
float ScreenPostEffectTag::GetWaterCausticsScale() const { return flags_.causticsScale; }
void ScreenPostEffectTag::SetWaterCausticsSpeed(float speed) { flags_.causticsSpeed = speed; }
float ScreenPostEffectTag::GetWaterCausticsSpeed() const { return flags_.causticsSpeed; }
void ScreenPostEffectTag::SetWaterCausticsIntensity(float intensity) { flags_.causticsIntensity = intensity; }
float ScreenPostEffectTag::GetWaterCausticsIntensity() const { return flags_.causticsIntensity; }
void ScreenPostEffectTag::SetWaterLightShaftsIntensity(float intensity) { flags_.lightShaftsIntensity = intensity; }
float ScreenPostEffectTag::GetWaterLightShaftsIntensity() const { return flags_.lightShaftsIntensity; }
void ScreenPostEffectTag::SetWaterLightDirection(const Vector3& dir) { flags_.lightDirection = dir; }
Vector3 ScreenPostEffectTag::GetWaterLightDirection() const { return flags_.lightDirection; }

// Pixelate
void ScreenPostEffectTag::SetPixelSizeX(float size) { flags_.pixelSizeX = size; }
float ScreenPostEffectTag::GetPixelSizeX() const { return flags_.pixelSizeX; }
void ScreenPostEffectTag::SetPixelSizeY(float size) { flags_.pixelSizeY = size; }
float ScreenPostEffectTag::GetPixelSizeY() const { return flags_.pixelSizeY; }

// Size limitation
void ScreenPostEffectTag::SetPostEffectWidth(int32_t width) { flags_.postEffectWidth = width; }
int32_t ScreenPostEffectTag::GetPostEffectWidth() const { return flags_.postEffectWidth; }
void ScreenPostEffectTag::SetPostEffectHeight(int32_t height) { flags_.postEffectHeight = height; }
int32_t ScreenPostEffectTag::GetPostEffectHeight() const { return flags_.postEffectHeight; }

Vector2 ScreenPostEffectTag::GetDispatchSize(ECSGroup* ecsGroup, EntityComponentSystem* entityComponentSystem) {
	ECSGroup* activeGroup = ecsGroup ? ecsGroup : entityComponentSystem->GetCurrentGroup();
	if (activeGroup) {
		ComponentArray<ScreenPostEffectTag>* screenPostEffectTagArray = activeGroup->GetComponentArray<ScreenPostEffectTag>();
		if (screenPostEffectTagArray && !screenPostEffectTagArray->GetUsedComponents().empty()) {
			for (auto& comp : screenPostEffectTagArray->GetUsedComponents()) {
				if (comp && comp->enable) {
					float w = comp->GetPostEffectWidth() > 0 ? static_cast<float>(comp->GetPostEffectWidth()) : EngineConfig::kWindowSize.x;
					float h = comp->GetPostEffectHeight() > 0 ? static_cast<float>(comp->GetPostEffectHeight()) : EngineConfig::kWindowSize.y;
					return Vector2(w, h);
				}
			}
		}
	}
	return EngineConfig::kWindowSize;
}

void ScreenPostEffectTag::SetPostEffectStartX(int32_t x) { flags_.postEffectStartX = x; }
int32_t ScreenPostEffectTag::GetPostEffectStartX() const { return flags_.postEffectStartX; }
void ScreenPostEffectTag::SetPostEffectStartY(int32_t y) { flags_.postEffectStartY = y; }
int32_t ScreenPostEffectTag::GetPostEffectStartY() const { return flags_.postEffectStartY; }
void ScreenPostEffectTag::SetPostEffectPivot(int32_t pivot) { flags_.postEffectPivot = pivot; }
int32_t ScreenPostEffectTag::GetPostEffectPivot() const { return flags_.postEffectPivot; }

Vector2 ScreenPostEffectTag::GetDispatchStartOffset(ECSGroup* ecsGroup, EntityComponentSystem* entityComponentSystem) {
	ECSGroup* activeGroup = ecsGroup ? ecsGroup : entityComponentSystem->GetCurrentGroup();
	if (activeGroup) {
		ComponentArray<ScreenPostEffectTag>* screenPostEffectTagArray = activeGroup->GetComponentArray<ScreenPostEffectTag>();
		if (screenPostEffectTagArray && !screenPostEffectTagArray->GetUsedComponents().empty()) {
			for (auto& comp : screenPostEffectTagArray->GetUsedComponents()) {
				if (comp && comp->enable) {
					int32_t startX = comp->GetPostEffectStartX();
					int32_t startY = comp->GetPostEffectStartY();
					if (comp->GetPostEffectPivot() == 1) { // 1: Center
						int32_t width = comp->GetPostEffectWidth();
						int32_t height = comp->GetPostEffectHeight();
						if (width > 0) startX -= width / 2;
						if (height > 0) startY -= height / 2;
					}
					startX = std::max(0, startX);
					startY = std::max(0, startY);
					return Vector2(static_cast<float>(startX), static_cast<float>(startY));
				}
			}
		}
	}
	return Vector2(0.0f, 0.0f);
}




void ComponentDebug::ScreenPostEffectTagDebug(ScreenPostEffectTag* component) {
	if (!component) {
		return;
	}

	for (size_t i = 0; i < component->flags_.flags.size(); ++i) {
		Editor::ImMathf::Checkbox(gFlagNames[i], &component->flags_.flags[i]);
	}

	if (component->flags_.flags[PostEffectType_Fisheye]) {
		ImGui::Separator();
		ImGui::Text("Fisheye Settings");

		Editor::ImMathf::SliderFloat("Distortion Strength##Fisheye", &component->flags_.fisheyeStrength, -1.0f, 1.0f);
		Editor::ImMathf::SliderFloat("View Scale##Fisheye", &component->flags_.fisheyeScale, 0.1f, 2.0f);
	}

	if (component->flags_.flags[PostEffectType_WaterDistortion]) {
		ImGui::Separator();
		ImGui::Text("Water Distortion Settings");

		Editor::ImMathf::SliderFloat("Strength##Distortion", &component->flags_.distortionStrength, 0.0f, 0.1f, "%.4f");
		Editor::ImMathf::SliderFloat("Speed##Distortion", &component->flags_.distortionSpeed, 0.0f, 10.0f);
		Editor::ImMathf::SliderFloat("Frequency##Distortion", &component->flags_.distortionFrequency, 0.1f, 50.0f);
	}

	if (component->flags_.flags[PostEffectType_WaterDepthFogVignette]) {
		ImGui::Separator();
		ImGui::Text("Water Depth Fog & Vignette Settings");

		Editor::ImMathf::ColorEdit3("Fog Color##DepthFog", &component->flags_.fogColor);
		Editor::ImMathf::SliderFloat("Fog Density##DepthFog", &component->flags_.fogDensity, 0.0f, 0.1f, "%.4f");
		Editor::ImMathf::SliderFloat("Water Surface Y##DepthFog", &component->flags_.fogWaterSurfaceY, -100.0f, 100.0f);
		Editor::ImMathf::SliderFloat("Vignette Strength##DepthFog", &component->flags_.vignetteStrength, 0.0f, 2.0f);
	}

	if (component->flags_.flags[PostEffectType_WaterColorGrading]) {
		ImGui::Separator();
		ImGui::Text("Water Color Grading & Absorption Settings");

		Editor::ImMathf::SliderFloat3("Absorption Coeffs##Absorption", &component->flags_.absorptionCoefficients, 0.0f, 1.0f, "%.3f");
		Editor::ImMathf::SliderFloat("Contrast##ColorGrading", &component->flags_.contrast, 0.5f, 2.0f);
		Editor::ImMathf::SliderFloat("Saturation##ColorGrading", &component->flags_.saturation, 0.0f, 2.0f);
		Editor::ImMathf::ColorEdit3("Color Filter##ColorGrading", &component->flags_.colorFilter);
	}

	if (component->flags_.flags[PostEffectType_WaterCausticsLightShafts]) {
		ImGui::Separator();
		ImGui::Text("Water Caustics & Light Shafts Settings");

		Editor::ImMathf::SliderFloat("Caustics Scale##Caustics", &component->flags_.causticsScale, 0.01f, 5.0f);
		Editor::ImMathf::SliderFloat("Caustics Speed##Caustics", &component->flags_.causticsSpeed, 0.0f, 5.0f);
		Editor::ImMathf::SliderFloat("Caustics Intensity##Caustics", &component->flags_.causticsIntensity, 0.0f, 2.0f);
		Editor::ImMathf::SliderFloat("Light Shafts Intensity##Caustics", &component->flags_.lightShaftsIntensity, 0.0f, 2.0f);
		Editor::ImMathf::SliderFloat3("Light Direction##Caustics", &component->flags_.lightDirection, -1.0f, 1.0f);
	}

	if (component->flags_.flags[PostEffectType_Pixelate]) {
		ImGui::Separator();
		ImGui::Text("Pixelate Settings");

		Editor::ImMathf::SliderFloat("Pixel Size X##Pixelate", &component->flags_.pixelSizeX, 1.0f, 64.0f, "%.1f");
		Editor::ImMathf::SliderFloat("Pixel Size Y##Pixelate", &component->flags_.pixelSizeY, 1.0f, 64.0f, "%.1f");
	}

	ImGui::Separator();
	ImGui::Text("Global PostEffect Resolution Settings");
	ImGui::Text("Set <= 0 for full screen resolution");
	
	int width = component->GetPostEffectWidth();
	int height = component->GetPostEffectHeight();
	if (ImGui::InputInt("Width Limit", &width)) {
		component->SetPostEffectWidth(width);
	}
	if (ImGui::InputInt("Height Limit", &height)) {
		component->SetPostEffectHeight(height);
	}

	int startX = component->GetPostEffectStartX();
	int startY = component->GetPostEffectStartY();
	int pivot = component->GetPostEffectPivot();
	if (ImGui::InputInt("Start X Offset", &startX)) {
		component->SetPostEffectStartX(startX);
	}
	if (ImGui::InputInt("Start Y Offset", &startY)) {
		component->SetPostEffectStartY(startY);
	}
	const char* pivotItems[] = { "Top-Left", "Center" };
	if (ImGui::Combo("Pivot Mode", &pivot, pivotItems, IM_ARRAYSIZE(pivotItems))) {
		component->SetPostEffectPivot(pivot);
	}
}

void ONEngine::from_json(const nlohmann::json& j, ScreenPostEffectTag& c) {
	if (j.contains("enable")) {
		c.enable = j["enable"].get<int>();
	}
	if (j.contains("id")) {
		c.id = j["id"].get<uint32_t>();
	}
	// Handle post effect flags if they are present in the JSON
	if (j.contains("postEffects")) {
		for (const auto& effect : j["postEffects"]) {
			auto type = effect["type"].get<int>();
			bool enabled = effect["enabled"].get<bool>();
			c.SetPostEffectEnable(PostEffectType(type), enabled);
		}
	}
	if (j.contains("fisheyeStrength")) {
		c.SetFisheyeStrength(j["fisheyeStrength"].get<float>());
	}
	if (j.contains("fisheyeScale")) {
		c.SetFisheyeScale(j["fisheyeScale"].get<float>());
	}
	
	// Water Distortion
	if (j.contains("distortionStrength")) c.SetWaterDistortionStrength(j["distortionStrength"].get<float>());
	if (j.contains("distortionSpeed")) c.SetWaterDistortionSpeed(j["distortionSpeed"].get<float>());
	if (j.contains("distortionFrequency")) c.SetWaterDistortionFrequency(j["distortionFrequency"].get<float>());

	// Depth Fog
	if (j.contains("fogColor")) {
		auto color = j["fogColor"];
		c.SetWaterFogColor(Vector3(color[0].get<float>(), color[1].get<float>(), color[2].get<float>()));
	}
	if (j.contains("fogDensity")) c.SetWaterFogDensity(j["fogDensity"].get<float>());
	if (j.contains("fogWaterSurfaceY")) c.SetWaterFogWaterSurfaceY(j["fogWaterSurfaceY"].get<float>());
	if (j.contains("vignetteStrength")) c.SetWaterVignetteStrength(j["vignetteStrength"].get<float>());

	// Color Grading
	if (j.contains("absorptionCoefficients")) {
		auto coeffs = j["absorptionCoefficients"];
		c.SetWaterAbsorptionCoefficients(Vector3(coeffs[0].get<float>(), coeffs[1].get<float>(), coeffs[2].get<float>()));
	}
	if (j.contains("contrast")) c.SetWaterContrast(j["contrast"].get<float>());
	if (j.contains("saturation")) c.SetWaterSaturation(j["saturation"].get<float>());
	if (j.contains("colorFilter")) {
		auto filter = j["colorFilter"];
		c.SetWaterColorFilter(Vector3(filter[0].get<float>(), filter[1].get<float>(), filter[2].get<float>()));
	}

	// Caustics
	if (j.contains("causticsScale")) c.SetWaterCausticsScale(j["causticsScale"].get<float>());
	if (j.contains("causticsSpeed")) c.SetWaterCausticsSpeed(j["causticsSpeed"].get<float>());
	if (j.contains("causticsIntensity")) c.SetWaterCausticsIntensity(j["causticsIntensity"].get<float>());
	if (j.contains("lightShaftsIntensity")) c.SetWaterLightShaftsIntensity(j["lightShaftsIntensity"].get<float>());
	if (j.contains("lightDirection")) {
		auto dir = j["lightDirection"];
		c.SetWaterLightDirection(Vector3(dir[0].get<float>(), dir[1].get<float>(), dir[2].get<float>()));
	}

	// Pixelate
	if (j.contains("pixelSizeX")) c.SetPixelSizeX(j["pixelSizeX"].get<float>());
	if (j.contains("pixelSizeY")) c.SetPixelSizeY(j["pixelSizeY"].get<float>());

	// Size limitation
	if (j.contains("postEffectWidth")) c.SetPostEffectWidth(j["postEffectWidth"].get<int32_t>());
	if (j.contains("postEffectHeight")) c.SetPostEffectHeight(j["postEffectHeight"].get<int32_t>());

	// Offset & Pivot
	if (j.contains("postEffectStartX")) c.SetPostEffectStartX(j["postEffectStartX"].get<int32_t>());
	if (j.contains("postEffectStartY")) c.SetPostEffectStartY(j["postEffectStartY"].get<int32_t>());
	if (j.contains("postEffectPivot")) c.SetPostEffectPivot(j["postEffectPivot"].get<int32_t>());
}

void ONEngine::to_json(nlohmann::json& j, const ScreenPostEffectTag& c) {
	j["type"] = "ScreenPostEffectTag";
	j["enable"] = c.enable;
	j["id"] = c.id;
	// Serialize post effect flags
	j["postEffects"] = nlohmann::json::array();
	for (size_t i = 0; i < c.flags_.flags.size(); ++i) {
		j["postEffects"].push_back({
			{ "type", i },
			{ "enabled", c.flags_.flags[i] }
			});
	}
	j["fisheyeStrength"] = c.GetFisheyeStrength();
	j["fisheyeScale"] = c.GetFisheyeScale();

	// Water Distortion
	j["distortionStrength"] = c.GetWaterDistortionStrength();
	j["distortionSpeed"] = c.GetWaterDistortionSpeed();
	j["distortionFrequency"] = c.GetWaterDistortionFrequency();

	// Depth Fog
	Vector3 fogColor = c.GetWaterFogColor();
	j["fogColor"] = { fogColor.x, fogColor.y, fogColor.z };
	j["fogDensity"] = c.GetWaterFogDensity();
	j["fogWaterSurfaceY"] = c.GetWaterFogWaterSurfaceY();
	j["vignetteStrength"] = c.GetWaterVignetteStrength();

	// Color Grading
	Vector3 absorption = c.GetWaterAbsorptionCoefficients();
	j["absorptionCoefficients"] = { absorption.x, absorption.y, absorption.z };
	j["contrast"] = c.GetWaterContrast();
	j["saturation"] = c.GetWaterSaturation();
	Vector3 filter = c.GetWaterColorFilter();
	j["colorFilter"] = { filter.x, filter.y, filter.z };

	// Caustics
	j["causticsScale"] = c.GetWaterCausticsScale();
	j["causticsSpeed"] = c.GetWaterCausticsSpeed();
	j["causticsIntensity"] = c.GetWaterCausticsIntensity();
	j["lightShaftsIntensity"] = c.GetWaterLightShaftsIntensity();
	Vector3 dir = c.GetWaterLightDirection();
	j["lightDirection"] = { dir.x, dir.y, dir.z };

	// Pixelate
	j["pixelSizeX"] = c.GetPixelSizeX();
	j["pixelSizeY"] = c.GetPixelSizeY();

	// Size limitation
	j["postEffectWidth"] = c.GetPostEffectWidth();
	j["postEffectHeight"] = c.GetPostEffectHeight();

	// Offset & Pivot
	j["postEffectStartX"] = c.GetPostEffectStartX();
	j["postEffectStartY"] = c.GetPostEffectStartY();
	j["postEffectPivot"] = c.GetPostEffectPivot();
}
