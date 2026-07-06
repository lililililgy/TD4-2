#include "DebugSceneView.h"

/// std
#include <array>
#include <thread>
#include <atomic>
#include <windows.h>
#include <filesystem>
#include <fstream>

/// engine
#include "Engine/Editor/Manager/HotReloadManager.h"
#include "Engine/Core/Utility/Time/Time.h"

/// external
#include <imgui.h>
#include <ImGuizmo.h>

/// engine
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/Utility/Utility.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Script/MonoScriptEngine.h"
#include "Engine/Core/DirectX12/GPUTimeStamp/GPUTimeStamp.h"
#include "Engine/Core/Utility/Time/CPUTimeStamp.h"

/// editor
#include "Engine/Editor/EditorUtils.h"
#include "Engine/Editor/Manager/ImGuiManager.h"
#include "Engine/Editor/Math/ImGuiSelection.h"
#include "InspectorWindow.h"

namespace {
template<typename... Args>
std::string Format(const char* fmt, Args... args) {
	int size = std::snprintf(nullptr, 0, fmt, args...) + 1;
	std::string buf(size, '\0');
	std::snprintf(buf.data(), size, fmt, args...);
	buf.pop_back(); // null文字削除
	return buf;
}

std::string ConvertACPToUTF8(const std::string& acpStr) {
	int wlen = MultiByteToWideChar(CP_ACP, 0, acpStr.c_str(), -1, NULL, 0);
	if (wlen > 0) {
		std::vector<wchar_t> wbuf(wlen);
		MultiByteToWideChar(CP_ACP, 0, acpStr.c_str(), -1, wbuf.data(), wlen);
		int ulen = WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), -1, NULL, 0, NULL, NULL);
		if (ulen > 0) {
			std::vector<char> ubuf(ulen);
			WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), -1, ubuf.data(), ulen, NULL, NULL);
			return std::string(ubuf.data());
		}
	}
	return acpStr;
}
}


