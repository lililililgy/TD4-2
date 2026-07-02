#include "ScreenPostEffectTag.h"

/// std
#include <vector>
#include <string>

/// external
#include <imgui.h>


using namespace ONEngine;

namespace {

	struct Flags {
		Flags() : 
			flags(PostEffectType_Count, false), 
			fisheyeStrength(0.15f), 
			fisheyeScale(0.9f),
			distortionStrength(0.015f),
			distortionSpeed(1.0f),
			distortionFrequency(10.0f),
			fogColor(0.0f, 0.3f, 0.6f),
			fogDensity(0.05f),
			fogWaterSurfaceY(0.0f),
			vignetteStrength(1.5f),
			absorptionCoefficients(1.0f, 0.3f, 0.0f),
			contrast(1.1f),
			saturation(0.9f),
			colorFilter(0.4f, 0.8f, 1.0f),
			causticsScale(0.5f),
			causticsSpeed(1.0f),
			causticsIntensity(1.5f),
			lightShaftsIntensity(1.5f),
			lightDirection(0.2f, -0.9f, 0.3f)
		{}
		std::vector<bool> flags;
		std::vector<std::string> flagNames = {
			"Grayscale",
			"Radial Blur",
			"Fisheye",
			"Water Distortion",
			"Water Depth Fog & Vignette",
			"Water Color Grading & Absorption",
			"Water Caustics & Light Shafts"
		};
		float fisheyeStrength;
		float fisheyeScale;

		// Water Distortion
		float distortionStrength;
		float distortionSpeed;
		float distortionFrequency;

		// Depth Fog & Vignette
		Vector3 fogColor;
		float fogDensity;
		float fogWaterSurfaceY;
		float vignetteStrength;

		// Color Grading & Absorption
		Vector3 absorptionCoefficients;
		float contrast;
		float saturation;
		Vector3 colorFilter;

		// Caustics & Light Shafts
		float causticsScale;
		float causticsSpeed;
		float causticsIntensity;
		float lightShaftsIntensity;
		Vector3 lightDirection;
	};

	Flags gFlags;

} /// namespace

void ScreenPostEffectTag::SetPostEffectEnable(PostEffectType type, bool enable) {
	gFlags.flags[static_cast<size_t>(type)] = enable;
}

bool ScreenPostEffectTag::GetPostEffectEnable(PostEffectType type) const {
	return gFlags.flags[static_cast<size_t>(type)];
}

void ScreenPostEffectTag::SetFisheyeStrength(float strength) {
	gFlags.fisheyeStrength = strength;
}

float ScreenPostEffectTag::GetFisheyeStrength() const {
	return gFlags.fisheyeStrength;
}

void ScreenPostEffectTag::SetFisheyeScale(float scale) {
	gFlags.fisheyeScale = scale;
}

float ScreenPostEffectTag::GetFisheyeScale() const {
	return gFlags.fisheyeScale;
}

// Wave Distortion
void ScreenPostEffectTag::SetWaterDistortionStrength(float strength) { gFlags.distortionStrength = strength; }
float ScreenPostEffectTag::GetWaterDistortionStrength() const { return gFlags.distortionStrength; }
void ScreenPostEffectTag::SetWaterDistortionSpeed(float speed) { gFlags.distortionSpeed = speed; }
float ScreenPostEffectTag::GetWaterDistortionSpeed() const { return gFlags.distortionSpeed; }
void ScreenPostEffectTag::SetWaterDistortionFrequency(float freq) { gFlags.distortionFrequency = freq; }
float ScreenPostEffectTag::GetWaterDistortionFrequency() const { return gFlags.distortionFrequency; }

// Depth Fog & Vignette
void ScreenPostEffectTag::SetWaterFogColor(const Vector3& color) { gFlags.fogColor = color; }
Vector3 ScreenPostEffectTag::GetWaterFogColor() const { return gFlags.fogColor; }
void ScreenPostEffectTag::SetWaterFogDensity(float density) { gFlags.fogDensity = density; }
float ScreenPostEffectTag::GetWaterFogDensity() const { return gFlags.fogDensity; }
void ScreenPostEffectTag::SetWaterFogWaterSurfaceY(float y) { gFlags.fogWaterSurfaceY = y; }
float ScreenPostEffectTag::GetWaterFogWaterSurfaceY() const { return gFlags.fogWaterSurfaceY; }
void ScreenPostEffectTag::SetWaterVignetteStrength(float strength) { gFlags.vignetteStrength = strength; }
float ScreenPostEffectTag::GetWaterVignetteStrength() const { return gFlags.vignetteStrength; }

