#include "MeshRenderer.h"
#include <nlohmann/json.hpp>
#include "Engine/Script/MonoScriptEngine.h"

/// std
#include <format>

/// engine
#include "Engine/Asset/AssetType.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/Editor/Commands/ComponentEditCommands/ComponentJsonConverter.h"

/// editor
#include "Engine/Editor/Math/ImGuiMath.h"
#include "Engine/Editor/Math/AssetDebugger.h"
#include "Engine/Editor/Math/AssetPayload.h"
#include "Engine/Editor/Commands/LambdaCommand.h"
#include "Engine/Editor/Manager/EditCommand.h"

using namespace ONEngine;

MeshRenderer::MeshRenderer() {
	SetMeshPath("./Packages/Models/primitive/cube.obj");
	material_.baseColor = Vector4::White;
	material_.postEffectFlags = PostEffectFlags_Lighting;
}

MeshRenderer::~MeshRenderer() = default;

void MeshRenderer::SetupRenderData(Asset::AssetCollection* assetCollection) {
	gpuMaterial_.postEffectFlags = material_.postEffectFlags;
	gpuMaterial_.baseColor = material_.baseColor;
	gpuMaterial_.uvTransform = material_.uvTransform;
	gpuMaterial_.entityId = GetOwner() ? GetOwner()->GetId() : 0;

	if (material_.HasBaseTexture()) {
		gpuMaterial_.baseTextureId = assetCollection->GetTextureIndexFromGuid(material_.GetBaseTextureGuid());
	} else {
		gpuMaterial_.baseTextureId = 0;
	}
}

void MeshRenderer::SetMeshPath(const std::string& path) {
	meshPath_ = path;
}

void MeshRenderer::SetColor(const Vector4& color) {
	material_.baseColor = color;
}

void MeshRenderer::SetPostEffectFlags(uint32_t flags) {
	material_.postEffectFlags = flags;
}

void MeshRenderer::SetUVTransform(const UVTransform& uvTransform) {
	material_.uvTransform = uvTransform;
}

void MeshRenderer::SetRenderQueue(RenderQueue queue) {
	renderQueue_ = queue;
}

RenderQueue MeshRenderer::GetRenderQueue() const {
	return renderQueue_;
}

const std::string& MeshRenderer::GetMeshPath() const {
	return meshPath_;
}

const Vector4& MeshRenderer::GetColor() const {
	return material_.baseColor;
}

const GPUMaterial& MeshRenderer::GetGpuMaterial() const {
	return gpuMaterial_;
}

uint32_t MeshRenderer::GetPostEffectFlags() const {
	return material_.postEffectFlags;
}

const UVTransform& MeshRenderer::GetUVTransform() const {
	return material_.uvTransform;
}

const Guid& MeshRenderer::GetTextureGuid() const {
	return material_.guid;
}


MonoString* ONEngine::InternalGetMeshName(uint64_t nativeHandle) {
	/// ptrから MeshRenderer を取得
	MeshRenderer* renderer = reinterpret_cast<MeshRenderer*>(nativeHandle);
	if (!renderer) {
		Console::Log("MeshRenderer pointer is null");
		return nullptr;
	}

	return mono_string_new(MonoScriptEngine::GetInstance().Domain(), renderer->GetMeshPath().c_str());
}

void ONEngine::InternalSetMeshName(uint64_t nativeHandle, MonoString* meshName) {
	/// ptrから MeshRenderer を取得
	MeshRenderer* renderer = reinterpret_cast<MeshRenderer*>(nativeHandle);
	if (!renderer) {
		Console::Log("MeshRenderer pointer is null");
		return;
	}

	/// stringに変換&設定
	char* meshNameCStr = mono_string_to_utf8(meshName);
	if(meshNameCStr) {
		std::string meshNameStr(meshNameCStr);
		renderer->SetMeshPath(meshNameStr);
		mono_free(meshNameCStr);
	}
}

Vector4 ONEngine::InternalGetMeshColor(uint64_t nativeHandle) {
	/// ptrから MeshRenderer を取得
	MeshRenderer* renderer = reinterpret_cast<MeshRenderer*>(nativeHandle);
	if (!renderer) {
		Console::Log("MeshRenderer pointer is null");
		return Vector4::Zero;
	}

	return renderer->GetColor();
}

void ONEngine::InternalSetMeshColor(uint64_t nativeHandle, Vector4 color) {
	/// ptrから MeshRenderer を取得
	MeshRenderer* renderer = reinterpret_cast<MeshRenderer*>(nativeHandle);
	if (renderer) {
		renderer->SetColor(color);
	} else {
		Console::Log("MeshRenderer pointer is null");
	}
}

uint32_t ONEngine::InternalGetPostEffectFlags(uint64_t nativeHandle) {
	/// ptrから MeshRenderer を取得
	MeshRenderer* renderer = reinterpret_cast<MeshRenderer*>(nativeHandle);
	if (!renderer) {
		Console::LogError("MeshRenderer pointer is null");
		return PostEffectFlags_None;
	}
	return renderer->GetPostEffectFlags();
}

void ONEngine::InternalSetPostEffectFlags(uint64_t nativeHandle, uint32_t flags) {
	/// ptrから MeshRenderer を取得
	MeshRenderer* renderer = reinterpret_cast<MeshRenderer*>(nativeHandle);
	if (renderer) {
		renderer->SetPostEffectFlags(flags);
	} else {
		Console::LogError("MeshRenderer pointer is null");
	}
}