namespace Editor {

DebugSceneView::DebugSceneView(ONEngine::EntityComponentSystem* ecs, ONEngine::Asset::AssetCollection* assetCollection, ONEngine::SceneManager* sceneManager, InspectorWindow* inspector)
	: pEcs_(ecs), pAssetCollection_(assetCollection), pSceneManager_(sceneManager), pInspector_(inspector) {

}


void DebugSceneView::ShowImGui() {
	if(!ImGui::Begin("Scene")) {
		ImGui::End();
		return;
	}

	HandleCameraFocus();
	DrawToolbar();

	ImGui::Separator();

	ImVec2 imagePos, imageSize;
	DrawSceneTexture(imagePos, imageSize);

	DrawGizmoAndOverlays(imagePos, imageSize);

	ImGui::End();
}

void DebugSceneView::SetGamePlay(bool isGamePlay) {
	ONEngine::DebugConfig::isDebugging = isGamePlay;
	ONEngine::DebugConfig::isPause = false; // 一時停止状態をリセット

	/// ゲームの開始処理
	if(ONEngine::DebugConfig::isDebugging) {
		ONEngine::Console::ClearLogBuffer(ONEngine::LogCategory::Application);
		pSceneManager_->SaveCurrentSceneTemporary();

		/// Monoスクリプトエンジンのホットリロードでスクリプトの初期化を行う
		/// シーンをロードする前にドメインを最新の状態にしておく
		ONEngine::MonoScriptEngine::GetInstance().HotReload();

		pSceneManager_->ReloadScene(true);
		// ImGuiSelection::SetSelectedObject(ONEngine::Guid::kInvalid, SelectionType::None);
	} else {

		/// 共通の処理（ゲーム開始、停止時に行う処理）
		pSceneManager_->ReloadScene(true);
		pSceneManager_->ClearTemporarySavedSceneName();
		// ImGuiSelection::SetSelectedObject(ONEngine::Guid::kInvalid, SelectionType::None);
	}

}

void Editor::DebugSceneView::ShowDebugSceneView(const ImVec2& imagePos) {
	std::vector<OverlaySection> leftSections;

	{
		// アクティブシーン セクション
		const auto& activeScenes = pEcs_->GetActiveGroupNames();
		std::string currentScene = pEcs_->GetCurrentGroupName();

		OverlaySection sceneSection;
		sceneSection.name = "アクティブシーン";
		sceneSection.opened = true;

		sceneSection.items.push_back({ "Current Scene", currentScene, IM_COL32(100, 255, 100, 255) });
		if (activeScenes.empty()) {
			sceneSection.items.push_back({ "Active Scenes", "None (Single)", IM_COL32(255, 255, 255, 255) });
		} else {
			for (size_t i = 0; i < activeScenes.size(); ++i) {
				sceneSection.items.push_back({ "Scene [" + std::to_string(i) + "]", activeScenes[i], IM_COL32(255, 255, 255, 255) });
			}
		}
		leftSections.push_back(sceneSection);
	}

	{
		// 地形描画 セクション
		double regularCellTime = ONEngine::GPUTimeStamp::GetInstance().GetTimeStampMSec(ONEngine::GPUTimeStampID::VoxelTerrainRegularCell); // ms
		double transitionCellTime = ONEngine::GPUTimeStamp::GetInstance().GetTimeStampMSec(ONEngine::GPUTimeStampID::VoxelTerrainTransitionCell); // ms
		double editorComputeTime = ONEngine::GPUTimeStamp::GetInstance().GetTimeStampMSec(ONEngine::GPUTimeStampID::VoxelTerrainEditorCompute); // ms
		double editorComputeBrushPreview = ONEngine::GPUTimeStamp::GetInstance().GetTimeStampMSec(ONEngine::GPUTimeStampID::VoxelTerrainEditorBrushPreview); // ms
		OverlaySection renderer;
		renderer.name = "地形描画";
		renderer.opened = true;
		renderer.items = {
			{ "RegularCell", Format("%f ms", regularCellTime), IM_COL32(255, 255, 255, 255) },
			{ "TransitionCell", Format("%f ms", transitionCellTime), IM_COL32(255, 255, 255, 255) },
			{ "EditorCompute", Format("%f ms", editorComputeTime), IM_COL32(255, 255, 255, 255) },
			{ "BrushPreview", Format("%f ms", editorComputeBrushPreview), IM_COL32(255, 255, 255, 255) },
		};
		leftSections.push_back(renderer);
	}

	{
		/// CPUパフォーマンス セクション
		auto& cpu = ONEngine::CPUTimeStamp::GetInstance();
		double scriptUpdateTime = cpu.GetElapsedTimeMicroseconds(ONEngine::CPUTimeStampID::CSharpScriptUpdate) / 1000.0;
		double ecsUpdateTime = cpu.GetElapsedTimeMicroseconds(ONEngine::CPUTimeStampID::ECSUpdate) / 1000.0;
		double renderUpdateTime = cpu.GetElapsedTimeMicroseconds(ONEngine::CPUTimeStampID::RenderUpdate) / 1000.0;
		double physicsUpdateTime = cpu.GetElapsedTimeMicroseconds(ONEngine::CPUTimeStampID::PhysicsUpdate) / 1000.0;
		double totalCpuTime = scriptUpdateTime + ecsUpdateTime + renderUpdateTime + physicsUpdateTime;

		OverlaySection cpuSection;
		cpuSection.name = "CPUパフォーマンス";
		cpuSection.opened = true;
		cpuSection.items = {
			{ "Total CPU", Format("%.3f ms", totalCpuTime), IM_COL32(255, 255, 100, 255) },
			{ "C# Script", Format("%.3f ms", scriptUpdateTime), IM_COL32(255, 255, 255, 255) },
			{ "ECS Update", Format("%.3f ms", ecsUpdateTime), IM_COL32(255, 255, 255, 255) },
			{ "Physics", Format("%.3f ms", physicsUpdateTime), IM_COL32(255, 255, 255, 255) },
			{ "Render Update", Format("%.3f ms", renderUpdateTime), IM_COL32(255, 255, 255, 255) },
			{ "FPS", Format("%.1f", 1.0f / ONEngine::Time::DeltaTime()), IM_COL32(100, 255, 100, 255) }
		};
		leftSections.push_back(cpuSection);
	}

	{
		/// GPUパフォーマンス セクション
		auto& gpu = ONEngine::GPUTimeStamp::GetInstance();
		OverlaySection gpuSection;
		gpuSection.name = "GPUパフォーマンス";
		gpuSection.opened = true;
		gpuSection.items = {
			{ "Total GPU", Format("%.3f ms", gpu.GetTimeStampMSec(ONEngine::GPUTimeStampID::RenderingTotal)), IM_COL32(255, 255, 100, 255) },
			{ "ShadowMap", Format("%.3f ms", gpu.GetTimeStampMSec(ONEngine::GPUTimeStampID::ShadowMap)), IM_COL32(255, 255, 255, 255) },
			{ "MainScene", Format("%.3f ms", gpu.GetTimeStampMSec(ONEngine::GPUTimeStampID::MainScene)), IM_COL32(255, 255, 255, 255) },
			{ "PostProcess", Format("%.3f ms", gpu.GetTimeStampMSec(ONEngine::GPUTimeStampID::PostProcess)), IM_COL32(255, 255, 255, 255) },
			{ "Mesh", Format("%.3f ms", gpu.GetTimeStampMSec(ONEngine::GPUTimeStampID::MeshRendering)), IM_COL32(200, 200, 255, 255) },
			{ "SkinMesh", Format("%.3f ms", gpu.GetTimeStampMSec(ONEngine::GPUTimeStampID::SkinMeshRendering)), IM_COL32(200, 200, 255, 255) },
			{ "Dissolve", Format("%.3f ms", gpu.GetTimeStampMSec(ONEngine::GPUTimeStampID::DissolveMeshRendering)), IM_COL32(200, 200, 255, 255) },
			{ "Sprite", Format("%.3f ms", gpu.GetTimeStampMSec(ONEngine::GPUTimeStampID::SpriteRendering)), IM_COL32(200, 200, 255, 255) },
			{ "Particle", Format("%.3f ms", gpu.GetTimeStampMSec(ONEngine::GPUTimeStampID::ParticleRendering)), IM_COL32(200, 200, 255, 255) }
		};
		leftSections.push_back(gpuSection);
	}

	// 1カラム目（左側）の描画
	DrawSceneOverlayStats(imagePos, leftSections, 8.0f);

	// 2カラム目（右側）のデータ作成
	std::vector<OverlaySection> rightSections;
	{
		auto& mono = ONEngine::MonoScriptEngine::GetInstance();
		auto* image = mono.Image();
		auto* domain = mono.Domain();
		if (image && domain) {
			auto* aiUpdaterClass = mono_class_from_name(image, "", "AIUpdater");
			if (aiUpdaterClass) {
				auto* vtable = mono_class_vtable(domain, aiUpdaterClass);
				auto* nameField = mono_class_get_field_from_name(aiUpdaterClass, "lastBossName");
				auto* actionField = mono_class_get_field_from_name(aiUpdaterClass, "lastBossAction");
				auto* phaseField = mono_class_get_field_from_name(aiUpdaterClass, "lastBossPhase");

				if (vtable && nameField && actionField && phaseField) {
					MonoString* nameStr = nullptr;
					MonoString* actionStr = nullptr;
					MonoString* phaseStr = nullptr;
					mono_field_static_get_value(vtable, nameField, &nameStr);
					mono_field_static_get_value(vtable, actionField, &actionStr);
					mono_field_static_get_value(vtable, phaseField, &phaseStr);

					std::string bossName = "Unknown";
					if (nameStr) {
						char* cstr = mono_string_to_utf8(nameStr);
						bossName = cstr;
						mono_free(cstr);
					}

					std::string bossAction = "Idle";
					if (actionStr) {
						char* cstr = mono_string_to_utf8(actionStr);
						bossAction = cstr;
						mono_free(cstr);
					}

					std::string bossPhase = "Intro";
					if (phaseStr) {
						char* cstr = mono_string_to_utf8(phaseStr);
						bossPhase = cstr;
						mono_free(cstr);
					}

					OverlaySection bossSection;
					bossSection.name = "ボスの状態監視";
					bossSection.opened = true;
					bossSection.items = {
						{ "Entity Name", bossName, IM_COL32(255, 255, 255, 255) },
						{ "Current Phase", bossPhase, IM_COL32(255, 200, 100, 255) },
						{ "Current Action", bossAction, IM_COL32(100, 255, 255, 255) }
					};
					rightSections.push_back(bossSection);
				}

				// --- GameController の情報を追加 ---
				auto* gcClass = mono_class_from_name(image, "", "GameController");
				if (gcClass) {
					auto* statusField = mono_class_get_field_from_name(gcClass, "currentStatus");
					auto* gcPhaseField = mono_class_get_field_from_name(gcClass, "currentPhase");
					auto* gcVtable = mono_class_vtable(domain, gcClass);

					if (gcVtable && statusField && gcPhaseField) {
						MonoString* statusStr = nullptr;
						MonoString* phaseStr = nullptr;
						mono_field_static_get_value(gcVtable, statusField, &statusStr);
						mono_field_static_get_value(gcVtable, gcPhaseField, &phaseStr);

						std::string gameStatus = "N/A";
						if (statusStr) {
							char* cstr = mono_string_to_utf8(statusStr);
							gameStatus = cstr;
							mono_free(cstr);
						}

						std::string gcPhase = "N/A";
						if (phaseStr) {
							char* cstr = mono_string_to_utf8(phaseStr);
							gcPhase = cstr;
							mono_free(cstr);
						}

						OverlaySection gcSection;
						gcSection.name = "GameController 監視";
						gcSection.opened = true;
						gcSection.items = {
							{ "Game Status", gameStatus, IM_COL32(255, 255, 255, 255) },
							{ "GC Phase Sync", gcPhase, IM_COL32(200, 255, 100, 255) }
						};
						rightSections.push_back(gcSection);
					}
				}
			}
		}
	}

	// 2カラム目（右側）の描画
	if (!rightSections.empty()) {
		DrawSceneOverlayStats(imagePos, rightSections, 250.0f);
	}
}

void DebugSceneView::DrawSceneOverlayStats(const ImVec2& imagePos, const std::vector<OverlaySection>& sections, float xOffset) {
	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	float y = imagePos.y + 8.0f; // 上マージン
	float x = imagePos.x + xOffset; // オフセットに基づいた左マージン

	auto DrawSeparator = [&](const std::vector<OverlayItem>& items)
	{
		float maxWidth = 0.0f;

		// セクション内のテキスト幅を計算して最大値を取得
		for(const auto& item : items) {
			if(!item.visible) continue;
			ImVec2 size = ImGui::CalcTextSize((item.label + " : " + item.value).c_str());
			if(size.x > maxWidth) maxWidth = size.x;
		}

		// 少し余白をつける
		maxWidth += 8.0f;

		drawList->AddLine(
			{ x, y },
			{ x + maxWidth, y },
			IM_COL32(255, 255, 255, 80)
		);

		y += 6.0f; // 線の下にマージン
	};


	auto DrawHeader = [&](const std::string& title, bool opened)
	{
		ImU32 color = opened
			? IM_COL32(255, 220, 120, 255)
			: IM_COL32(180, 180, 180, 255);

		drawList->AddText(
			{ x, y },
			color,
			title.c_str()
		);

		y += ImGui::GetTextLineHeight();
	};

	auto DrawItem = [&](const OverlayItem& item)
	{
		if(!item.visible) return;

		std::string line = item.label + " : " + item.value;
		drawList->AddText(
			{ x, y },
			item.color,
			line.c_str()
		);
		y += ImGui::GetTextLineHeight();
	};

	// 各セクション描画
	for(const auto& section : sections) {
		DrawHeader(section.name, section.opened);
		DrawSeparator(section.items);

		if(section.opened) {
			for(const auto& item : section.items) {
				DrawItem(item);
			}
		}

		y += 6.0f; // セクション間マージン
	}
}

///
/// Fキーによるカメラフォーカス
///
void DebugSceneView::HandleCameraFocus() {
	if(ImGui::IsWindowHovered() || ImGui::IsWindowFocused()) {
		if(ONEngine::Input::TriggerKey(DIK_F)) {
			ONEngine::Guid selectedGuid = ImGuiSelection::GetLastSelectedObject();
			if(selectedGuid.CheckValid()) {
				ONEngine::GameEntity* targetEntity = pEcs_->GetCurrentGroup()->GetEntityFromGuid(selectedGuid);
				if(targetEntity) {
					ONEngine::Vector3 targetPos = targetEntity->GetPosition();
					ONEngine::CameraComponent* editorCamera = pEcs_->GetECSGroup("Debug")->GetMainCamera();
					if(!editorCamera) {
						editorCamera = pEcs_->GetECSGroup("Debug")->GetMainCamera2D();
					}

					if(editorCamera) {
						float distance = 10.0f;
						ONEngine::Vector3 cameraForward = { 0.0f, -0.5f, 1.0f };
						ONEngine::Vector3 newCameraPos;
						newCameraPos.x = targetPos.x - (cameraForward.x * distance);
						newCameraPos.y = targetPos.y - (cameraForward.y * distance);
						newCameraPos.z = targetPos.z - (cameraForward.z * distance);

						ONEngine::Transform* transform = editorCamera->GetOwner()->GetComponent<ONEngine::Transform>();
						transform->SetPosition(newCameraPos);
						editorCamera->LookAt(targetPos - newCameraPos);
						editorCamera->UpdateViewProjection();
					}
				}
			}
		}
	}
}

///
/// ツールバーの表示(再生ボタン、設定チェックボックスなど)
///
void DebugSceneView::DrawToolbar() {
	std::array<const ONEngine::Asset::Texture*, 2> buttons = {
		pAssetCollection_->GetTexture("./Packages/Textures/ImGui/play.png"),
		pAssetCollection_->GetTexture("./Packages/Textures/ImGui/pause.png")
	};

	// dds用フォールバック
	std::array<std::string, 2> paths = {
		"./Packages/Textures/ImGui/play.dds",
		"./Packages/Textures/ImGui/pause.dds"
	};
	for(uint8_t i = 0; i < 2; ++i) {
		if(!buttons[i]) {
			buttons[i] = pAssetCollection_->GetTexture(paths[i]);
		}
	}

	ImVec2 buttonSize = ImVec2(12.0f, 12.0f);
	bool isGameDebug = ONEngine::DebugConfig::isDebugging;

	if(isGameDebug) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.125f, 0.263f, 0.388f, 1.0f));
	}

	if(ImGui::ImageButton("##play", ImTextureID(buttons[0]->GetSRVGPUHandle().ptr), buttonSize)) {
		SetGamePlay(!isGameDebug);
	}
	ImGui::SameLine();

	if(isGameDebug) {
		ImGui::PopStyleColor(1);
	}

	// 一時停止ボタン
	bool isPause = ONEngine::DebugConfig::isPause;
	if(isPause) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.125f, 0.263f, 0.388f, 1.0f));
	}
	if(ImGui::ImageButton("##pause", ImTextureID(buttons[1]->GetSRVGPUHandle().ptr), buttonSize)) {
		if(isGameDebug) {
			ONEngine::DebugConfig::isPause = !isPause;
		}
	}
	if(isPause) {
		ImGui::PopStyleColor(1);
	}

	ImGui::SameLine();

	// デバッグシーンの表示トグル
	if(ImGui::Checkbox("show debug scene", &ONEngine::DebugConfig::isShowDebugScene)) {
		ONEngine::Console::Log("ImGuiSceneWindow::ShowImGui -> clicked show debug scene");
	}

	ImGui::SameLine();

	// スタッツの表示トグル (メンバ変数に変更)
	ImGui::Checkbox("show scene stats", &isDrawSceneStats_);

	ImGui::SameLine();

	// C# Build button
	if (isCSBuilding_) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
		ImGui::Button("Building C#...");
		ImGui::PopStyleColor(3);
	} else {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.15f, 0.45f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.45f, 0.2f, 0.55f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.1f, 0.35f, 1.0f));
		if (ImGui::Button("Build C#")) {
			TriggerCSBuild();
		}
		ImGui::PopStyleColor(3);
	}

	if (showCSBuildResult_) {
		ImGui::SameLine();
		if (csBuildSuccess_) {
			ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Build OK");
		} else {
			ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Build Failed");
		}

		csBuildResultTimer_ -= ONEngine::Time::DeltaTime();
		if (csBuildResultTimer_ <= 0.0f) {
			showCSBuildResult_ = false;
		}
	}

	ImGui::SameLine();

	// 2D/3D モードの切り替え
	bool is2D = Editor::Is2DMode();
	if (ImGui::RadioButton("2D", is2D)) {
		Editor::Set2DMode(true);
		ONEngine::CameraComponent* debugCamComp = nullptr;
		ONEngine::CameraComponent* cam2DComp = nullptr;
		if (auto* debugGroup = pEcs_->GetECSGroup("Debug")) {
			for (auto& entity : debugGroup->GetEntities()) {
				if (entity->GetName() == "DebugCamera") {
					debugCamComp = entity->GetComponent<ONEngine::CameraComponent>();
				} else if (entity->GetName() == "2DCamera") {
					cam2DComp = entity->GetComponent<ONEngine::CameraComponent>();
				}
			}
		}
		if (debugCamComp) {
			debugCamComp->SetIsMainCameraRequest(false);
		}
		if (cam2DComp) {
			cam2DComp->SetIsMainCameraRequest(true);
		}
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("3D", !is2D)) {
		Editor::Set2DMode(false);
		ONEngine::CameraComponent* debugCamComp = nullptr;
		ONEngine::CameraComponent* cam2DComp = nullptr;
		if (auto* debugGroup = pEcs_->GetECSGroup("Debug")) {
			for (auto& entity : debugGroup->GetEntities()) {
				if (entity->GetName() == "DebugCamera") {
					debugCamComp = entity->GetComponent<ONEngine::CameraComponent>();
				} else if (entity->GetName() == "2DCamera") {
					cam2DComp = entity->GetComponent<ONEngine::CameraComponent>();
				}
			}
		}
		if (debugCamComp) {
			debugCamComp->SetIsMainCameraRequest(true);
		}
		if (cam2DComp) {
			cam2DComp->SetIsMainCameraRequest(false);
		}
	}

	ImGui::SameLine();
	ImGui::Text("|");
	ImGui::SameLine();

	// Local/World モードの切り替え
	int pivotMode = Editor::GetPivotMode();
	if (ImGui::RadioButton("World", pivotMode == ImGuizmo::MODE::WORLD)) {
		Editor::SetPivotMode(ImGuizmo::MODE::WORLD);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Local", pivotMode == ImGuizmo::MODE::LOCAL)) {
		Editor::SetPivotMode(ImGuizmo::MODE::LOCAL);
	}

	// ImGuiInfo の右寄せ表示
	{
		ImGui::SameLine();
		const std::string& text = ImGuiInfo::GetInfo();
		float textWidth = ImGui::CalcTextSize(text.c_str()).x;
		float windowWidth = ImGui::GetContentRegionAvail().x;
		ImGui::SetCursorPosX(windowWidth - textWidth);
		ImGui::TextColored(ImVec4(0.75f, 0, 0, 1), text.c_str());
	}
}