// Color Grading & Absorption
void ScreenPostEffectTag::SetWaterAbsorptionCoefficients(const Vector3& coeffs) { gFlags.absorptionCoefficients = coeffs; }
Vector3 ScreenPostEffectTag::GetWaterAbsorptionCoefficients() const { return gFlags.absorptionCoefficients; }
void ScreenPostEffectTag::SetWaterContrast(float contrast) { gFlags.contrast = contrast; }
float ScreenPostEffectTag::GetWaterContrast() const { return gFlags.contrast; }
void ScreenPostEffectTag::SetWaterSaturation(float sat) { gFlags.saturation = sat; }
float ScreenPostEffectTag::GetWaterSaturation() const { return gFlags.saturation; }
void ScreenPostEffectTag::SetWaterColorFilter(const Vector3& filter) { gFlags.colorFilter = filter; }
Vector3 ScreenPostEffectTag::GetWaterColorFilter() const { return gFlags.colorFilter; }

// Caustics & Light Shafts
void ScreenPostEffectTag::SetWaterCausticsScale(float scale) { gFlags.causticsScale = scale; }
float ScreenPostEffectTag::GetWaterCausticsScale() const { return gFlags.causticsScale; }
void ScreenPostEffectTag::SetWaterCausticsSpeed(float speed) { gFlags.causticsSpeed = speed; }
float ScreenPostEffectTag::GetWaterCausticsSpeed() const { return gFlags.causticsSpeed; }
void ScreenPostEffectTag::SetWaterCausticsIntensity(float intensity) { gFlags.causticsIntensity = intensity; }
float ScreenPostEffectTag::GetWaterCausticsIntensity() const { return gFlags.causticsIntensity; }
void ScreenPostEffectTag::SetWaterLightShaftsIntensity(float intensity) { gFlags.lightShaftsIntensity = intensity; }
float ScreenPostEffectTag::GetWaterLightShaftsIntensity() const { return gFlags.lightShaftsIntensity; }
void ScreenPostEffectTag::SetWaterLightDirection(const Vector3& dir) { gFlags.lightDirection = dir; }
Vector3 ScreenPostEffectTag::GetWaterLightDirection() const { return gFlags.lightDirection; }