uint32_t ONEngine::InternalGetRenderQueue(uint64_t nativeHandle) {
	MeshRenderer* renderer = reinterpret_cast<MeshRenderer*>(nativeHandle);
	return renderer ? static_cast<uint32_t>(renderer->GetRenderQueue()) : 2;
}

void ONEngine::InternalSetRenderQueue(uint64_t nativeHandle, uint32_t queue) {
	MeshRenderer* renderer = reinterpret_cast<MeshRenderer*>(nativeHandle);
	if (renderer) renderer->SetRenderQueue(static_cast<RenderQueue>(queue));
}

void ComponentDebug::MeshRendererDebug(MeshRenderer* mr, Asset::AssetCollection* assetCollection) {
	if (!mr) {
		return;
	}

	/// param get
	Vector4& color = mr->material_.baseColor;
	std::string& meshPath = mr->meshPath_;

	/// edit
	if (Editor::ImGuiColorEdit("color", &color)) {
		mr->SetColor(color);
	}

	const char* queueNames[] = { "Background", "Telegraph", "Default" };
	int currentQueue = static_cast<int>(mr->renderQueue_);
	if (ImGui::Combo("Render Queue", &currentQueue, queueNames, 3)) {
		mr->renderQueue_ = static_cast<RenderQueue>(currentQueue);
	}

	ImGui::Spacing();


	/// meshの変更
	ImGui::Text("mesh path");
	ImGui::InputText("##mesh", meshPath.data(), meshPath.capacity(), ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {
			if (payload->Data) {
				Editor::AssetPayload* assetPayload = *static_cast<Editor::AssetPayload**>(payload->Data);
				std::string path = assetPayload->filePath;
				Asset::AssetType type = Asset::GetAssetTypeFromExtension(FileSystem::FileExtension(path));

				/// メッシュのパスが有効な形式か確認
				if (type == Asset::AssetType::Mesh) {
					std::string oldPath = mr->GetMeshPath();
					std::string newPath = path;
					Editor::EditCommand::Execute<Editor::LambdaCommand>(
						[mr, newPath]() { mr->SetMeshPath(newPath); },
						[mr, oldPath]() { mr->SetMeshPath(oldPath); }
					);

					Console::Log(std::format("Mesh path set to: {}", path));
				} else {
					Console::LogError("Invalid mesh format. Please use .obj or .gltf.");
				}
			}
		}
		ImGui::EndDragDropTarget();
	}


	/// textureの変更
	ImGui::Text("texture path");

	/// ----------------------------------------------
	/// テクスチャのプレビュー表示
	/// ----------------------------------------------

	bool hasTextureGuid = mr->material_.HasBaseTexture();
	if (hasTextureGuid) {
		if (Asset::Texture* tex = assetCollection->GetTexture(assetCollection->GetTexturePath(mr->material_.GetBaseTextureGuid()))) {
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

		// InvisibleButton はクリック判定やDragDropのターゲット領域になる
		ImGui::InvisibleButton("DropArea", size);

		// 視覚的な四角形を描く
		ImU32 imColor = IM_COL32(100, 100, 255, 100); // 半透明の青
		drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), imColor, 4.0f);

		// 枠線
		drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(255, 255, 255, 200), 4.0f, 0, 2.0f);
	}


	/// ----------------------------------------------
	/// ドラッグアンドドロップでテクスチャを設定
	/// ----------------------------------------------
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {

			/// ペイロードが存在する場合
			if (payload->Data) {
				Editor::AssetPayload* assetPayload = *static_cast<Editor::AssetPayload**>(payload->Data);
				const std::string path = assetPayload->filePath;

				/// テクスチャのパスが有効な形式か確認
				const Asset::AssetType type = Asset::GetAssetTypeFromExtension(FileSystem::FileExtension(path));
				if (type == Asset::AssetType::Texture) {
					Guid oldGuid = mr->material_.GetBaseTextureGuid();
					Guid newGuid = assetPayload->guid;
					Editor::EditCommand::Execute<Editor::LambdaCommand>(
						[mr, newGuid]() { mr->material_.SetBaseTextureGuid(newGuid); },
						[mr, oldGuid]() { mr->material_.SetBaseTextureGuid(oldGuid); }
					);

					Console::Log(std::format("Texture path set to: {}", path));
				} else {
					Console::LogError("Invalid texture format. Please use .png, .jpg, or .jpeg.");
				}
			}
		}

		ImGui::EndDragDropTarget();
	}


	/// ----------------------------------------------
	/// materialの設定
	/// ----------------------------------------------
	Editor::ImMathf::MaterialEdit("Material##MeshRenderer", &mr->material_, assetCollection);

}


void ONEngine::from_json(const nlohmann::json& j, MeshRenderer& m) {
	if (j.contains("enable")) {
		m.enable = j.at("enable").get<int>();
	}

	m.SetMeshPath(j.at("meshPath").get<std::string>());


	/// デバッグのためにvalueではなくcontainsでチェック
	if (j.contains("material")) {
		m.material_ = j.at("material").get<Asset::Material>();
	}

	if (j.contains("renderQueue")) {
		m.renderQueue_ = static_cast<RenderQueue>(j.at("renderQueue").get<uint32_t>());
	}

}

void ONEngine::to_json(nlohmann::json& j, const MeshRenderer& m) {
	j = nlohmann::json{
		{ "type", "MeshRenderer" },
		{ "enable", m.enable },
		{ "meshPath", m.GetMeshPath() },
		{ "material", m.material_ },
		{ "renderQueue", static_cast<uint32_t>(m.renderQueue_) },
	};
}
