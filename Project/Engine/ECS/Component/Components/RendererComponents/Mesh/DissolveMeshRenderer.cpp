#include "DissolveMeshRenderer.h"

/// externals
#include <magic_enum/magic_enum.hpp>

/// engine
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/Editor/EditorUtils.h"
#include "Engine/Editor/Math/AssetPayload.h"
#include "Engine/Editor/Commands/LambdaCommand.h"
#include "Engine/Editor/Manager/EditCommand.h"

using namespace ONEngine;

void ONEngine::ShowGUI(DissolveMeshRenderer* dmr, Asset::AssetCollection* ac) {
	if(!dmr) {
		return;
	}

	/// param get
	Vector4& color = dmr->material_.baseColor;

	/// edit
	if (Editor::ImGuiColorEdit("color", &color)) {
		// color updated
	}

	ImGui::Spacing();

	/// meshの変更
	std::string meshName = ac->GetAssetPath<Asset::Model>(dmr->meshGuid_);
	ImGui::Text("mesh path");
	ImGui::InputText("##mesh", meshName.data(), meshName.capacity(), ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {
			if (payload->Data) {
				Editor::AssetPayload* assetPayload = *static_cast<Editor::AssetPayload**>(payload->Data);
				if (ac->GetAssetTypeFromGuid(assetPayload->guid) == Asset::AssetType::Mesh) {
					Guid oldGuid = dmr->meshGuid_;
					Guid newGuid = assetPayload->guid;
					Editor::EditCommand::Execute<Editor::LambdaCommand>(
						[dmr, newGuid]() { dmr->meshGuid_ = newGuid; },
						[dmr, oldGuid]() { dmr->meshGuid_ = oldGuid; }
					);
					Console::Log(std::format("Mesh path set to: {}", assetPayload->filePath));
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	/// textureの変更 (Dissolve)
	ImGui::Text("dissolve texture path");

	/// ----------------------------------------------
	/// テクスチャのプレビュー表示
	/// ----------------------------------------------
	bool hasTextureGuid = dmr->dissolveTexture_ != Guid::kInvalid;
	if (hasTextureGuid) {
		const Asset::Texture* tex = ac->GetTextureFromGuid(dmr->dissolveTexture_);
		if (tex) {
			Vector2 aspectRatio = tex->GetTextureSize();
			aspectRatio /= (std::max)(aspectRatio.x, aspectRatio.y);

			ImTextureID texId = reinterpret_cast<ImTextureID>(tex->GetSRVGPUHandle().ptr);
			ImGui::Image(texId, ImVec2(64.0f * aspectRatio.x, 64.0f * aspectRatio.y));
		}
	} else {
		/// テクスチャがない場合はドラッグドロップ領域を表示する
		ImVec2 size = ImVec2(64, 64);
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		ImGui::InvisibleButton("DropAreaDissolve", size);

		ImU32 imColor = IM_COL32(100, 100, 255, 100); // 半透明の青
		drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), imColor, 4.0f);
		drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(255, 255, 255, 200), 4.0f, 0, 2.0f);
	}

	/// ----------------------------------------------
	/// ドラッグアンドドロップでテクスチャを設定
	/// ----------------------------------------------
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {
			if (payload->Data) {
				Editor::AssetPayload* assetPayload = *static_cast<Editor::AssetPayload**>(payload->Data);
				if (ac->GetAssetTypeFromGuid(assetPayload->guid) == Asset::AssetType::Texture) {
					Guid oldGuid = dmr->dissolveTexture_;
					Guid newGuid = assetPayload->guid;
					Editor::EditCommand::Execute<Editor::LambdaCommand>(
						[dmr, newGuid]() { dmr->dissolveTexture_ = newGuid; },
						[dmr, oldGuid]() { dmr->dissolveTexture_ = oldGuid; }
					);
					Console::Log(std::format("Dissolve Texture path set to: {}", assetPayload->filePath));
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Spacing();
	ImGui::SeparatorText("Dissolve Settings");

	/// compare
	Editor::Combo<DissolveCompare>("Dissolve Compare", dmr->dissolveCompare_);
	Editor::SliderFloat("Dissolve Threshold", dmr->dissolveThreshold_, 0.0f, 1.0f);
	Editor::SliderFloat("Edge Width", dmr->edgeWidth_, 0.0f, 0.5f);
	Editor::ImGuiColorEdit("Edge Color", &dmr->edgeColor_);

	ImGui::Spacing();

	/// material
	Editor::ImMathf::MaterialEdit("Material##DissolveMeshRenderer", &dmr->material_, ac);

}

void ONEngine::from_json(const nlohmann::json& j, DissolveMeshRenderer& dmr) {
	dmr.meshGuid_ = j.value("meshGuid", Guid::kInvalid);
	dmr.material_ = j.value("material", Asset::Material());
	dmr.dissolveTexture_ = j.value("dissolveTexture", Guid::kInvalid);
	dmr.dissolveCompare_ = j.value("dissolveCompare", DissolveCompare::LessEqual);
	dmr.dissolveThreshold_ = j.value("dissolveThreshold", 1.0f);
	dmr.edgeWidth_ = j.value("edgeWidth", 0.05f);
	dmr.edgeColor_ = j.value("edgeColor", Vector4(1.0f, 0.5f, 0.0f, 1.0f));
}

void ONEngine::to_json(nlohmann::json& j, const DissolveMeshRenderer& dmr) {
	j = {
		{ "type", "DissolveMeshRenderer" },
		{ "meshGuid", dmr.meshGuid_ },
		{ "material", dmr.material_ },
		{ "dissolveTexture", dmr.dissolveTexture_ },
		{ "dissolveCompare", dmr.dissolveCompare_ },
		{ "dissolveThreshold", dmr.dissolveThreshold_ },
		{ "edgeWidth", dmr.edgeWidth_ },
		{ "edgeColor", dmr.edgeColor_ }
	};
}

/// ///////////////////////////////////////////////////
/// ここから DissolveMeshRenderer の定義
/// ///////////////////////////////////////////////////

DissolveMeshRenderer::DissolveMeshRenderer() {
	dissolveThreshold_ = 1.0f;
}
DissolveMeshRenderer::~DissolveMeshRenderer() {}


const Guid& DissolveMeshRenderer::GetMeshGuid() const {
	return meshGuid_;
}

const Guid& DissolveMeshRenderer::GetDissolveTextureGuid() const {
	return dissolveTexture_;
}

uint32_t DissolveMeshRenderer::GetDissolveTextureId(Asset::AssetCollection* ac) const {
	const Asset::Texture* dissolveTex = ac->GetTextureFromGuid(dissolveTexture_);
	if(dissolveTex) {
		return dissolveTex->GetSRVDescriptorIndex();
	}
	return 0;
}

float DissolveMeshRenderer::GetDissolveThreshold() const {
	return dissolveThreshold_;
}

GPUMaterial DissolveMeshRenderer::GetGPUMaterial(Asset::AssetCollection* ac) const {
	GPUMaterial result{};
	result.uvTransform = material_.uvTransform;
	result.baseColor = material_.baseColor;
	result.postEffectFlags = material_.postEffectFlags;
	result.entityId = GetOwner()->GetId();

	if(material_.HasBaseTexture()) {
		const Asset::Texture* baseTex = ac->GetTextureFromGuid(material_.GetBaseTextureGuid());
		if(baseTex) {
			result.baseTextureId = baseTex->GetSRVDescriptorIndex();
		}
	}

	return result;
}

uint32_t ONEngine::DissolveMeshRenderer::GetDissolveCompare() const {
	return static_cast<uint32_t>(dissolveCompare_);
}