void ComponentDebug::ScreenPostEffectTagDebug(ScreenPostEffectTag* component) {
	if (!component) {
		return;
	}

	for (size_t i = 0; i < gFlags.flags.size(); ++i) {
		bool flag = gFlags.flags[i];

		ImGui::Checkbox(gFlags.flagNames[i].c_str(), &flag);

		gFlags.flags[i] = flag;
	}

	if (gFlags.flags[PostEffectType_Fisheye]) {
		ImGui::Separator();
		ImGui::Text("Fisheye Settings");

		float strength = gFlags.fisheyeStrength;
		if (ImGui::SliderFloat("Distortion Strength##Fisheye", &strength, -1.0f, 1.0f)) {
			gFlags.fisheyeStrength = strength;
		}

		float scale = gFlags.fisheyeScale;
		if (ImGui::SliderFloat("View Scale##Fisheye", &scale, 0.1f, 2.0f)) {
			gFlags.fisheyeScale = scale;
		}
	}

	if (gFlags.flags[PostEffectType_WaterDistortion]) {
		ImGui::Separator();
		ImGui::Text("Water Distortion Settings");

		float strength = gFlags.distortionStrength;
		if (ImGui::SliderFloat("Strength##Distortion", &strength, 0.0f, 0.1f, "%.4f")) {
			gFlags.distortionStrength = strength;
		}

		float speed = gFlags.distortionSpeed;
		if (ImGui::SliderFloat("Speed##Distortion", &speed, 0.0f, 10.0f)) {
			gFlags.distortionSpeed = speed;
		}

		float freq = gFlags.distortionFrequency;
		if (ImGui::SliderFloat("Frequency##Distortion", &freq, 0.1f, 50.0f)) {
			gFlags.distortionFrequency = freq;
		}
	}

	if (gFlags.flags[PostEffectType_WaterDepthFogVignette]) {
		ImGui::Separator();
		ImGui::Text("Water Depth Fog & Vignette Settings");

		float color[3] = { gFlags.fogColor.x, gFlags.fogColor.y, gFlags.fogColor.z };
		if (ImGui::ColorEdit3("Fog Color##DepthFog", color)) {
			gFlags.fogColor = Vector3(color[0], color[1], color[2]);
		}

		float density = gFlags.fogDensity;
		if (ImGui::SliderFloat("Fog Density##DepthFog", &density, 0.0f, 0.1f, "%.4f")) {
			gFlags.fogDensity = density;
		}

		float surfaceY = gFlags.fogWaterSurfaceY;
		if (ImGui::SliderFloat("Water Surface Y##DepthFog", &surfaceY, -100.0f, 100.0f)) {
			gFlags.fogWaterSurfaceY = surfaceY;
		}

		float vignette = gFlags.vignetteStrength;
		if (ImGui::SliderFloat("Vignette Strength##DepthFog", &vignette, 0.0f, 2.0f)) {
			gFlags.vignetteStrength = vignette;
		}
	}

	if (gFlags.flags[PostEffectType_WaterColorGrading]) {
		ImGui::Separator();
		ImGui::Text("Water Color Grading & Absorption Settings");

		float coeffs[3] = { gFlags.absorptionCoefficients.x, gFlags.absorptionCoefficients.y, gFlags.absorptionCoefficients.z };
		if (ImGui::SliderFloat3("Absorption Coeffs##Absorption", coeffs, 0.0f, 1.0f, "%.3f")) {
			gFlags.absorptionCoefficients = Vector3(coeffs[0], coeffs[1], coeffs[2]);
		}

		float contrast = gFlags.contrast;
		if (ImGui::SliderFloat("Contrast##ColorGrading", &contrast, 0.5f, 2.0f)) {
			gFlags.contrast = contrast;
		}

		float sat = gFlags.saturation;
		if (ImGui::SliderFloat("Saturation##ColorGrading", &sat, 0.0f, 2.0f)) {
			gFlags.saturation = sat;
		}

		float filter[3] = { gFlags.colorFilter.x, gFlags.colorFilter.y, gFlags.colorFilter.z };
		if (ImGui::ColorEdit3("Color Filter##ColorGrading", filter)) {
			gFlags.colorFilter = Vector3(filter[0], filter[1], filter[2]);
		}
	}

	if (gFlags.flags[PostEffectType_WaterCausticsLightShafts]) {
		ImGui::Separator();
		ImGui::Text("Water Caustics & Light Shafts Settings");

		float scale = gFlags.causticsScale;
		if (ImGui::SliderFloat("Caustics Scale##Caustics", &scale, 0.01f, 5.0f)) {
			gFlags.causticsScale = scale;
		}

		float speed = gFlags.causticsSpeed;
		if (ImGui::SliderFloat("Caustics Speed##Caustics", &speed, 0.0f, 5.0f)) {
			gFlags.causticsSpeed = speed;
		}

		float intensity = gFlags.causticsIntensity;
		if (ImGui::SliderFloat("Caustics Intensity##Caustics", &intensity, 0.0f, 2.0f)) {
			gFlags.causticsIntensity = intensity;
		}

		float shafts = gFlags.lightShaftsIntensity;
		if (ImGui::SliderFloat("Light Shafts Intensity##Caustics", &shafts, 0.0f, 2.0f)) {
			gFlags.lightShaftsIntensity = shafts;
		}

		float dir[3] = { gFlags.lightDirection.x, gFlags.lightDirection.y, gFlags.lightDirection.z };
		if (ImGui::SliderFloat3("Light Direction##Caustics", dir, -1.0f, 1.0f)) {
			gFlags.lightDirection = Vector3(dir[0], dir[1], dir[2]);
		}
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
}

void ONEngine::to_json(nlohmann::json& j, const ScreenPostEffectTag& c) {
	j["type"] = "ScreenPostEffectTag";
	j["enable"] = c.enable;
	j["id"] = c.id;
	// Serialize post effect flags
	j["postEffects"] = nlohmann::json::array();
	for (size_t i = 0; i < gFlags.flags.size(); ++i) {
		j["postEffects"].push_back({
			{ "type", i },
			{ "enabled", gFlags.flags[i] }
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
}