///
/// シーンテクスチャの描画と座標計算
///
void DebugSceneView::DrawSceneTexture(ImVec2& outImagePos, ImVec2& outImageSize) {
	const auto& textures = pAssetCollection_->GetTextures();
	const ONEngine::Asset::Texture* texture = &textures[pAssetCollection_->GetTextureIndex("./Assets/Scene/RenderTexture/debugScene")];

	ImVec2 availRegion = ImGui::GetContentRegionAvail();
	float aspectRatio = 16.0f / 9.0f;
	if (ONEngine::EngineConfig::windowHeight > 0) {
		aspectRatio = static_cast<float>(ONEngine::EngineConfig::windowWidth) / static_cast<float>(ONEngine::EngineConfig::windowHeight);
	}

	outImageSize = availRegion;
	if(outImageSize.x / outImageSize.y > aspectRatio) {
		outImageSize.x = outImageSize.y * aspectRatio;
	} else {
		outImageSize.y = outImageSize.x / aspectRatio;
	}

	ImVec2 windowPos = ImGui::GetCursorScreenPos();
	outImagePos = windowPos;
	outImagePos.x += (availRegion.x - outImageSize.x) * 0.5f;
	outImagePos.y += (availRegion.y - outImageSize.y) * 0.5f;

	ImGui::SetCursorScreenPos(outImagePos);
	ImGui::Image(ImTextureID(texture->GetSRVGPUHandle().ptr), outImageSize);

	// 情報保存（ギズモのピッキング等に使用）
	pImGuiManager_->AddSceneImageInfo("Scene", ImGuiSceneImageInfo{ outImagePos, outImageSize, ImGui::IsItemHovered() });
}

