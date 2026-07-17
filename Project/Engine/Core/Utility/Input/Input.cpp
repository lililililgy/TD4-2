#include "Input.h"

using namespace ONEngine;

/// std
#include <fstream>
#include <vector>
#include <cstring>
#include <nlohmann/json.hpp>

/// engine
#include "InputSystem.h"
#include "Engine/Core/Config/EngineConfig.h"

namespace {
	std::unique_ptr<InputSystem> gInputSystem_;
} /// namespace

namespace ONEngine::MockInput {
	struct KeyState {
		int key;
		bool isDown;
	};
	
	struct MouseState {
		bool buttons[4] = {false, false, false, false};
		Vector2 position = Vector2::Zero;
		Vector2 velocity = Vector2::Zero;
		float wheel = 0.0f;
	};

	struct GamepadState {
		WORD buttons = 0;
		Vector2 leftThumb = Vector2::Zero;
		Vector2 rightThumb = Vector2::Zero;
	};

	struct FrameInput {
		int frame;
		std::vector<KeyState> keys;
		bool hasMouse = false;
		MouseState mouse;
		bool hasGamepad = false;
		GamepadState gamepad;
	};

	std::vector<FrameInput> testInputs;
	int currentTestFrame = 0;
	bool isInitialized = false;
	
	bool mockKeys[256] = {false};
	bool mockPreKeys[256] = {false};
	MouseState mockMouse;
	MouseState mockPreMouse;
	GamepadState mockGamepad;
	GamepadState mockPreGamepad;
	float mockLeftVibration = 0.0f;
	float mockRightVibration = 0.0f;

	void Initialize() {
		if (isInitialized) return;
		isInitialized = true;
		
		currentTestFrame = 0;
		std::memset(mockKeys, 0, sizeof(mockKeys));
		std::memset(mockPreKeys, 0, sizeof(mockPreKeys));
		mockLeftVibration = 0.0f;
		mockRightVibration = 0.0f;
		testInputs.clear();
		
		if (EngineConfig::testInputPath.empty()) return;
		
		std::ifstream ifs(EngineConfig::testInputPath);
		if (!ifs.is_open()) return;
		
		nlohmann::json j;
		try {
			ifs >> j;
			if (j.contains("inputs") && j["inputs"].is_array()) {
				for (auto& item : j["inputs"]) {
					FrameInput fi;
					fi.frame = item.value("frame", 0);
					
					if (item.contains("keys") && item["keys"].is_array()) {
						for (auto& k : item["keys"]) {
							KeyState ks;
							if (k["key"].is_number()) {
								ks.key = k["key"].get<int>();
							} else {
								std::string keyStr = k["key"].get<std::string>();
								if (keyStr == "SPACE") ks.key = 0x39;
								else if (keyStr == "P") ks.key = 0x19;
								else if (keyStr == "W") ks.key = 0x11;
								else if (keyStr == "A") ks.key = 0x1E;
								else if (keyStr == "S") ks.key = 0x1F;
								else if (keyStr == "D") ks.key = 0x20;
							}
							ks.isDown = k.value("down", true);
							fi.keys.push_back(ks);
						}
					}
					
					if (item.contains("mouse")) {
						fi.hasMouse = true;
						auto& m = item["mouse"];
						if (m.contains("buttons") && m["buttons"].is_array()) {
							for (int b = 0; b < 4 && b < m["buttons"].size(); ++b) {
								fi.mouse.buttons[b] = m["buttons"][b].get<bool>();
							}
						}
						if (m.contains("position") && m["position"].is_array() && m["position"].size() >= 2) {
							fi.mouse.position.x = m["position"][0].get<float>();
							fi.mouse.position.y = m["position"][1].get<float>();
						}
						if (m.contains("velocity") && m["velocity"].is_array() && m["velocity"].size() >= 2) {
							fi.mouse.velocity.x = m["velocity"][0].get<float>();
							fi.mouse.velocity.y = m["velocity"][1].get<float>();
						}
						if (m.contains("wheel")) {
							fi.mouse.wheel = m["wheel"].get<float>();
						}
					}
					
					if (item.contains("gamepad")) {
						fi.hasGamepad = true;
						auto& gp = item["gamepad"];
						fi.gamepad.buttons = static_cast<WORD>(gp.value("buttons", 0));
						if (gp.contains("leftStick") && gp["leftStick"].is_array() && gp["leftStick"].size() >= 2) {
							fi.gamepad.leftThumb.x = gp["leftStick"][0].get<float>();
							fi.gamepad.leftThumb.y = gp["leftStick"][1].get<float>();
						}
						if (gp.contains("rightStick") && gp["rightStick"].is_array() && gp["rightStick"].size() >= 2) {
							fi.gamepad.rightThumb.x = gp["rightStick"][0].get<float>();
							fi.gamepad.rightThumb.y = gp["rightStick"][1].get<float>();
						}
					}
					
					testInputs.push_back(fi);
				}
			}
		} catch (...) {
			// Ignore parse error
		}
	}

	void Update() {
		if (!isInitialized) {
			Initialize();
		}
		
		std::memcpy(mockPreKeys, mockKeys, sizeof(mockKeys));
		mockPreMouse = mockMouse;
		mockPreGamepad = mockGamepad;
		
		for (const auto& fi : testInputs) {
			if (fi.frame == currentTestFrame) {
				for (const auto& ks : fi.keys) {
					mockKeys[ks.key] = ks.isDown;
				}
				if (fi.hasMouse) {
					mockMouse = fi.mouse;
				}
				if (fi.hasGamepad) {
					mockGamepad = fi.gamepad;
				}
			}
		}
		
		currentTestFrame++;
	}
}

