#include "VoxelTerrain.h"

/// std
#include <algorithm>

/// externals
#include <imgui.h>
#include <magic_enum/magic_enum.hpp>

/// engine
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Asset/Assets/Texture/Texture.h"
#include "Engine/Core/Utility/Utility.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Editor/Commands/ImGuiCommand/ImGuiCommand.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/Core/DirectX12/GPUTimeStamp/GPUTimeStamp.h"

/// editor
#include "Engine/Editor/EditorUtils.h"
#include "Engine/Editor/Math/AssetPayload.h"
#include "Engine/Editor/Commands/LambdaCommand.h"
#include "Engine/Editor/Manager/EditCommand.h"

namespace ONEngine {

void ComponentDebug::VoxelTerrainDebug(VoxelTerrain* vt, DxManager* dxm, Asset::AssetCollection* ac) {
	if(!vt) {
		Console::LogError("VoxelTerrainDebug: voxelTerrain is nullptr");
		return;
	}


	ImGui::SeparatorText("DebugRendering");
	Editor::ImMathf::Checkbox("Can MeshShader Rendering", &vt->canMeshShaderRendering_);
	Editor::ImMathf::Checkbox("Is Rendering Wireframe", &vt->isRenderingWireframe_);
	Editor::ImMathf::Checkbox("Is Rendering Transvoxel", &vt->isRenderingTransvoxel_);
	//Editor::ImMathf::Checkbox("Is Rendering Cubic", &voxelTerrain->isRenderingCubic_);
	//Editor::ImMathf::Checkbox("Can VertexShader Rendering", &voxelTerrain->canVertexShaderRendering_);
	static bool showChunkBounds = false;
	Editor::ImMathf::Checkbox("Show Chunk Bounds", &showChunkBounds);
	if(showChunkBounds) {
		const Vector3Int& chunkSizeInt = vt->GetChunkSize();
		const Vector2Int& chunkCount = vt->GetChunkCountXZ();
		for(int x = 0; x < chunkCount.x; ++x) {
			for(int z = 0; z < chunkCount.y; ++z) {
				// 各チャンクの位置を計算
				Vector3 chunkSize = Vector3(
					static_cast<float>(chunkSizeInt.x),
					static_cast<float>(chunkSizeInt.y),
					static_cast<float>(chunkSizeInt.z)
				);

				Vector3 chunkPosition = Vector3(static_cast<float>(x), 0.0f, static_cast<float>(z)) * chunkSize;
				chunkPosition += chunkSize * 0.5f;
				chunkPosition -= Vector3(0.5f, 0.0f, 0.5f); // 中心を合わせる調整

				// Gizmoで枠線を描画
				Gizmo::DrawWireCube(chunkPosition, chunkSize, Quaternion::kIdentity, Color::kWhite);
			}
		}
	}


	ImGui::Separator();

	/// チャンクのデバッグ表示
	Editor::ImMathf::DragInt2("Chunk Count XZ", &vt->chunkCountXZ_, 1, 1, 32);
	Editor::ImMathf::DragInt3("Chunk Size", &vt->chunkSize_, 1, 1, 1024);
	Editor::ImMathf::DragInt3("Texture Size", &vt->textureSize_, 1, 1, 256);
	Editor::ImMathf::DragFloat("ISOLevel", &vt->isoLevel_, 0.05f, 0.0f, 1.0f);

	if(ImGui::CollapsingHeader("LODInfo")) {
		bool useLOD = vt->lodInfo_.useLOD;
		if(Editor::ImMathf::Checkbox("Use LOD", &useLOD)) {
			vt->lodInfo_.useLOD = useLOD;
		}

		if(useLOD) {
			Editor::ImMathf::DragFloat("LOD Distance 0", &vt->lodInfo_.lodDistance0, 1.0f, 0.0f, 1000.0f);
			Editor::ImMathf::DragFloat("LOD Distance 1", &vt->lodInfo_.lodDistance1, 1.0f, 0.0f, 1000.0f);
			Editor::ImMathf::DragFloat("LOD Distance 2", &vt->lodInfo_.lodDistance2, 1.0f, 0.0f, 1000.0f);
			Editor::ImMathf::DragFloat("Max Draw Distance", &vt->lodInfo_.maxDrawDistance, 10.0f, 0.0f, 5000.0f);

			const int minLodLevel = 0;
			const int maxLodLevel = 5;
			Editor::SliderInt("LOD Level 0", vt->lodInfo_.lodLevel0, minLodLevel, maxLodLevel);
			Editor::SliderInt("LOD Level 1", vt->lodInfo_.lodLevel1, minLodLevel, maxLodLevel);
			Editor::SliderInt("LOD Level 2", vt->lodInfo_.lodLevel2, minLodLevel, maxLodLevel);
			Editor::SliderInt("LOD Level 3", vt->lodInfo_.lodLevel3, minLodLevel, maxLodLevel);

		} else {

			int lod = static_cast<int>(vt->lodInfo_.lod);
			if(Editor::ImMathf::DragInt("LOD", &lod, 1, 0, 3)) {
				vt->lodInfo_.lod = static_cast<uint32_t>(lod);
			}
		}
	}


	Editor::ImMathf::MaterialEdit("Material", &vt->material_, ac, true);
	Editor::ImMathf::MaterialEdit("CliffMaterial", &vt->cliffMaterial_, ac, true);


	/// ===========================================
	/// 地形描画に使用するテクスチャID
	/// ===========================================
	if(ImGui::CollapsingHeader("UsedTextureID")) {

		const int kMaxLoop = 3;
		for(int i = 0; i < kMaxLoop; ++i) {
			ImGui::PushID(i);

			Guid& guid = vt->usedTextureGuids_[i];
			bool isSelected = (vt->materialId_ == i);

			/// Guidが有効であれば
			if(guid.CheckValid()) {
				/// Textureのプレビューかつボタンを表示、
				/// 選択中のTextureは強調表示する。
				if(isSelected) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
				}

				/// テクスチャのプレビュー表示
				if(Editor::ImMathf::TextureButton("##button", ac->GetTexture(ac->GetTexturePath(guid)))) {
					vt->materialId_ = i;
				}

				if(isSelected) {
					ImGui::PopStyleColor();
				}

				/// ----- 枠で強調表示 ----- ///
				if(isSelected) {
					ImDrawList* dl = ImGui::GetWindowDrawList();
					dl->AddRect(
						ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
						IM_COL32(255, 255, 0, 255),
						4.0f, 0, 3.0f
					);
				}

			} else {
				/// Guidが無効値であればドラッグスペースを表示する
				Editor::ImMathf::DrawTextureDropSpace("BaseTex");
			}

			/// ----- ドラッグ&ドロップ ----- ///
			if(ImGui::BeginDragDropTarget()) {
				if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {
					if(payload->Data) {
						Editor::AssetPayload* assetPayload = *static_cast<Editor::AssetPayload**>(payload->Data);
						const std::string path = assetPayload->filePath;
						if(ONEngine::Asset::CheckAssetType(ONEngine::FileSystem::FileExtension(path), ONEngine::Asset::AssetType::Texture)) {
							const ONEngine::Guid& dropGuid = assetPayload->guid;
							Guid oldGuid = guid;
							Guid newGuid = dropGuid;
							Guid* pGuid = &guid;
							Editor::EditCommand::Execute<Editor::LambdaCommand>(
								[pGuid, newGuid]() { *pGuid = newGuid; },
								[pGuid, oldGuid]() { *pGuid = oldGuid; }
							);
						}
					}
				}
				ImGui::EndDragDropTarget();
			}


			//if(guid.CheckValid()) {
			//	int32_t& id = vt->usedTextureIds_.ids[i];
			//	id = ac->GetTextureFromGuid(guid)->GetSRVDescriptorIndex();
			//}

			ImGui::PopID();

			if(i != kMaxLoop - 1) {
				ImGui::SameLine();
			}

		}
	}