///
/// ギズモ操作と統計情報の表示
///
void DebugSceneView::DrawGizmoAndOverlays(const ImVec2& imagePos, const ImVec2& imageSize) {
	Editor::SetEntity(ImGuiSelection::GetLastSelectedObject());

	ONEngine::Vector2 imagePosV = { imagePos.x, imagePos.y };
	ONEngine::Vector2 imageSizeV = { imageSize.x, imageSize.y };
	Editor::SetDrawRect(imagePosV, imageSizeV);
	Editor::UpdatePivot(pEcs_);

	if(isDrawSceneStats_) {
		ShowDebugSceneView(imagePos);
	}
}

void DebugSceneView::TriggerCSBuild() {
	if (isCSBuilding_) return;

	isCSBuilding_ = true;
	showCSBuildResult_ = false;

	std::thread buildThread([this]() {
		ONEngine::Console::Log("Starting C# build...", ONEngine::LogCategory::ScriptEngine);

		// Determine project root directory by traversing upwards
		std::filesystem::path rootPath = std::filesystem::current_path();
		std::filesystem::path checkPath = rootPath;
		while (checkPath.has_parent_path()) {
			if (std::filesystem::exists(checkPath / "SubProjects") && std::filesystem::exists(checkPath / "Project")) {
				rootPath = checkPath;
				break;
			}
			checkPath = checkPath.parent_path();
		}

		std::filesystem::path csprojPath = rootPath / "SubProjects" / "CSharpLibrary" / "CSharpLibrary.csproj";
		std::filesystem::path logPath = rootPath / "temp_cs_build.log";

		// Select dotnet executable path (prioritize absolute path)
		std::wstring dotnetPath = L"dotnet";
		if (std::filesystem::exists("C:\\Program Files\\dotnet\\dotnet.exe")) {
			dotnetPath = L"\"C:\\Program Files\\dotnet\\dotnet.exe\"";
		}

		// Construct command line to run dotnet build directly
		std::wstring cmdLine = dotnetPath + L" build \"" + csprojPath.wstring() + L"\"";

		// Configure security attributes to make file handle inheritable
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;

		HANDLE hLogFile = CreateFileW(
			logPath.wstring().c_str(),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			&sa,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		DWORD exitCode = 9999;
		bool success = false;

		if (hLogFile != INVALID_HANDLE_VALUE) {
			STARTUPINFOW si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			si.hStdOutput = hLogFile;
			si.hStdError = hLogFile;
			si.dwFlags |= STARTF_USESTDHANDLES;
			ZeroMemory(&pi, sizeof(pi));

			// Execute build command directly (bInheritHandles must be TRUE)
			if (CreateProcessW(
				NULL,                   // No module name
				cmdLine.data(),         // Command line
				NULL,                   // Process handle not inheritable
				NULL,                   // Thread handle not inheritable
				TRUE,                   // Inherit handles
				CREATE_NO_WINDOW,       // Do not create console window
				NULL,                   // Use parent's environment block
				NULL,                   // Use parent's starting directory 
				&si,                    // Pointer to STARTUPINFO structure
				&pi)                    // Pointer to PROCESS_INFORMATION structure
			) {
				WaitForSingleObject(pi.hProcess, INFINITE);
				GetExitCodeProcess(pi.hProcess, &exitCode);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				success = (exitCode == 0);
			} else {
				DWORD err = GetLastError();
				ONEngine::Console::LogError("Failed to launch build process (CreateProcessW failed with error: " + std::to_string(err) + ").", ONEngine::LogCategory::ScriptEngine);
			}

			// Close the log file handle so we can read it
			CloseHandle(hLogFile);
		} else {
			DWORD err = GetLastError();
			ONEngine::Console::LogError("Failed to create log file handle (CreateFileW failed with error: " + std::to_string(err) + ").", ONEngine::LogCategory::ScriptEngine);
		}

		// Read and log stdout/stderr of dotnet build
		if (std::filesystem::exists(logPath)) {
			std::ifstream logFile(logPath);
			if (logFile.is_open()) {
				std::string line;
				while (std::getline(logFile, line)) {
					std::string utf8Line = ConvertACPToUTF8(line);
					if (success) {
						ONEngine::Console::Log(utf8Line, ONEngine::LogCategory::ScriptEngine);
					} else {
						ONEngine::Console::LogError(utf8Line, ONEngine::LogCategory::ScriptEngine);
					}
				}
				logFile.close();
			}
			std::error_code ec;
			std::filesystem::remove(logPath, ec);
		} else {
			ONEngine::Console::LogError("Build log file not found at: " + ONEngine::ConvertString(logPath.wstring()), ONEngine::LogCategory::ScriptEngine);
		}

		csBuildSuccess_ = success;
		isCSBuilding_ = false;
		showCSBuildResult_ = true;
		csBuildResultTimer_ = 3.0f; // show result for 3 seconds

		if (success) {
			ONEngine::Console::Log("C# build succeeded! Requesting script hot reload...", ONEngine::LogCategory::ScriptEngine);
			HotReloadManager::GetInstance().RequestScriptHotReload();
		} else {
			ONEngine::Console::LogError("C# build failed! Exit code: " + std::to_string(exitCode) + ", Command: " + ONEngine::ConvertString(cmdLine), ONEngine::LogCategory::ScriptEngine);
		}
	});

	buildThread.detach();
}

} /// namespace Editor
