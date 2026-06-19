#include "BoxCollider2D.h"

#include <magic_enum/magic_enum.hpp>
#include <vector>

/// externals
#include <imgui.h>

/// engine
#include "Engine/Editor/EditorUtils.h"

using namespace ONEngine;

void ComponentDebug::BoxCollider2DDebug(BoxCollider2D* bc) {
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
	Editor::DrawVec2Control("size", bc->size_, 0.1f, 0.0f, 1024.0f, 100.0f, &unified);
}

void ONEngine::from_json(const nlohmann::json& j, BoxCollider2D& b) {
	b.enable = j.value("enable", 1);
	b.size_ = j.value("size", Vector2::One);
	b.isTrigger_ = j.value("isTrigger", false);
	b.useOwnerScale_ = j.value("useOwnerScale", true);
	b.freezeY_ = j.value("freezeY", false);
	b.mass_ = j.value("mass", 1.0f);
	b.collisionState_ = magic_enum::enum_cast<CollisionState>(j.value("state", "Dynamic")).value_or(CollisionState::Dynamic);
	b.categoryBits_ = j.value("categoryBits", static_cast<uint32_t>(CollisionFilter::Default));
	b.maskBits_ = j.value("maskBits", static_cast<uint32_t>(CollisionFilter::ALL));
}

void ONEngine::to_json(nlohmann::json& j, const BoxCollider2D& b) {
	j = nlohmann::json{
		{ "type", "BoxCollider2D" },
		{ "enable", b.enable },
		{ "size", b.size_ },
		{ "isTrigger", b.IsTrigger() },
		{ "useOwnerScale", b.IsUseOwnerScale() },
		{ "freezeY", b.freezeY_ },
		{ "mass", b.mass_ },
		{ "state", magic_enum::enum_name(b.collisionState_) },
		{ "categoryBits", b.categoryBits_ },
		{ "maskBits", b.maskBits_ }
	};
}

BoxCollider2D::BoxCollider2D() {
	size_ = Vector2::One; // 1x1 に初期化
}

void BoxCollider2D::SetSize(const Vector2& size) {
	size_ = size;
}

const Vector2& BoxCollider2D::GetSize() const {
	return size_;
}

Vector2 ONEngine::InternalGetSizeBox2D(uint64_t nativeHandle) {
	BoxCollider2D* c = reinterpret_cast<BoxCollider2D*>(nativeHandle);
	return c ? c->GetSize() : Vector2::Zero;
}

void ONEngine::InternalSetSizeBox2D(uint64_t nativeHandle, Vector2 size) {
	BoxCollider2D* c = reinterpret_cast<BoxCollider2D*>(nativeHandle);
	if(c) c->SetSize(size);
}

bool ONEngine::InternalIsTriggerBox2D(uint64_t nativeHandle) {
	BoxCollider2D* c = reinterpret_cast<BoxCollider2D*>(nativeHandle);
	return c ? c->IsTrigger() : false;
}

void ONEngine::InternalSetTriggerBox2D(uint64_t nativeHandle, bool trigger) {
	BoxCollider2D* c = reinterpret_cast<BoxCollider2D*>(nativeHandle);
	if(c) c->SetTrigger(trigger);
}

float ONEngine::InternalGetMassBox2D(uint64_t nativeHandle) {
	BoxCollider2D* c = reinterpret_cast<BoxCollider2D*>(nativeHandle);
	return c ? c->GetMass() : 1.0f;
}

void ONEngine::InternalSetMassBox2D(uint64_t nativeHandle, float mass) {
	BoxCollider2D* c = reinterpret_cast<BoxCollider2D*>(nativeHandle);
	if(c) c->SetMass(mass);
}

bool ONEngine::InternalIsUseOwnerScaleBox2D(uint64_t nativeHandle) {
	BoxCollider2D* c = reinterpret_cast<BoxCollider2D*>(nativeHandle);
	return c ? c->IsUseOwnerScale() : true;
}

void ONEngine::InternalSetUseOwnerScaleBox2D(uint64_t nativeHandle, bool use) {
	BoxCollider2D* c = reinterpret_cast<BoxCollider2D*>(nativeHandle);
	if(c) c->SetUseOwnerScale(use);
}
