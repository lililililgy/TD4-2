#include "AgentIntentComponent.h"
#include <nlohmann/json.hpp>
#include "Engine/Editor/Commands/ComponentEditCommands/ComponentJsonConverter.h"
#include <imgui.h>

namespace ONEngine {


void ComponentDebug::AgentIntentComponentDebug(AgentIntentComponent* comp) {
	if(!comp) {
		return;
	}
	ImGui::DragFloat3("Desired Direction", &comp->desiredMoveDirection.x, 0.1f);
	ImGui::DragFloat("Current Speed", &comp->currentSpeed, 0.1f);
	ImGui::DragFloat("Max Speed", &comp->maxSpeed, 0.1f);
	ImGui::DragFloat("Rotation Speed", &comp->rotationSpeed, 0.1f);
	ImGui::Checkbox("Use Desired Rotation", &comp->useDesiredRotation);
	ImGui::Checkbox("Is Attacking", &comp->isAttacking);
	ImGui::InputInt("Target ID", &comp->targetEntityId);
}

void from_json(const nlohmann::json& j, AgentIntentComponent& c) {
	c.enable = j.at("enable").get<int>();
	if (j.contains("desiredMoveDirection")) {
		c.desiredMoveDirection = j.at("desiredMoveDirection").get<Vector3>();
	}
	if (j.contains("desiredRotation")) {
		c.desiredRotation = j.at("desiredRotation").get<Quaternion>();
	}
	if (j.contains("rotationSpeed")) {
		c.rotationSpeed = j.at("rotationSpeed").get<float>();
	}
	if (j.contains("maxSpeed")) {
		c.maxSpeed = j.at("maxSpeed").get<float>();
	}
	if (j.contains("useDesiredRotation")) {
		c.useDesiredRotation = j.at("useDesiredRotation").get<bool>();
	}
	if (j.contains("isAttacking")) {
		c.isAttacking = j.at("isAttacking").get<bool>();
	}
	if (j.contains("targetEntityId")) {
		c.targetEntityId = j.at("targetEntityId").get<int32_t>();
	}
}

void to_json(nlohmann::json& j, const AgentIntentComponent& c) {
	j = nlohmann::json{
		{ "type", "AgentIntentComponent" },
		{ "enable", c.enable },
		{ "desiredMoveDirection", c.desiredMoveDirection },
		{ "desiredRotation", c.desiredRotation },
		{ "rotationSpeed", c.rotationSpeed },
		{ "maxSpeed", c.maxSpeed },
		{ "useDesiredRotation", c.useDesiredRotation },
		{ "isAttacking", c.isAttacking },
		{ "targetEntityId", c.targetEntityId }
	};
}


}