#include "CircleCollider.h"

/// std
#include <bit>
#include <vector>

/// external
#include <magic_enum/magic_enum.hpp>
#include <imgui.h>

/// engine
#include "Engine/Core/Utility/Utility.h"
#include "Engine/Editor/Commands/ImGuiCommand/ImGuiCommand.h"

using namespace ONEngine;

void ComponentDebug::CircleColliderDebug(CircleCollider* c) {
	if(!c) {
		return;
	}

	/// 共通パラメータ
	ImGui::SeparatorText("common parameter");
	{	/// CollisionState
		int currentIndex = static_cast<int>(c->collisionState_);

		auto names = magic_enum::enum_names<CollisionState>();
		std::vector<const char*> items;
		for(auto& n : names) items.push_back(n.data());

		if(ImGui::Combo("CollisionState", &currentIndex, items.data(), (int)items.size())) {
			c->collisionState_ = static_cast<CollisionState>(currentIndex);
		}

		ImGui::Checkbox("Is Trigger", &c->isTrigger_);
		ImGui::Checkbox("Use Owner Scale", &c->useOwnerScale_);
		ImGui::Checkbox("Freeze Y", &c->freezeY_);
		ImGui::DragFloat("Mass", &c->mass_, 0.1f, 0.001f, 10000.0f);
	}

	{	/// CollisionFilter (ImGui側の処理)
		constexpr auto entries = magic_enum::enum_entries<CollisionFilter>();
		std::vector<const char*> items;
		for(const auto& entry : entries) {
			items.push_back(entry.second.data());
		}

		ImGui::Text("Collision Settings");
		ImGui::Separator();

		// ---------------------------------------------------------
		// 1. Category の設定 (Combo)
		// ---------------------------------------------------------
		uint32_t currentCategoryBit = c->GetCategoryBits();
		int categoryIdx = 0;

		for(size_t i = 0; i < entries.size(); ++i) {
			if(currentCategoryBit == static_cast<uint32_t>(entries[i].first)) {
				categoryIdx = static_cast<int>(i);
				break;
			}
		}

		if(ImGui::Combo("Category", &categoryIdx, items.data(), static_cast<int>(items.size()))) {
			c->SetCategoryBits(static_cast<uint32_t>(entries[categoryIdx].first));
		}

		// ---------------------------------------------------------
		// 2. Mask の設定 (CheckboxFlags)
		// ---------------------------------------------------------
		ImGui::Spacing();
		ImGui::Text("Collides With (Mask):");

		uint32_t currentMask = c->GetMaskBits();

		if(ImGui::Button("Select All")) {
			currentMask = 0xFFFFFFFF; // ALL定数
			c->SetMaskBits(currentMask);
		}
		ImGui::SameLine();
		if(ImGui::Button("Clear All")) {
			currentMask = 0;
			c->SetMaskBits(currentMask);
		}

		// 各レイヤーをチェックボックスとして表示 (複数選択可能)
		for(const auto& entry : entries) {
			uint32_t bitValue = static_cast<uint32_t>(entry.first);
			if(ImGui::CheckboxFlags(entry.second.data(), &currentMask, bitValue)) {
				c->SetMaskBits(currentMask);
			}
		}
	}

	/// circle parameter
	ImGui::SeparatorText("circle parameter");
	Editor::ImMathf::DragFloat("radius", &c->radius_, 0.1f);
}

void ONEngine::from_json(const nlohmann::json& j, CircleCollider& s) {
	s.enable = j.value("enable", 1);
	s.radius_ = j.value("radius", 1.0f);
	s.isTrigger_ = j.value("isTrigger", false);
	s.useOwnerScale_ = j.value("useOwnerScale", true);
	s.freezeY_ = j.value("freezeY", false);
	s.mass_ = j.value("mass", 1.0f);

	if (j.contains("state")) {
		if (j["state"].is_string()) {
			s.collisionState_ = magic_enum::enum_cast<CollisionState>(j["state"].get<std::string>()).value_or(CollisionState::Dynamic);
		} else if (j["state"].is_number()) {
			s.collisionState_ = static_cast<CollisionState>(j["state"].get<int>());
		} else {
			s.collisionState_ = CollisionState::Dynamic;
		}
	} else {
		s.collisionState_ = CollisionState::Dynamic;
	}

	s.categoryBits_ = j.value("categoryBits", static_cast<uint32_t>(CollisionFilter::Default));
	s.maskBits_ = j.value("maskBits", static_cast<uint32_t>(CollisionFilter::ALL));
}

void ONEngine::to_json(nlohmann::json& j, const CircleCollider& s) {
	auto stateName = magic_enum::enum_name(s.collisionState_);
	std::string stateStr = stateName.empty() ? "Dynamic" : std::string(stateName);
	j = nlohmann::json{
		{ "type", "CircleCollider" },
		{ "enable", s.enable },
		{ "radius", s.GetRadius() },
		{ "isTrigger", s.IsTrigger() },
		{ "useOwnerScale", s.IsUseOwnerScale() },
		{ "freezeY", s.freezeY_ },
		{ "mass", s.mass_ },
		{ "state", stateStr },
		{ "categoryBits", s.categoryBits_ },
		{ "maskBits", s.maskBits_ }
	};
}

CircleCollider::CircleCollider() {
	radius_ = 1.0f;
}

void CircleCollider::SetRadius(float radius) {
	radius_ = radius;
}

float CircleCollider::GetRadius() const {
	return radius_;
}

float ONEngine::InternalGetRadiusCircle(uint64_t nativeHandle) {
	CircleCollider* c = reinterpret_cast<CircleCollider*>(nativeHandle);
	return c ? c->GetRadius() : 0.0f;
}

void ONEngine::InternalSetRadiusCircle(uint64_t nativeHandle, float radius) {
	CircleCollider* c = reinterpret_cast<CircleCollider*>(nativeHandle);
	if(c) c->SetRadius(radius);
}

bool ONEngine::InternalIsTriggerCircle(uint64_t nativeHandle) {
	CircleCollider* c = reinterpret_cast<CircleCollider*>(nativeHandle);
	return c ? c->IsTrigger() : false;
}

void ONEngine::InternalSetTriggerCircle(uint64_t nativeHandle, bool trigger) {
	CircleCollider* c = reinterpret_cast<CircleCollider*>(nativeHandle);
	if(c) c->SetTrigger(trigger);
}

float ONEngine::InternalGetMassCircle(uint64_t nativeHandle) {
	CircleCollider* c = reinterpret_cast<CircleCollider*>(nativeHandle);
	return c ? c->GetMass() : 1.0f;
}

void ONEngine::InternalSetMassCircle(uint64_t nativeHandle, float mass) {
	CircleCollider* c = reinterpret_cast<CircleCollider*>(nativeHandle);
	if(c) c->SetMass(mass);
}

bool ONEngine::InternalIsUseOwnerScaleCircle(uint64_t nativeHandle) {
	CircleCollider* c = reinterpret_cast<CircleCollider*>(nativeHandle);
	return c ? c->IsUseOwnerScale() : true;
}

void ONEngine::InternalSetUseOwnerScaleCircle(uint64_t nativeHandle, bool use) {
	CircleCollider* c = reinterpret_cast<CircleCollider*>(nativeHandle);
	if(c) c->SetUseOwnerScale(use);
}
