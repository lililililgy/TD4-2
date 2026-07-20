#pragma once

/// engine
#include "Keyboard.h"
#include "Mouse.h"
#include "Gamepad.h"

namespace Editor {
	class ImGuiManager;
} /// Editor

/// //////////////////////////////////////////////////
/// 入力処理クラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

class Input final {
	friend class GameFramework;
	friend class MonoScriptEngine;

	static void Initialize(class WindowManager* windowManager, Editor::ImGuiManager* imguiManager);
	static void Update();
	static void Finalize();

public:

	/// ===================================================
	/// public : methods
	/// ===================================================

	/// ----- Keyboard ----- ///

	/// @brief キーが押されているか
	/// @param key DIK_*** 定数
	/// @return true: 押されている, false: 押されていない
	static bool PressKey(int key);

	/// @brief キーがトリガーされたか
	/// @param key DIK_*** 定数
	/// @return true: トリガーされた, false: トリガーされていない
	static bool TriggerKey(int key);

	/// @brief キーが離されたか
	/// @param key DIK_*** 定数
	/// @return true: 離された, false: 離されていない
	static bool ReleaseKey(int key);


	
	/// ----- mouse ----- ///

	/// @brief マウスボタンが押されているか
	/// @param button Mouse::*** 定数
	/// @return true: 押されている, false: 押されていない
	static bool PressMouse(int button);

	/// @brief マウスボタンがトリガーされたか
	/// @param button Mouse::*** 定数
	/// @return true: トリガーされた, false: トリガーされていない
	static bool TriggerMouse(int button);

	/// @brief マウスボタンが離されたか
	/// @param button Mouse::*** 定数
	/// @return true: 離された, false: 離されていない
	static bool ReleaseMouse(int button);


	/// @brief マウスのホイールの回転量を取得
	/// @return 回転量 (上方向: 正の値, 下方向: 負の値)
	static float GetMouseWheel();

	/// @brief マウスの位置を取得
	static const Vector2& GetMousePosition();

	/// @brief マウスの移動量を取得
	static const Vector2& GetMouseVelocity();


	/// @brief ImGuiImage状でのマウス位置を正規化した座標で取得
	/// @param imageName ImGuiImageの名前
	/// @return 1280x720内でのマウス位置
	static const Vector2& GetImGuiImageMousePosNormalized(const std::string& imageName);

	/// @brief ImGuiImageの位置を取得
	/// @param imageName Imageの名前
	static const Vector2& GetImGuiImagePos(const std::string& imageName);

	/// @brief ImGuiImageのサイズを取得
	/// @param imageName Imageの名前
	static const Vector2& GetImGuiImageSize(const std::string& imageName);



	/// ----- gamepad ----- ///

	/// @brief Gamepadのボタンが押されているか
	/// @param button Gamepad::*** 定数
	/// @return true: 押されている, false: 押されていない
	static bool PressGamepad(int button);

	/// @brief Gamepadのボタンがトリガーされたか
	/// @param button Gamepad::*** 定数
	/// @return true: トリガーされた, false: トリガーされていない
	static bool TriggerGamepad(int button);

	/// @brief Gamepadのボタンが離されたか
	/// @param button Gamepad::*** 定数
	/// @return rue: 離された, false: 離されていない
	static bool ReleaseGamepad(int button);


	/// @brief Gamepadの左スティックの値を取得
	static Vector2 GetGamepadLeftThumb();

	/// @brief Gamepadの右スティックの値を取得
	static Vector2 GetGamepadRightThumb();

	/// @brief Gamepadの振動を設定する
	/// @param leftMotorSpeed 左モーターの速度 (0.0f - 1.0f)
	/// @param rightMotorSpeed 右モーターの速度 (0.0f - 1.0f)
	static void SetGamepadVibration(float leftMotorSpeed, float rightMotorSpeed);

	/// @brief Gamepadの振動強さを取得する（検証用）
	static void GetGamepadVibration(float& left, float& right);

	/// @brief Gamepadを指定時間振動させる
	/// @param leftMotorSpeed 左モーターの速度 (0.0f - 1.0f)
	/// @param rightMotorSpeed 右モーターの速度 (0.0f - 1.0f)
	/// @param duration 振動時間（秒）
	static void PlayGamepadVibration(float leftMotorSpeed, float rightMotorSpeed, float duration);

};


} /// ONEngine
