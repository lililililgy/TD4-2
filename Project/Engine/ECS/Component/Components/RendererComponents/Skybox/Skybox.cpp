#include "Skybox.h"

/// externals
#include <imgui.h>

/// editor
#include "Engine/Editor/Math/ImGuiMath.h"

using namespace ONEngine;

Skybox::Skybox() {
	SetDDSTexturePath("./Packages/Textures/kloofendal_48d_partly_cloudy_puresky_2k.dds");
}
Skybox::~Skybox() {}


void Skybox::SetDDSTexturePath(const std::string& texturePath) {
	texturePath_ = texturePath;
}

const std::string& Skybox::GetDDSTexturePath() const {
	return texturePath_;
}



void ComponentDebug::SkyboxDebug(const Skybox* skybox) {
	if (!skybox) {
		return;
	}

	std::string texturePath = skybox->GetDDSTexturePath();
	Editor::ImMathf::InputText("Skybox Texture Path", &texturePath, ImGuiInputTextFlags_ReadOnly);

}


void ONEngine::from_json(const nlohmann::json& j, Skybox& s) {
	if (j.contains("enable")) {
		s.enable = j.at("enable").get<int>();
	}

	if (j.contains("texturePath")) {
		s.SetDDSTexturePath(j.at("texturePath").get<std::string>());
	} else {
		s.SetDDSTexturePath("./Packages/Textures/kloofendal_48d_partly_cloudy_puresky_2k.dds");
	}
}

void ONEngine::to_json(nlohmann::json& j, const Skybox& s) {
	j = nlohmann::json{
		{ "type", "Skybox" },
		{ "enable", s.enable },
		{ "texturePath", s.GetDDSTexturePath() },
	};
}