	/// ===========================================
	/// エディタ用 項目
	/// ===========================================
	/*
		エディタ モードの切り替え
		・左Ctrl + E : 編集モードの切り替え

		編集モードの切り替え
		・1 : 隣接編集モード
		・2 : 範囲編集モード
		・ESC : 編集モードの終了

		ブラシサイズの変更
		・左Shift + マウスホイール : ブラシサイズの変更

		ブラシの強さの変更
		・左Alt + マウスホイール : ブラシの強さの変更
	*/


	Editor::ImMathf::Checkbox("IsEditMode", &vt->isEditEnabled_);
	if(Input::PressKey(DIK_LCONTROL) && Input::TriggerKey(DIK_E)) {
		vt->isEditEnabled_ = !vt->isEditEnabled_;
	}

	if(vt->isEditEnabled_) {
		static bool isEdit = false;

		/// 編集モードの切り替え
		if(Input::TriggerKey(DIK_ESCAPE)) {
			vt->editMode_ = VoxelTerrain::EditMode::UNKOWN;
			isEdit = true;
		}

		for(int i = 0; i < VoxelTerrain::EditMode::COUNT; ++i) {
			if(Input::TriggerKey(DIK_1 + i)) {
				vt->editMode_ = i + 1;
				isEdit = true;
			}
		}


		/// ---------------------------------------------------
		/// 編集モードのコンボボックス
		/// ---------------------------------------------------
		std::string_view currentModeName = magic_enum::enum_name(VoxelTerrain::EditMode(vt->editMode_));
		if(ImGui::BeginCombo("Edit Mode", currentModeName.data())) {
			// 列挙型のすべての値と名前を取得
			constexpr auto& enumValues = magic_enum::enum_values<VoxelTerrain::EditMode>();
			constexpr auto& enumNames = magic_enum::enum_names<VoxelTerrain::EditMode>();

			for(size_t i = 0; i < enumValues.size(); ++i) {
				bool isSelected = (vt->editMode_ == enumValues[i]);

				/// 選択したモードに変更
				if(ImGui::Selectable(enumNames[i].data(), isSelected)) {
					vt->editMode_ = enumValues[i];
				}

				if(isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}


		/// ---------------------------------------------------
		/// ブラシサイズと強さの変更
		/// ---------------------------------------------------
		static int   radius = 5;
		static float strength = 0.5f;

		/// ブラシサイズの変更
		Editor::DragInt("Brush Radius", radius, 1, 1, 100);
		if(Input::PressKey(DIK_LSHIFT) && Input::GetMouseWheel() != 0.0f) {
			int delta = static_cast<int>(Input::GetMouseWheel());
			delta = (delta > 0) ? 1 : -1;
			radius += delta;
			radius = std::clamp(radius, 1, 100);
			isEdit = true;
		}

		/// ブラシの強さの変更
		Editor::DragFloat("Strength", strength, 0.01f, 0.0f, 1.0f);
		if(Input::PressKey(DIK_LALT) && Input::GetMouseWheel() != 0.0f) {
			float delta = Input::GetMouseWheel();
			delta = (delta > 0) ? 0.01f : -0.01f;
			strength += delta;
			strength = std::clamp(strength, 0.0f, 1.0f);
			isEdit = true;
		}

		isEdit |= Editor::SliderInt("MaterialID", vt->materialId_, 0, 2);



		/// ブラシサイズや強さが変更されたとき
		if(isEdit) {
			vt->cBufferEditInfo_.SetMappedData({ uint32_t(radius), strength, uint32_t(vt->materialId_) });

			/// マウスを動かしたら表示を消す
			const Vector2& mouseVelocity = Input::GetMouseVelocity();
			if(std::abs(mouseVelocity.x) > 0.01f && std::abs(mouseVelocity.y) > 0.01f) {
				isEdit = false;
			}

			/// マウスの位置にブラシの情報を表示
			ImVec2 mousePos = ImGui::GetMousePos();
			float offset = 20.0f; // マウス位置から少し離すオフセット
			mousePos.x += offset;

			ImGui::SetNextWindowPos(mousePos);
			ImGui::Begin(
				"MouseText",
				nullptr,
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_AlwaysAutoResize
			);

			ImGui::Text("Edit Mode: %s", currentModeName.data());
			ImGui::Text("Radius: %d", radius);
			ImGui::Text("Strength: %.2f", strength);
			ImGui::End();
		}

	}






	ImGui::SeparatorText("TextureExport");

	/// テクスチャを初期の状態で保存する
	if(ImGui::Button("地形を初期状態に戻す")) {
		for(size_t i = 0; i < vt->maxChunkCount_; i++) {
			const std::wstring filename = L"./Packages/Textures/Terrain/Chunk/" + std::to_wstring(i) + L".dds";
			Asset::SaveTextureToDDS(
				filename,
				vt->textureSize_.x,
				vt->textureSize_.y,
				vt->textureSize_.z,
				true
			);
		}
	}

	ImGui::Spacing();

	/// 出力用
	if(ImGui::Button("地形を保存する")) {
		std::wstring filepath = L"";
		for(size_t i = 0; i < vt->editedChunkIDs_.size(); i++) {
			int id = vt->editedChunkIDs_[i];
			if(id < 0 || id >= static_cast<int>(vt->chunks_.size())) {
				Console::LogError("Invalid chunk ID: " + std::to_string(id));
				continue;
			}

			filepath = L"./Packages/Textures/Terrain/Chunk/" + std::to_wstring(id) + L".dds";

			const Chunk& chunk = vt->chunks_[id];
			chunk.pTexture->OutputTexture3D(filepath, dxm->GetDxDevice(), dxm->GetDxCommand());
			Console::Log("Chunk " + std::to_string(id) + ": Texture3D GUID = " + chunk.texture3DId.ToString());
		}
	}

}


void ONEngine::from_json(const nlohmann::json& j, std::vector<Chunk>& chunks) {
	nlohmann::json jChunks = j["chunks"];

	chunks.resize(jChunks.size());

	std::string key;
	for(size_t i = 0; i < jChunks.size(); i++) {
		key = std::to_string(i);
		if(jChunks.contains(key)) {
			chunks[i] = Chunk{ jChunks[key], nullptr };
		}
	}
}

void ONEngine::to_json(nlohmann::json& j, const std::vector<Chunk>& chunks) {
	nlohmann::json jChunks;

	for(size_t i = 0; i < chunks.size(); i++) {
		jChunks[std::to_string(i)] = chunks[i].texture3DId;
	}

	j = {
		{ "chunks", jChunks }
	};
}

void ONEngine::from_json(const nlohmann::json& j, VoxelTerrain& vt) {
	/// Json -> VoxelTerrain
	vt.enable = j.value("enable", 1);

	vt.maxChunkCount_ = j.value("maxChunkCount", 400);
	vt.chunkCountXZ_ = j.value("chunkCountXZ", Vector2Int{ 2, 2 });
	vt.chunkSize_ = j.value("chunkSize", Vector3Int{ 16, 128, 16 });
	vt.textureSize_ = j.value("textureSize", Vector3Int{ 32, 32, 32 });
	vt.isoLevel_ = j.value("isoLevel", 0.5f);

	vt.material_ = j.value("material", Asset::Material{});
	vt.cliffMaterial_ = j.value("cliffMaterial", Asset::Material{});
	vt.chunks_ = j.value("chunks", std::vector<Chunk>{});

	//vt.usedTextureIds_.usedBit = j.value("usedTextureIds.usedBit", 0);
	for(int i = 0; i < 3; ++i) {
		//const std::string str = "usedTextureIds.id" + std::to_string(i);
		//vt.usedTextureIds_.ids[i] = j.value(str, 0);
		const std::string guidKey = "usedTextureGuids" + std::to_string(i);
		vt.usedTextureGuids_[i] = j.value(guidKey, Guid::kInvalid);
	}


	vt.lodInfo_.useLOD = j.value("useLOD", 1);
	vt.lodInfo_.lodDistance0 = j.value("lod0Distance", 50.0f);
	vt.lodInfo_.lodDistance1 = j.value("lod1Distance", 100.0f);
	vt.lodInfo_.lodDistance2 = j.value("lod2Distance", 200.0f);
	vt.lodInfo_.lodLevel0 = j.value("lodLevel0", 0);
	vt.lodInfo_.lodLevel1 = j.value("lodLevel1", 1);
	vt.lodInfo_.lodLevel2 = j.value("lodLevel2", 2);
	vt.lodInfo_.lodLevel3 = j.value("lodLevel3", 3);
	vt.lodInfo_.maxDrawDistance = j.value("maxDrawDistance", 1000.0f);
	vt.lodInfo_.lod = j.value("lod", 1);
}

void ONEngine::to_json(nlohmann::json& j, const VoxelTerrain& voxelTerrain) {
	/// VoxelTerrain -> Json
	j = {
		{ "type", "VoxelTerrain" },
		{ "enable", voxelTerrain.enable },
		{ "maxChunkCount", voxelTerrain.maxChunkCount_ },
		{ "chunkSize", voxelTerrain.chunkSize_ },
		{ "textureSize", voxelTerrain.textureSize_ },
		{ "chunkCountXZ", voxelTerrain.chunkCountXZ_ },
		{ "isoLevel", voxelTerrain.isoLevel_ },
		{ "material", voxelTerrain.material_ },
		{ "cliffMaterial", voxelTerrain.cliffMaterial_ },
		{ "chunks", voxelTerrain.chunks_ },

		{ "useLOD", voxelTerrain.lodInfo_.useLOD },
		{ "lod0Distance", voxelTerrain.lodInfo_.lodDistance0 },
		{ "lod1Distance", voxelTerrain.lodInfo_.lodDistance1 },
		{ "lod2Distance", voxelTerrain.lodInfo_.lodDistance2 },
		{ "lodLevel0", voxelTerrain.lodInfo_.lodLevel0 },
		{ "lodLevel1", voxelTerrain.lodInfo_.lodLevel1 },
		{ "lodLevel2", voxelTerrain.lodInfo_.lodLevel2 },
		{ "lodLevel3", voxelTerrain.lodInfo_.lodLevel3 },
		{ "maxDrawDistance", voxelTerrain.lodInfo_.maxDrawDistance },
		{ "lod", voxelTerrain.lodInfo_.lod }
	};

	//j["usedTextureIds.usedBit"] = voxelTerrain.usedTextureIds_.usedBit;
	for(int i = 0; i < 3; ++i) {
		//const std::string str = "usedTextureIds.id" + std::to_string(i);
		//j[str] = voxelTerrain.usedTextureIds_.ids[i];

		const std::string guidStr = "usedTextureGuids" + std::to_string(i);
		j[guidStr] = voxelTerrain.usedTextureGuids_[i];
	}
}


/// ///////////////////////////////////////////////////
/// ボクセルで表現された地形
/// ///////////////////////////////////////////////////

VoxelTerrain::VoxelTerrain() {
	/// x*z でチャンクが並ぶ想定
	chunkCountXZ_ = Vector2Int{ 10, 10 };
	chunkSize_ = Vector3Int{ 16, 128, 16 };
	maxChunkCount_ = static_cast<UINT>(chunkCountXZ_.x * chunkCountXZ_.y);
	for(int i = 0; i < 3; ++i) {
		usedTextureGuids_[i] = Guid::kInvalid;
	}
}

VoxelTerrain::~VoxelTerrain() {
	if(pDxSRVHeap_) {
		/// 使用しているUAVテクスチャの解放を行う
		for(auto& chunk : chunks_) {
			if(chunk.uavTexture.HasUAVHandle()) {
				pDxSRVHeap_->Free(chunk.uavTexture.GetUAVHandle().descriptorIndex);
			}
		}
	}
}

void VoxelTerrain::SettingChunksGuid(Asset::AssetCollection* assetCollection) {
	maxChunkCount_ = static_cast<size_t>(chunkCountXZ_.x * chunkCountXZ_.y);
	if(maxChunkCount_ > chunks_.size()) {
		chunks_.resize(maxChunkCount_);
	}

	for(size_t i = 0; i < maxChunkCount_; i++) {
		/// indexを元にファイルパスを生成
		const std::string filepath = "./Packages/Textures/Terrain/Chunk/" + std::to_string(i) + ".dds";

		/// AssetCollectionからGuidを取得して設定
		const Guid& texture3DGuid = assetCollection->GetAssetGuidFromPath(filepath);
		chunks_[i].texture3DId = texture3DGuid;

		Asset::Texture* texture = assetCollection->GetTextureFromGuid(texture3DGuid);
		chunks_[i].pTexture = texture;
	}
}

bool VoxelTerrain::CheckCreatedBuffers() const {
	bool isCreated = false;

	/// BufferのリソースポインタがNullかどうかで判定
	isCreated |= cBufferTerrainInfo_.Get() != nullptr;
	isCreated |= sBufferChunks_.GetResource().Get() != nullptr;

	return isCreated;
}

void VoxelTerrain::CreateBuffers(DxDevice* dxDevice, DxSRVHeap* dxSRVHeap, Asset::AssetCollection* assetCollection) {
	UINT chunkCount = static_cast<UINT>(32 * 32);

	cBufferTerrainInfo_.Create(dxDevice);
	sBufferChunks_.Create(chunkCount, dxDevice, dxSRVHeap);
	cBufferMaterial_.Create(dxDevice);
	cBufferCliffMaterial_.Create(dxDevice);
	cBufferLODInfo_.Create(dxDevice);
	cBufferUsedTextureIds_.Create(dxDevice);


	/// ChunkArrayの設定
	for(size_t i = 0; i < maxChunkCount_; i++) {
		const Asset::Texture* texture = assetCollection->GetTextureFromGuid(chunks_[i].texture3DId);
		if(texture) {
			sBufferChunks_.SetMappedData(i, GPUData::Chunk{ static_cast<uint32_t>(texture->GetSRVDescriptorIndex()) });
		} else {
			const Asset::Texture* frontTex = assetCollection->GetTextureFromGuid(chunks_[0].texture3DId);
			sBufferChunks_.SetMappedData(i, GPUData::Chunk{ static_cast<uint32_t>(frontTex->GetSRVDescriptorIndex()) });
		}
	}
}

void VoxelTerrain::SetupGraphicBuffers(ID3D12GraphicsCommandList* cmdList, const std::array<UINT, 6> rootParamIndices, Asset::AssetCollection* assetCollection) {
	maxChunkCount_ = static_cast<UINT>(chunkCountXZ_.x * chunkCountXZ_.y);

	/// VoxelTerrainInfoの設定
	Vector3 terrainOrigin = GetOwner()->GetTransform()->GetPosition();
	cBufferTerrainInfo_.SetMappedData(GPUData::VoxelTerrainInfo{ terrainOrigin, 0, textureSize_, 0, chunkSize_, 0, chunkCountXZ_, maxChunkCount_, isoLevel_ });
	cBufferTerrainInfo_.BindForGraphicsCommandList(cmdList, rootParamIndices[0]);

	/// LODの設定
	cBufferLODInfo_.SetMappedData(lodInfo_);
	cBufferLODInfo_.BindForGraphicsCommandList(cmdList, rootParamIndices[3]);

	/// Materialの設定
	SettingMaterial(assetCollection);
	cBufferMaterial_.BindForGraphicsCommandList(cmdList, rootParamIndices[1]);
	cBufferCliffMaterial_.BindForGraphicsCommandList(cmdList, rootParamIndices[4]);

	/// UsedTextureIDs
	cBufferUsedTextureIds_.SetMappedData(usedTextureIds_);
	cBufferUsedTextureIds_.BindForGraphicsCommandList(cmdList, rootParamIndices[5]);

	/// ChunkArrayの設定
	for(size_t i = 0; i < maxChunkCount_; i++) {
		const Asset::Texture* texture = assetCollection->GetTextureFromGuid(chunks_[i].texture3DId);
		if(texture) {
			sBufferChunks_.SetMappedData(i, GPUData::Chunk{ static_cast<uint32_t>(texture->GetSRVDescriptorIndex()) });
		} else {
			const Asset::Texture* frontTex = assetCollection->GetTextureFromGuid(chunks_[0].texture3DId);
			sBufferChunks_.SetMappedData(i, GPUData::Chunk{ static_cast<uint32_t>(frontTex->GetSRVDescriptorIndex()) });
		}
	}

	sBufferChunks_.SRVBindForGraphicsCommandList(cmdList, rootParamIndices[2]);
}

void VoxelTerrain::TransitionTextureStates(DxCommand* dxCommand, Asset::AssetCollection* assetCollection, D3D12_RESOURCE_STATES afterState) {
	/// チャンク用テクスチャの状態遷移
	std::vector<DxResource*> resources;
	resources.reserve(maxChunkCount_);
	for(size_t i = 0; i < maxChunkCount_; i++) {
		const Guid& guid = chunks_[i].texture3DId;
		chunks_[i].pTexture = assetCollection->GetTextureFromGuid(guid);
		if(chunks_[i].pTexture) {
			resources.push_back(&chunks_[i].pTexture->GetDxResource());
		}
	}

	CreateBarriers(resources, afterState, dxCommand);
}

UINT VoxelTerrain::MaxChunkCount() const {
	return maxChunkCount_;
}

const Vector2Int& VoxelTerrain::GetChunkCountXZ() const {
	return chunkCountXZ_;
}

const Vector3Int& VoxelTerrain::GetChunkSize() const {
	return chunkSize_;
}

void VoxelTerrain::SettingMaterial(Asset::AssetCollection* assetCollection) {
	{	/// DefaultMaterialの設定
		int32_t baseTextureId = 0;
		if(material_.HasBaseTexture()) {
			baseTextureId = assetCollection->GetTextureFromGuid(
				material_.GetBaseTextureGuid())->GetSRVDescriptorIndex();
		}

		int32_t normalTextureId = 0;
		if(material_.HasNormalTexture()) {
			normalTextureId = assetCollection->GetTextureFromGuid(
				material_.GetNormalTextureGuid())->GetSRVDescriptorIndex();
		}

		/// Materialの設定
		cBufferMaterial_.SetMappedData(
			{
				.baseColor = material_.baseColor,
				.postEffectFlags = material_.postEffectFlags,
				.entityId = GetOwner()->GetId(),
				.baseTextureId = baseTextureId,
				.normalTextureId = normalTextureId
			}
		);
	}

	{	/// CliffMaterialの設定
		int32_t baseTextureId = 0;
		if(cliffMaterial_.HasBaseTexture()) {
			baseTextureId = assetCollection->GetTextureFromGuid(
				cliffMaterial_.GetBaseTextureGuid())->GetSRVDescriptorIndex();
		}
		int32_t normalTextureId = 0;
		if(cliffMaterial_.HasNormalTexture()) {
			normalTextureId = assetCollection->GetTextureFromGuid(
				cliffMaterial_.GetNormalTextureGuid())->GetSRVDescriptorIndex();
		}

		cBufferCliffMaterial_.SetMappedData(
			{
				.baseColor = cliffMaterial_.baseColor,
				.postEffectFlags = cliffMaterial_.postEffectFlags,
				.entityId = GetOwner()->GetId(),
				.baseTextureId = baseTextureId,
				.normalTextureId = normalTextureId
			}
		);
	}


	{	/// その他三つのテクスチャを設定
		for(int i = 0; i < 3; ++i) {
			const Guid& guid = usedTextureGuids_[i];
			if(!guid.CheckValid()) { continue; }
			usedTextureIds_.ids[i] = assetCollection->GetTextureFromGuid(guid)->GetSRVDescriptorIndex();
		}
	}
}

void VoxelTerrain::SettingTerrainInfo() {
	cBufferTerrainInfo_.SetMappedData(
		GPUData::VoxelTerrainInfo{
			.terrainOrigin = GetOwner()->GetTransform()->GetPosition(),
			.textureSize = textureSize_, .chunkSize = chunkSize_,
			.chunkCountXZ = chunkCountXZ_, .maxChunkCount = maxChunkCount_,
			.isoLevel = isoLevel_
		}
	);
}

bool VoxelTerrain::CheckBufferCreatedForEditor() const {
	bool result = false;

	result |= cBufferInputInfo_.Get() != nullptr;
	result |= cBufferEditInfo_.Get() != nullptr;

	return result;
}

void VoxelTerrain::CreateEditorBuffers(DxDevice* dxDevice, DxSRVHeap* dxSRVHeap) {
	UINT chunkCount = static_cast<UINT>(32 * 32);

	cBufferInputInfo_.Create(dxDevice);
	cBufferEditInfo_.Create(dxDevice);
	sBufferEditorChunks_.Create(chunkCount, dxDevice, dxSRVHeap);
	cBufferTerrainInfo_.Create(dxDevice);
	cBufferUsedTextureIds_.Create(dxDevice);
	cBufferBitMask_.Create(dxDevice);
}

void VoxelTerrain::SetupEditorBuffers(ID3D12GraphicsCommandList* cmdList, const std::array<UINT, 5> rootParamIndices, const GPUData::InputInfo& inputInfo) {
	/// InputInfoの設定
	cBufferInputInfo_.SetMappedData(inputInfo);
	cBufferInputInfo_.BindForComputeCommandList(cmdList, rootParamIndices[0]);
	/// TerrainInfoの設定
	SettingTerrainInfo();
	cBufferTerrainInfo_.BindForComputeCommandList(cmdList, rootParamIndices[1]);
	/// EditInfoの設定
	cBufferEditInfo_.BindForComputeCommandList(cmdList, rootParamIndices[2]);
	/// BitMask
	cBufferBitMask_.SetMappedData(editBitMask_);
	cBufferBitMask_.BindForComputeCommandList(cmdList, rootParamIndices[4]);

	/// ChunkArrayの設定
	for(size_t i = 0; i < maxChunkCount_; i++) {
		sBufferEditorChunks_.SetMappedData(i, GPUData::Chunk{ static_cast<uint32_t>(chunks_[i].uavTexture.GetUAVDescriptorIndex()) });
	}
	/// Chunk
	sBufferEditorChunks_.SRVBindForComputeCommandList(cmdList, rootParamIndices[3]);
}

void VoxelTerrain::CreateChunkTextureUAV(DxCommand* dxCommand, DxDevice* dxDevice, DxSRVHeap* dxSRVHeap) {

	pDxSRVHeap_ = dxSRVHeap;
	for(auto& chunk : chunks_) {
		chunk.uavTexture.CreateUAVTexture3D(
			static_cast<UINT>(textureSize_.x),
			static_cast<UINT>(textureSize_.y),
			static_cast<UINT>(textureSize_.z),
			dxDevice, dxSRVHeap,
			DXGI_FORMAT_R8G8B8A8_UNORM
		);
	}

	//for(size_t i = 0; i < maxChunkCount_; i++) {
	//	Chunk& chunk = chunks_[i];
	//	const uint32_t vertexCount = 80000;
	//	chunk.rwVertices.CreateUAV(vertexCount, dxDevice, dxCommand, dxSRVHeap);
	//	chunk.rwVertexCounter.CreateUAV(vertexCount, dxDevice, dxCommand, dxSRVHeap);
	//	chunk.vbv.Create(1, dxDevice, dxCommand);
	//	chunk.vbv.Resize(1);
	//}


	D3D12_RESOURCE_STATES srvTextureBefore = chunks_[0].pTexture->GetDxResource().GetCurrentState();
	D3D12_RESOURCE_STATES uavTextureBefore = chunks_[0].uavTexture.GetDxResource().GetCurrentState();

	auto cmdList = dxCommand->GetCommandList();
	/// テクスチャの状態遷移
	for(auto& chunk : chunks_) {
		chunk.pTexture->GetDxResource().CreateBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, dxCommand);
		chunk.uavTexture.GetDxResource().CreateBarrier(D3D12_RESOURCE_STATE_COPY_DEST, dxCommand);
	}

	/// 実際に使用するSRVをUAVテクスチャにコピーする
	for(auto& chunk : chunks_) {
		cmdList->CopyResource(
			chunk.uavTexture.GetDxResource().Get(),
			chunk.pTexture->GetDxResource().Get()
		);
	}

	/// テクスチャの状態遷移
	for(auto& chunk : chunks_) {
		chunk.pTexture->GetDxResource().CreateBarrier(srvTextureBefore, dxCommand);
		chunk.uavTexture.GetDxResource().CreateBarrier(uavTextureBefore, dxCommand);
	}
}

void VoxelTerrain::CopyEditorTextureToChunkTexture(DxCommand* dxCommand) {
	D3D12_RESOURCE_STATES srvTextureBefore = chunks_[0].pTexture->GetDxResource().GetCurrentState();
	D3D12_RESOURCE_STATES uavTextureBefore = chunks_[0].uavTexture.GetDxResource().GetCurrentState();

	auto cmdList = dxCommand->GetCommandList();
	/// テクスチャの状態遷移
	for(auto& chunk : chunks_) {
		chunk.uavTexture.GetDxResource().CreateBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, dxCommand);
		chunk.pTexture->GetDxResource().CreateBarrier(D3D12_RESOURCE_STATE_COPY_DEST, dxCommand);
	}

