#include "Light.h"

/// external
#include <imgui.h>

/// editor
#include "Engine/Editor/Math/ImGuiMath.h"

using namespace ONEngine;

DirectionalLight::DirectionalLight() {
	SetDirection({ 0.0f, -1.0f, 0.0f });
	SetIntensity(1.0f);
	SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
}

PointLight::PointLight() {
	SetIntensity(1.0f);
	SetRadius(10.0f);
	SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
}

SpotLight::SpotLight() {
	SetIntensity(1.0f);
	SetDirection({ 0.0f, -1.0f, 0.0f });
	SetRadius(10.0f);
	SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	SetInnerAngle(0.9f);
	SetOuterAngle(1.0f);
}

void ONEngine::DirectionalLightDebug(DirectionalLight* light) {
	if(!light) return;
	if(ImGui::CollapsingHeader("DirectionalLight", ImGuiTreeNodeFlags_DefaultOpen)) {
		ONEngine::Vector4 color = light->GetColor();
		if (Editor::ImGuiColorEdit("color", &color)) {
			light->SetColor(color);
		}

		float intensity = light->GetIntensity();
		if (ImGui::DragFloat("intensity", &intensity, 0.1f, 0.0f, 1000.0f)) {
			light->SetIntensity(intensity);
		}

		ONEngine::Vector3 direction = light->GetDirection();
		if (Editor::ImMathf::DragFloat3("direction", &direction, 0.01f, -1.0f, 1.0f)) {
			light->SetDirection(direction);
		}
	}
}

void ONEngine::PointLightDebug(PointLight* light) {
	if(!light) return;
	if(ImGui::CollapsingHeader("PointLight", ImGuiTreeNodeFlags_DefaultOpen)) {
		ONEngine::Vector4 color = light->GetColor();
		if (Editor::ImGuiColorEdit("color", &color)) {
			light->SetColor(color);
		}

		float intensity = light->GetIntensity();
		if (ImGui::DragFloat("intensity", &intensity, 0.1f, 0.0f, 1000.0f)) {
			light->SetIntensity(intensity);
		}

		float radius = light->GetRadius();
		if (ImGui::DragFloat("radius", &radius, 0.1f, 0.0f, 1000.0f)) {
			light->SetRadius(radius);
		}
	}
}

void ONEngine::SpotLightDebug(SpotLight* light) {
	if(!light) return;
	if(ImGui::CollapsingHeader("SpotLight", ImGuiTreeNodeFlags_DefaultOpen)) {
		ONEngine::Vector4 color = light->GetColor();
		if (Editor::ImGuiColorEdit("color", &color)) {
			light->SetColor(color);
		}

		float intensity = light->GetIntensity();
		if (ImGui::DragFloat("intensity", &intensity, 0.1f, 0.0f, 1000.0f)) {
			light->SetIntensity(intensity);
		}

		ONEngine::Vector3 direction = light->GetDirection();
		if (Editor::ImMathf::DragFloat3("direction", &direction, 0.01f, -1.0f, 1.0f)) {
			light->SetDirection(direction);
		}

		float radius = light->GetRadius();
		if (ImGui::DragFloat("radius", &radius, 0.1f, 0.0f, 1000.0f)) {
			light->SetRadius(radius);
		}

		float innerAngle = light->GetInnerAngle();
		if (ImGui::DragFloat("innerAngle", &innerAngle, 0.01f, 0.0f, 3.14159f)) {
			light->SetInnerAngle(innerAngle);
		}

		float outerAngle = light->GetOuterAngle();
		if (ImGui::DragFloat("outerAngle", &outerAngle, 0.01f, 0.0f, 3.14159f)) {
			light->SetOuterAngle(outerAngle);
		}
	}
}
