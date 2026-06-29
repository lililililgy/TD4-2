#include "ScreenPostEffectTag.h"

/// std
#include <vector>
#include <string>

/// external
#include <imgui.h>


using namespace ONEngine;

namespace {

	struct Flags {
		Flags() : flags(PostEffectType_Count, false), fisheyeStrength(0.15f), fisheyeScale(0.9f) {}
		std::vector<bool> flags;
		std::vector<std::string> flagNames = {
			"Grayscale",
			"Radial Blur",
			"Fisheye"
		};
		float fisheyeStrength;
		float fisheyeScale;
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
		if (ImGui::SliderFloat("Distortion Strength", &strength, -1.0f, 1.0f)) {
			gFlags.fisheyeStrength = strength;
		}

		float scale = gFlags.fisheyeScale;
		if (ImGui::SliderFloat("View Scale", &scale, 0.1f, 2.0f)) {
			gFlags.fisheyeScale = scale;
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
}

void ONEngine::to_json(nlohmann::json& j, const ScreenPostEffectTag& c) {
	j["type"] = "ScreenPostEffectTag";
	j["enable"] = c.enable;
	j["id"] = c.id;
	// Serialize post effect flags
	for (size_t i = 0; i < gFlags.flags.size(); ++i) {
		j["postEffects"].push_back({
			{ "type", i },
			{ "enabled", gFlags.flags[i] }
			});
	}
	j["fisheyeStrength"] = c.GetFisheyeStrength();
	j["fisheyeScale"] = c.GetFisheyeScale();
}
