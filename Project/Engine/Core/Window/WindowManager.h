#pragma once

/// std
#include <memory>
#include <vector>

/// engine
#include "Window.h"

/// ///////////////////////////////////////////////////
/// windowの管理クラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

class WindowManager final {
public:
	/// ===================================================
	/// public : enum
	/// ===================================================

	enum class WindowType {
		Main,
		Sub,
	};


public:
	/// ===================================================
	/// public : static method
	/// ===================================================

	/// @brief main windowのwindowプロシージャ
	/// @param hwnd 
	/// @param msg 
	/// @param wparam 
	/// @param lparam 
	/// @return 
	static LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	/// @brief sub windowのwindowプロシージャ
	/// @param hwnd 
	/// @param msg 
	/// @param wparam 
	/// @param lparam 
	/// @return 
	static LRESULT CALLBACK SubWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


public:
	/// ===================================================
	/// public : method
	/// ===================================================

	WindowManager(DxManager* dxm);
	~WindowManager();

	/// @brief Instanceの取得
	static WindowManager* GetInstance();

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	void Update();

	/// @brief 終了処理
	void Finalize();

	/// @brief main windowの描画前処理
	void MainWindowPreDraw();

	/// @brief main windowの描画後処理
	void MainWindowPostDraw();

	/// @brief windows_の描画前処理
	void PreDrawAll();

	/// @brief windows_の描画後処理
	void PostDrawAll();

	/// @brief windows_の描画
	void PresentAll();

	/// @brief 新しいwindowを生成
	/// @param windowName windowの名前
	/// @param windowSize windowのサイズ
	/// @param windowType windowの種類
	/// @return 生成したwindowのポインタ
	Window* GenerateWindow(const std::wstring& windowName, const Vector2& windowSize, WindowType windowType = WindowType::Sub, UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME));

	/// @brief game windowを非表示
	/// @param windowPtr 隠したいwindowのポインタ
	void HideGameWindow(Window* windowPtr) { ShowWindow(windowPtr->GetHwnd(), SW_HIDE); }

	/// @brief game windowを表示
	/// @param windowPtr 表示したいwindowのポインタ
	void ShowGameWindow(Window* windowPtr) { ShowWindow(windowPtr->GetHwnd(), SW_SHOW); }

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	/// @brief  game windowの生成
	/// @param title windowのタイトル
	/// @param size  windowのサイズ
	/// @param windowStyle windowのスタイル
	/// @param windowPtr windowのポインタ
	/// @param windowType windowの種類
	void CreateGameWindow(const wchar_t* title, const Vector2& size, UINT windowStyle, Window* windowPtr, WindowType windowType);

	/// @brief main windowの更新
	void UpdateMainWindow();


private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	///  ----- other class ----- ///
	class DxManager*                     pDxManager_ = nullptr;

	std::vector<std::unique_ptr<Window>> windows_;
	Window*                              pMainWindow_ = nullptr;

	bool                                 isProcessEnd_;
	bool                                 closeRequested_ = false;


public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	/// @brief MainWindowの取得
	Window* GetMainWindow() const;

	/// @brief 現在のアクティブなWindowを取得
	Window* GetActiveWindow() const;

	/// @brief 終了リクエストが来ているか
	bool IsCloseRequested() const;

	/// @brief 終了リクエストを設定
	void SetCloseRequested(bool isCloseRequested);

private:
	/// ===================================================
	/// private : copy delete
	/// ===================================================

	WindowManager(const WindowManager&)            = delete;
	WindowManager(WindowManager&&)                 = delete;
	WindowManager& operator=(const WindowManager&) = delete;
	WindowManager& operator=(WindowManager&&)      = delete;
};

void InternalGetWindowSize(Vector2* size);

} /// ONEngine
