#include "Gamepad.h"
#include <algorithm>

using namespace ONEngine;

/// comment
#pragma comment(lib, "xinput.lib")

/// engine
#include "Engine/Core/Window/WindowManager.h"
#include "Engine/Core/Utility/Utility.h"

Gamepad::Gamepad() {}
Gamepad::~Gamepad() {}

void Gamepad::Initialize(IDirectInput8* directInput, WindowManager* windowManager) {

	HRESULT hr = directInput->CreateDevice(GUID_SysKeyboard, &gamepadDevice_, NULL);
	Assert(SUCCEEDED(hr), "Failed to create gamepad device");

	/// 入力データ形式のセット
	hr = gamepadDevice_->SetDataFormat(&c_dfDIKeyboard); ///< 標準形式
	Assert(SUCCEEDED(hr), "Failed to set data format for gamepad device");

	/*  /// 排他制御レベルのセット
		DISCL_FOREGROUND   : 画面が手前にある場合のみ入力を受け付ける
		DISCL_NONEXCLUSIVE : デバイスをこのアプリだけで占有しない
		DISCL_NOWINKEY     : Windowsキーを無効にする
	*/

	hr = gamepadDevice_->SetCooperativeLevel(
		windowManager->GetMainWindow()->GetHwnd(),
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	Assert(SUCCEEDED(hr), "Failed to set cooperative level for gamepad device");
}

void Gamepad::Update(Window* /*window*/) {

	/// 前フレームの状態を保存
	prevState_ = state_;

	/// 現在の状態を取得
	ZeroMemory(&state_, sizeof(XINPUT_STATE));
	XInputGetState(0, &state_);
}

void Gamepad::SetVibration(float leftMotorSpeed, float rightMotorSpeed) {
	leftVibration_ = std::clamp(leftMotorSpeed, 0.0f, 1.0f);
	rightVibration_ = std::clamp(rightMotorSpeed, 0.0f, 1.0f);

	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = static_cast<WORD>(leftVibration_ * 65535.0f);
	vibration.wRightMotorSpeed = static_cast<WORD>(rightVibration_ * 65535.0f);
	XInputSetState(0, &vibration);
}