void Input::Initialize(WindowManager* windowManager, Editor::ImGuiManager* imguiManager) {
	gInputSystem_ = std::make_unique<InputSystem>();
	gInputSystem_->Initialize(windowManager, imguiManager);
	MockInput::isInitialized = false;
}

void Input::Update() {
	gInputSystem_->Update();
	if (EngineConfig::isTestMode) {
		MockInput::Update();
	}
}

void Input::Finalize() {
	gInputSystem_.reset();
}

bool Input::PressKey(int key) {
	if (EngineConfig::isTestMode) {
		return MockInput::mockKeys[key];
	}
	return gInputSystem_->keyboard_->keys_[key];
}

bool Input::TriggerKey(int key) {
	if (EngineConfig::isTestMode) {
		return MockInput::mockKeys[key] && !MockInput::mockPreKeys[key];
	}
	return gInputSystem_->keyboard_->keys_[key] && !gInputSystem_->keyboard_->preKeys_[key];
}

bool Input::ReleaseKey(int key) {
	if (EngineConfig::isTestMode) {
		return !MockInput::mockKeys[key] && MockInput::mockPreKeys[key];
	}
	return !gInputSystem_->keyboard_->keys_[key] && gInputSystem_->keyboard_->preKeys_[key];
}

bool Input::PressMouse(int button) {
	if (EngineConfig::isTestMode) {
		return MockInput::mockMouse.buttons[button];
	}
	return gInputSystem_->mouse_->state_.rgbButtons[button];
}

bool Input::TriggerMouse(int button) {
	if (EngineConfig::isTestMode) {
		return MockInput::mockMouse.buttons[button] && !MockInput::mockPreMouse.buttons[button];
	}
	return gInputSystem_->mouse_->state_.rgbButtons[button] && !gInputSystem_->mouse_->preState_.rgbButtons[button];
}

bool Input::ReleaseMouse(int button) {
	if (EngineConfig::isTestMode) {
		return !MockInput::mockMouse.buttons[button] && MockInput::mockPreMouse.buttons[button];
	}
	return !gInputSystem_->mouse_->state_.rgbButtons[button] && gInputSystem_->mouse_->preState_.rgbButtons[button];
}

bool Input::PressGamepad(int button) {
	if (EngineConfig::isTestMode) {
		return (MockInput::mockGamepad.buttons & static_cast<WORD>(button)) != 0;
	}
	return (gInputSystem_->gamepad_->state_.Gamepad.wButtons & static_cast<WORD>(button)) != 0;
}

bool Input::TriggerGamepad(int button) {
	if (EngineConfig::isTestMode) {
		return PressGamepad(button) && (MockInput::mockPreGamepad.buttons & static_cast<WORD>(button)) == 0;
	}
	return PressGamepad(button) && (gInputSystem_->gamepad_->prevState_.Gamepad.wButtons & static_cast<WORD>(button)) == 0;
}

bool Input::ReleaseGamepad(int button) {
	if (EngineConfig::isTestMode) {
		return !PressGamepad(button) && (MockInput::mockPreGamepad.buttons & static_cast<WORD>(button)) != 0;
	}
	return !PressGamepad(button) && (gInputSystem_->gamepad_->prevState_.Gamepad.wButtons & static_cast<WORD>(button)) != 0;
}

Vector2 Input::GetGamepadLeftThumb() {
	if (EngineConfig::isTestMode) {
		return MockInput::mockGamepad.leftThumb;
	}
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
	if (EngineConfig::isTestMode) {
		return MockInput::mockGamepad.rightThumb;
	}
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
	if (EngineConfig::isTestMode) {
		return MockInput::mockMouse.wheel;
	}
	return gInputSystem_->mouse_->wheel_;
}

const Vector2& Input::GetMousePosition() {
	if (EngineConfig::isTestMode) {
		return MockInput::mockMouse.position;
	}
	return gInputSystem_->mouse_->position_;
}

const Vector2& Input::GetMouseVelocity() {
	if (EngineConfig::isTestMode) {
		return MockInput::mockMouse.velocity;
	}
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

void Input::SetGamepadVibration(float leftMotorSpeed, float rightMotorSpeed) {
	if (EngineConfig::isTestMode) {
		MockInput::mockLeftVibration = std::clamp(leftMotorSpeed, 0.0f, 1.0f);
		MockInput::mockRightVibration = std::clamp(rightMotorSpeed, 0.0f, 1.0f);
		return;
	}
	if (gInputSystem_ && gInputSystem_->gamepad_) {
		gInputSystem_->gamepad_->SetVibration(leftMotorSpeed, rightMotorSpeed);
	}
}

void Input::GetGamepadVibration(float& left, float& right) {
	if (EngineConfig::isTestMode) {
		left = MockInput::mockLeftVibration;
		right = MockInput::mockRightVibration;
		return;
	}
	if (gInputSystem_ && gInputSystem_->gamepad_) {
		left = gInputSystem_->gamepad_->GetLeftVibration();
		right = gInputSystem_->gamepad_->GetRightVibration();
	} else {
		left = 0.0f;
		right = 0.0f;
	}
}

