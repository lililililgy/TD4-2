#pragma once

#include <Windows.h>

/// std
#include <memory>
#include <unordered_map>

/// external
#include <imgui.h>

/// engine
#include "Engine/Core/Utility/Math/Vector2.h"

/// editor
#include "Engine/Editor/Views/EditorViewCollection.h"
#include "Engine/Editor/Math/ImGuiSceneImageInfo.h"

namespace ONEngine {
class DxManager;
class WindowManager;
class Window;
class EntityComponentSystem;
class SceneManager;
}

namespace ONEngine::Asset {
class AssetCollection;
}



/// ///////////////////////////////////////////////////
/// ImGuiManager
/// ///////////////////////////////////////////////////
namespace Editor {

class EditorManager;

class ImGuiManager final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	ImGuiManager(ONEngine::DxManager*, ONEngine::WindowManager*, ONEngine::EntityComponentSystem*, EditorManager*, ONEngine::SceneManager*);
	~ImGuiManager();

	/// @brief 初期化
	/// @param assetCollection AssetCollection 
	void Initialize(ONEngine::Asset::AssetCollection* assetCollection);

	/// @brief 終了処理
	void Finalize();

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();


	/// @brief SceneImageInfoを追加する
	/// @param name SceneImageInfoの名前
	/// @param info 追加するSceneImageInfo
	void AddSceneImageInfo(const std::string& name, const ImGuiSceneImageInfo& info);

	/// @brief ImGuiのマウス位置を更新する
	/// @param winHwnd ImGuiを描画しているWindowのHWND
	void UpdateMousePosition(HWND winHwnd);


	/// @brief ImGuiのスタイルを出力する
	/// @param fileName ファイル名
	void OutputImGuiStyle(const std::string& fileName) const;

	/// @brief ImGuiのスタイルを入力する
	/// @param fileName ファイル名
	void InputImGuiStyle(const std::string& fileName) const;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	/// ----- other class ----- ///
	ONEngine::DxManager* pDxManager_;
	ONEngine::WindowManager* pWindowManager_;
	ONEngine::Asset::AssetCollection* pAssetCollection_;
	ONEngine::EntityComponentSystem* pEntityComponentSystem_;
	EditorManager* pEditorManager_;
	ONEngine::SceneManager* pSceneManager_;
	ONEngine::Window* pImGuiWindow_;
	ONEngine::Window* pDebugGameWindow_;

	std::unique_ptr<EditorViewCollection> imGuiWindowCollection_ = nullptr;

	std::unordered_map<std::string, ImGuiSceneImageInfo> sceneImageInfos_ = {}; ///< imguiのシーンイメージ情報

public:
	/// ===================================================
	/// public : accessors
	/// ===================================================

	/// @brief imgui windowを設定する
	/// @param window Window
	void SetImGuiWindow(ONEngine::Window* window);

	/// @brief game debug windowを取得する
	/// @return　Window
	ONEngine::Window* GetDebugGameWindow() const;

	/// @brief ImageInfoを取得する
	/// @param name ImageInfoの名前
	/// @return 見つかったImageInfoのポインタ、見つからなかった場合はnullptr
	const ImGuiSceneImageInfo* GetSceneImageInfo(const std::string& name) const;

	/// @brief WindowManagerの取得
	ONEngine::WindowManager* GetWindowManager() const { return pWindowManager_; }

	/// @brief SceneManagerの取得
	ONEngine::SceneManager* GetSceneManager() const { return pSceneManager_; }

	/// @brief EntityComponentSystemの取得
	ONEngine::EntityComponentSystem* GetEntityComponentSystem() const { return pEntityComponentSystem_; }
};


} /// Editor
