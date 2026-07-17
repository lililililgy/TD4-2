using System;

public class RumbleTestScript : MonoScript {
	private int frameCount_ = 0;

	public override void Update() {
		frameCount_++;

		if (frameCount_ == 5) {
			// Aボタンが押されているはず（モック入力により）
			if (!Input.PressGamepad(Gamepad.A)) {
				throw new Exception("RumbleTest: Gamepad A should be pressed at frame 5.");
			}
			Input.SetGamepadVibration(0.5f, 0.8f);
			Debug.Log("RumbleTest: Set vibration (0.5, 0.8) at frame 5.");
		}
		else if (frameCount_ == 6) {
			// 振動が設定されていることを確認
			float left, right;
			Input.GetGamepadVibration(out left, out right);
			if (Math.Abs(left - 0.5f) > 0.001f || Math.Abs(right - 0.8f) > 0.001f) {
				throw new Exception($"RumbleTest: Vibration was not set correctly at frame 6. Left: {left}, Right: {right}");
			}
			Debug.Log($"RumbleTest: Verified vibration (Left: {left}, Right: {right}) at frame 6.");
			Input.SetGamepadVibration(0.0f, 0.0f);
			Debug.Log("RumbleTest: Cleared vibration at frame 6.");
		}
		else if (frameCount_ == 7) {
			// 振動が停止されたことを確認
			float left, right;
			Input.GetGamepadVibration(out left, out right);
			if (left != 0.0f || right != 0.0f) {
				throw new Exception($"RumbleTest: Vibration was not cleared at frame 7. Left: {left}, Right: {right}");
			}
			Debug.Log("RumbleTest: Verified vibration is cleared at frame 7. Test Passed!");
		}
	}
}
