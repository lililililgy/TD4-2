#pragma once

/// std
#include <string>
#include <vector>

/// externals
#include <imgui.h>

/// engine
#include "../../EditorViewCollection.h"

/// ///////////////////////////////////////////////////
/// ImGuiSceneWindow
/// ///////////////////////////////////////////////////
namespace Editor {

class DebugSceneView : public IEditorWindow {

	struct OverlayItem {
		std::string label;
		std::string value;
		ImU32       color = IM_COL32(255, 255, 255, 255);
		bool        visible = true;
	};

	struct OverlaySection {
		std::string name;
		bool opened = true; // true=表示, false=折りたたみ
		std::vector<OverlayItem> items;
	};

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	DebugSceneView(ONEngine::EntityComponentSystem* ecs, ONEngine::Asset::AssetCollection* assetCollection, ONEngine::SceneManager* sceneManager, class InspectorWindow* inspector);
	~DebugSceneView() {}

	/// @brief imgui windowの描画処理
	void ShowImGui() override;

	/// @brief GamePlayモードの設定
	/// @param isGamePlay Runtime中かどうか
	void SetGamePlay(bool isGamePlay);


	void ShowDebugSceneView(const ImVec2& imagePos);


	/// @brief シーンの統計情報の描画
	void DrawSceneOverlayStats(
		const ImVec2& imagePos,
		const std::vector<OverlaySection>& sections,
		float xOffset
	);
private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	/// ----- 追加するリファクタリング用関数 ----- ///
	void HandleCameraFocus();
	void DrawToolbar();
	void DrawSceneTexture(ImVec2& outImagePos, ImVec2& outImageSize);
	void DrawGizmoAndOverlays(const ImVec2& imagePos, const ImVec2& imageSize);


private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	/// --------------- other class pointers --------------- ///
	ONEngine::EntityComponentSystem* pEcs_;
	ONEngine::Asset::AssetCollection*       pAssetCollection_;
	ONEngine::SceneManager*          pSceneManager_;
	class InspectorWindow*  pInspector_;

	bool isDrawSceneStats_ = true;

	OverlaySection sceneStatsSection_;

};


} /// Editor
