#include "BoxCollider.h"

#include <magic_enum/magic_enum.hpp>

/// externals
#include <imgui.h>

/// engine
#include "Engine/Editor/EditorUtils.h"


using namespace ONEngine;

void ComponentDebug::BoxColliderDebug(BoxCollider* bc) {
	if(!bc) {
		return;
	}

	/// 共通パラメータ
	ImGui::SeparatorText("common parameter");

	{	/// CollisionState
		int currentIndex = static_cast<int>(bc->collisionState_);
		auto names = magic_enum::enum_names<CollisionState>();
		std::vector<const char*> items;
		for(auto& n : names) items.push_back(n.data());

		if(ImGui::Combo("CollisionState", &currentIndex, items.data(), static_cast<int>(items.size()))) {
			bc->collisionState_ = static_cast<CollisionState>(currentIndex);
		}

		ImGui::Checkbox("Is Trigger", &bc->isTrigger_);
		ImGui::Checkbox("Use Owner Scale", &bc->useOwnerScale_);
		ImGui::Checkbox("Freeze Y", &bc->freezeY_);
		ImGui::DragFloat("Mass", &bc->mass_, 0.1f, 0.001f, 10000.0f);
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
		uint32_t currentCategoryBit = bc->GetCategoryBits();
		int categoryIdx = 0;

		for(size_t i = 0; i < entries.size(); ++i) {
			if(currentCategoryBit == static_cast<uint32_t>(entries[i].first)) {
				categoryIdx = static_cast<int>(i);
				break;
			}
		}

		if(ImGui::Combo("Category", &categoryIdx, items.data(), static_cast<int>(items.size()))) {
			bc->SetCategoryBits(static_cast<uint32_t>(entries[categoryIdx].first));
		}

		// ---------------------------------------------------------
		// 2. Mask の設定 (CheckboxFlags)
		// ---------------------------------------------------------
		ImGui::Spacing();
		ImGui::Text("Collides With (Mask):");

		uint32_t currentMask = bc->GetMaskBits();

		if(ImGui::Button("Select All")) {
			currentMask = 0xFFFFFFFF; // ALL定数
			bc->SetMaskBits(currentMask);
		}
		ImGui::SameLine();
		if(ImGui::Button("Clear All")) {
			currentMask = 0;
			bc->SetMaskBits(currentMask);
		}

		// 各レイヤーをチェックボックスとして表示 (複数選択可能)
		for(const auto& entry : entries) {
			uint32_t bitValue = static_cast<uint32_t>(entry.first);
			if(ImGui::CheckboxFlags(entry.second.data(), &currentMask, bitValue)) {
				bc->SetMaskBits(currentMask);
			}
		}
	}


	/// box parameter
	ImGui::SeparatorText("box parameter");
	static bool unified = false;
	Editor::DrawVec3Control("size", bc->size_, 0.1f, 0.0f, 1024.0f, 100.0f, &unified);
}

void ONEngine::from_json(const nlohmann::json& j, BoxCollider& b) {
	b.enable = j.value("enable", 1);
	b.size_ = j.value("size", Vector3::One);
	b.isTrigger_ = j.value("isTrigger", false);
	b.useOwnerScale_ = j.value("useOwnerScale", true);
	b.freezeY_ = j.value("freezeY", false);
	b.mass_ = j.value("mass", 1.0f);

	if (j.contains("state")) {
		if (j["state"].is_string()) {
			b.collisionState_ = magic_enum::enum_cast<CollisionState>(j["state"].get<std::string>()).value_or(CollisionState::Dynamic);
		} else if (j["state"].is_number()) {
			b.collisionState_ = static_cast<CollisionState>(j["state"].get<int>());
		} else {
			b.collisionState_ = CollisionState::Dynamic;
		}
	} else {
		b.collisionState_ = CollisionState::Dynamic;
	}

	b.categoryBits_ = j.value("categoryBits", static_cast<uint32_t>(CollisionFilter::Default));
	b.maskBits_ = j.value("maskBits", static_cast<uint32_t>(CollisionFilter::ALL));
}

void ONEngine::to_json(nlohmann::json& j, const BoxCollider& b) {
	auto stateName = magic_enum::enum_name(b.collisionState_);
	std::string stateStr = stateName.empty() ? "Dynamic" : std::string(stateName);
	j = nlohmann::json{
		{ "type", "BoxCollider" },
		{ "enable", b.enable },
		{ "size", b.size_ },
		{ "isTrigger", b.IsTrigger() },
		{ "useOwnerScale", b.IsUseOwnerScale() },
		{ "freezeY", b.freezeY_ },
		{ "mass", b.mass_ },
		{ "state", stateStr },
		{ "categoryBits", b.categoryBits_ },
		{ "maskBits", b.maskBits_ }
	};
}


BoxCollider::BoxCollider() {
	// デフォルトの値をセット
	size_ = Vector3::One; // サイズを1x1x1に初期化
}

void BoxCollider::SetSize(const Vector3& size) {
	size_ = size;
}

const Vector3& BoxCollider::GetSize() const {
	return size_;
}

Vector3 ONEngine::InternalGetSize(uint64_t nativeHandle) {
	BoxCollider* c = reinterpret_cast<BoxCollider*>(nativeHandle);
	return c ? c->GetSize() : Vector3::Zero;
}

void ONEngine::InternalSetSize(uint64_t nativeHandle, Vector3 size) {
	BoxCollider* c = reinterpret_cast<BoxCollider*>(nativeHandle);
	if(c) c->SetSize(size);
}

bool ONEngine::InternalIsTriggerBox(uint64_t nativeHandle) {
	BoxCollider* c = reinterpret_cast<BoxCollider*>(nativeHandle);
	return c ? c->IsTrigger() : false;
}

void ONEngine::InternalSetTriggerBox(uint64_t nativeHandle, bool trigger) {
	BoxCollider* c = reinterpret_cast<BoxCollider*>(nativeHandle);
	if(c) c->SetTrigger(trigger);
}

float ONEngine::InternalGetMassBox(uint64_t nativeHandle) {
	BoxCollider* c = reinterpret_cast<BoxCollider*>(nativeHandle);
	return c ? c->GetMass() : 1.0f;
}

void ONEngine::InternalSetMassBox(uint64_t nativeHandle, float mass) {
	BoxCollider* c = reinterpret_cast<BoxCollider*>(nativeHandle);
	if(c) c->SetMass(mass);
}

bool ONEngine::InternalIsUseOwnerScaleBox(uint64_t nativeHandle) {
	BoxCollider* c = reinterpret_cast<BoxCollider*>(nativeHandle);
	return c ? c->IsUseOwnerScale() : true;
}

void ONEngine::InternalSetUseOwnerScaleBox(uint64_t nativeHandle, bool use) {
	BoxCollider* c = reinterpret_cast<BoxCollider*>(nativeHandle);
	if(c) c->SetUseOwnerScale(use);
}