	/// 実際に使用するSRVをUAVテクスチャにコピーする
	for(auto& chunk : chunks_) {
		cmdList->CopyResource(
			chunk.pTexture->GetDxResource().Get(),
			chunk.uavTexture.GetDxResource().Get()
		);
	}

	/// テクスチャの状態遷移
	for(auto& chunk : chunks_) {
		chunk.uavTexture.GetDxResource().CreateBarrier(uavTextureBefore, dxCommand);
		chunk.pTexture->GetDxResource().CreateBarrier(srvTextureBefore, dxCommand);
	}
}

void VoxelTerrain::CopyEditorTextureToChunkTexture(DxCommand* dxCommand, const std::vector<int>& copyChunkIDs) {
	D3D12_RESOURCE_STATES srvTextureBefore = chunks_[0].pTexture->GetDxResource().GetCurrentState();
	D3D12_RESOURCE_STATES uavTextureBefore = chunks_[0].uavTexture.GetDxResource().GetCurrentState();
	auto cmdList = dxCommand->GetCommandList();

	auto EnableChunkID = [&](int chunkID) -> bool {
		return chunkID >= 0 && static_cast<size_t>(chunkID) < chunks_.size();
	};

	/// テクスチャの状態遷移
	for(const int chunkID : copyChunkIDs) {
		if(!EnableChunkID(chunkID)) {
			continue;
		}

		chunks_[chunkID].uavTexture.GetDxResource().CreateBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, dxCommand);
		chunks_[chunkID].pTexture->GetDxResource().CreateBarrier(D3D12_RESOURCE_STATE_COPY_DEST, dxCommand);
	}

	/// 実際に使用するSRVをUAVテクスチャにコピーする
	for(const int chunkID : copyChunkIDs) {
		if(!EnableChunkID(chunkID)) {
			continue;
		}

		cmdList->CopyResource(
			chunks_[chunkID].pTexture->GetDxResource().Get(),
			chunks_[chunkID].uavTexture.GetDxResource().Get()
		);
	}

	/// テクスチャの状態遷移
	for(const int chunkID : copyChunkIDs) {
		if(!EnableChunkID(chunkID)) {
			continue;
		}

		chunks_[chunkID].uavTexture.GetDxResource().CreateBarrier(uavTextureBefore, dxCommand);
		chunks_[chunkID].pTexture->GetDxResource().CreateBarrier(srvTextureBefore, dxCommand);
	}
}

void VoxelTerrain::PushBackEditChunkID(const std::vector<int>& editChunkID) {
	editedChunkIDs_.insert(editedChunkIDs_.end(), editChunkID.begin(), editChunkID.end());

	/// 昇順ソートの後、重複IDを削除
	std::sort(editedChunkIDs_.begin(), editedChunkIDs_.end());
	editedChunkIDs_.erase(std::unique(editedChunkIDs_.begin(), editedChunkIDs_.end()), editedChunkIDs_.end());
}

uint32_t VoxelTerrain::GetBrushRadius() const {
	if(!cBufferEditInfo_.Get()) {
		return 0;
	}

	return cBufferEditInfo_.GetMappingData().brushRadius;
}

float ONEngine::VoxelTerrain::GetBrushStrength() const {
	if(!cBufferEditInfo_.Get()) {
		return 0.0f;
	}
	return cBufferEditInfo_.GetMappingData().strength;
}


}