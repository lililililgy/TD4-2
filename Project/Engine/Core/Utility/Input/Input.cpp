#include "Input.h"

using namespace ONEngine;

/// engine
#include "InputSystem.h"

namespace {
	std::unique_ptr<InputSystem> gInputSystem_;
} /// namespace

void Input::Initialize(WindowManager* windowManager, Editor::ImGuiManager* imguiManager) {
	gInputSystem_ = std::make_unique<InputSystem>();
	gInputSystem_->Initialize(windowManager, imguiManager);
}

void Input::Update() {
	gInputSystem_->Update();
}

void Input::Finalize() {
	gInputSystem_.reset();
}

bool Input::PressKey(int key) {
	return gInputSystem_->keyboard_->keys_[key];
}

bool Input::TriggerKey(int key) {
	return gInputSystem_->keyboard_->keys_[key] && !gInputSystem_->keyboard_->preKeys_[key];
}

bool Input::ReleaseKey(int key) {
	return !gInputSystem_->keyboard_->keys_[key] && gInputSystem_->keyboard_->preKeys_[key];
}

bool Input::PressMouse(int button) {
	return gInputSystem_->mouse_->state_.rgbButtons[button];
}

bool Input::TriggerMouse(int button) {
	return gInputSystem_->mouse_->state_.rgbButtons[button] && !gInputSystem_->mouse_->preState_.rgbButtons[button];
}

bool Input::ReleaseMouse(int button) {
	return !gInputSystem_->mouse_->state_.rgbButtons[button] && gInputSystem_->mouse_->preState_.rgbButtons[button];
}

bool Input::PressGamepad(int button) {
	return (gInputSystem_->gamepad_->state_.Gamepad.wButtons & static_cast<WORD>(button)) != 0;
}

bool Input::TriggerGamepad(int button) {
	return PressGamepad(button) && (gInputSystem_->gamepad_->prevState_.Gamepad.wButtons & static_cast<WORD>(button)) == 0;
}

bool Input::ReleaseGamepad(int button) {
	return !PressGamepad(button) && (gInputSystem_->gamepad_->prevState_.Gamepad.wButtons & static_cast<WORD>(button)) != 0;
}

Vector2 Input::GetGamepadLeftThumb() {
	Gamepad* gamepad = gInputSystem_->gamepad_.get();
	if (std::abs(gamepad->state_.Gamepad.sThumbLX) != gamepad->stickDeadZone_
		|| std::abs(gamepad->state_.Gamepad.sThumbLY) != gamepad->stickDeadZone_) {
		return Vector2(
			static_cast<float>(gamepad->state_.Gamepad.sThumbLX) / XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
			static_cast<float>(gamepad->state_.Gamepad.sThumbLY) / XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
		);
	}

	return Vector2::Zero;
}

Vector2 Input::GetGamepadRightThumb() {
	Gamepad* gamepad = gInputSystem_->gamepad_.get();
	if (std::abs(gamepad->state_.Gamepad.sThumbRX) != gamepad->stickDeadZone_
		|| std::abs(gamepad->state_.Gamepad.sThumbRY) != gamepad->stickDeadZone_) {
		return Vector2(
			static_cast<float>(gamepad->state_.Gamepad.sThumbRX) / XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
			static_cast<float>(gamepad->state_.Gamepad.sThumbRY) / XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
		);
	}

	return Vector2::Zero;
}

float Input::GetMouseWheel() {
	return gInputSystem_->mouse_->wheel_;
}

const Vector2& Input::GetMousePosition() {
	return gInputSystem_->mouse_->position_;
}

const Vector2& Input::GetMouseVelocity() {
	return gInputSystem_->mouse_->velocity_;
}

const Vector2& Input::GetImGuiImageMousePosNormalized(const std::string& imageName) {
	return gInputSystem_->mouse_->GetImGuiImageMousePosNormalized(imageName);
}

const Vector2& Input::GetImGuiImagePos(const std::string& imageName) {
	return gInputSystem_->mouse_->GetImGuiImagePos(imageName);
}

const Vector2& Input::GetImGuiImageSize(const std::string& imageName) {
	return gInputSystem_->mouse_->GetImGuiImageSize(imageName);
}
