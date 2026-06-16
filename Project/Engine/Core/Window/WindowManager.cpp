#include "WindowManager.h"

using namespace ONEngine;

/// external
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

/// engine
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Core/Utility/Tools/Assert.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


static WindowManager* gWindowManager = nullptr;

WindowManager* WindowManager::GetInstance() {
	return gWindowManager;
}

void ONEngine::InternalGetWindowSize(Vector2* size) {
	if(size && gWindowManager && gWindowManager->GetMainWindow()) {
		*size = gWindowManager->GetMainWindow()->GetWindowSize();
	}
}

LRESULT WindowManager::MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifdef DEBUG_MODE
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif // DEBUG_MODE

	switch (msg) {
	case WM_CLOSE:
		if (gWindowManager) {
			gWindowManager->SetCloseRequested(true);
		}
		return 0;
	case WM_DESTROY: /// window破棄
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT WindowManager::SubWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifdef DEBUG_MODE
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif // DEBUG_MODE

	switch (msg) {
	case WM_CLOSE:
	case WM_DESTROY: /// window破棄
		DestroyWindow(hwnd);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}



WindowManager::WindowManager(DxManager* dxm)
	: pDxManager_(dxm) {}

WindowManager::~WindowManager() = default;


void WindowManager::Initialize() {
	/// COM初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	isProcessEnd_ = false;
	gWindowManager = this;
}

void WindowManager::Finalize() {
	windows_.clear();
	/// COM終了
	CoUninitialize();
}

void WindowManager::Update() {

	/// windowの更新
	for (auto itr = windows_.begin(); itr != windows_.end();) {
		if (!(*itr)->IsOpenWindow() && (*itr).get() != pMainWindow_) {
			itr = windows_.erase(itr);
		} else {
			++itr;
		}
	}

	/// main windowの更新
	UpdateMainWindow();

	/// sub windowの更新
	for (auto& window : windows_) {
		if (window.get() == pMainWindow_) {
			continue;
		}

		while (PeekMessage(&window->msg_, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&window->msg_);
			DispatchMessage(&window->msg_);
		}

		/// 終了メッセージ
		if (window->msg_.message == WM_QUIT) {
			window->processMessage_ = true;
			continue;
		}

		window->processMessage_ = false;
	}

}

void WindowManager::MainWindowPreDraw() {
	GetMainWindow()->PreDraw();
}

void WindowManager::MainWindowPostDraw() {
	GetMainWindow()->PostDraw();
}

void WindowManager::PreDrawAll() {
	for (auto& window : windows_) {
		window->PreDraw();
	}
}

void WindowManager::PostDrawAll() {
	for (auto& window : windows_) {
		window->PostDraw();
	}
}

void WindowManager::PresentAll() {
	for (auto& window : windows_) {
		window->Present();
	}
}



Window* WindowManager::GenerateWindow(const std::wstring& windowName, const Vector2& windowSize, WindowType windowType, UINT windowStyle) {
	std::unique_ptr<Window> newWindow = std::make_unique<Window>();

	/// game windowを作成して表示する
	CreateGameWindow(windowName.c_str(), windowSize, windowStyle, newWindow.get(), windowType);
	newWindow->Initialize(windowName, windowSize, pDxManager_);

	/// returnする用のpointer	
	Window* resultPtr = newWindow.get();

	windows_.push_back(std::move(newWindow));
	if (windowType == WindowType::Main) {
		pMainWindow_ = resultPtr;
	}

	return resultPtr;
}

void WindowManager::CreateGameWindow(const wchar_t* title, const Vector2& size, UINT windowStyle, Window* windowPtr, WindowType windowType) {

	timeBeginPeriod(1);

	windowPtr->windowClass_ = {};
	windowPtr->windowStyle_ = windowStyle;

	/// windowの設定
	if (windowType == WindowType::Main) {
		windowPtr->windowClass_.lpfnWndProc = MainWindowProc;
	} else {
		windowPtr->windowClass_.lpfnWndProc = SubWindowProc;
	}

	windowPtr->windowClass_.lpszClassName = title;
	windowPtr->windowClass_.hInstance = GetModuleHandle(nullptr);
	windowPtr->windowClass_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&windowPtr->windowClass_);

	windowPtr->wrc_ = { 0, 0, static_cast<int>(size.x), static_cast<int>(size.y) };
	AdjustWindowRect(&windowPtr->wrc_, WS_OVERLAPPEDWINDOW, false);

	windowPtr->hwnd_ = CreateWindowEx(
		0,
		windowPtr->windowClass_.lpszClassName,
		title,
		windowPtr->windowStyle_,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowPtr->wrc_.right - windowPtr->wrc_.left,
		windowPtr->wrc_.bottom - windowPtr->wrc_.top,
		nullptr,
		nullptr,
		windowPtr->windowClass_.hInstance,
		nullptr
	);

	/// windowの生成できたかチェック
	if (!windowPtr->hwnd_) {
		DWORD err = GetLastError();
		Console::LogError("CreateWindowEx failed. Error code: " + std::to_string(err));
		Assert(false, "Failed CreateWindowEx");
	}


	/// window表示
	ShowWindow(windowPtr->hwnd_, SW_SHOW);
}

void WindowManager::UpdateMainWindow() {
	pMainWindow_->Update();

	while (PeekMessage(&pMainWindow_->msg_, nullptr, 0, 0, PM_REMOVE)) {
		if (pMainWindow_->msg_.message == WM_QUIT) {
			break;
		}

		TranslateMessage(&pMainWindow_->msg_);
		DispatchMessage(&pMainWindow_->msg_);
	}

	/// 終了メッセージ
	if (pMainWindow_->msg_.message == WM_QUIT) {
		isProcessEnd_ = true;
		pMainWindow_->processMessage_ = true;
		return;
	}

	isProcessEnd_ = false;
	pMainWindow_->processMessage_ = false;
}

Window* WindowManager::GetMainWindow() const {
	return pMainWindow_;
}

Window* WindowManager::GetActiveWindow() const {

	HWND activeWindow = GetForegroundWindow();
	for (auto& window : windows_) {
		if (window->GetHwnd() == activeWindow) {
			return window.get();
		}
	}

	return GetMainWindow();
}

bool WindowManager::IsCloseRequested() const {
	return closeRequested_;
}

void WindowManager::SetCloseRequested(bool isCloseRequested) {
	closeRequested_ = isCloseRequested;
}
