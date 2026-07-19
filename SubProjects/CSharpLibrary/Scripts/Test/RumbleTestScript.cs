using System;

public class RumbleTestScript : MonoScript
{
	private int frameCount_ = 0;

	public override void Update()
	{
		frameCount_++;

		if (frameCount_ == 5) {
			// Aボタンが押されているはず（モック入力により）
			if (!Input.PressGamepad(Gamepad.A)) {
				throw new Exception("RumbleTest: Gamepad A should be pressed at frame 5.");
			}
			// 1.0秒間の振動をセット
			Input.PlayGamepadVibration(0.5f, 0.8f, 1.0f);
			Debug.Log("RumbleTest: Play vibration (0.5, 0.8, 1.0s) at frame 5.");
		}
		else if (frameCount_ == 6) {
			// 振動が設定されていることを確認
			float left, right;
			Input.GetGamepadVibration(out left, out right);
			if (Math.Abs(left - 0.5f) > 0.001f || Math.Abs(right - 0.8f) > 0.001f) {
				throw new Exception($"RumbleTest: PlayGamepadVibration was not set correctly at frame 6. Left: {left}, Right: {right}");
			}
			Debug.Log($"RumbleTest: Verified vibration (Left: {left}, Right: {right}) at frame 6.");
			
			// 非常に短い時間（0.0001秒）で再設定（次フレームで確実に自動停止するように）
			Input.PlayGamepadVibration(0.3f, 0.4f, 0.0001f);
			Debug.Log("RumbleTest: Play vibration (0.3, 0.4, 0.0001s) at frame 6.");
		}
		else if (frameCount_ == 8) {
			// タイムアウトして停止していることを確認（フレーム8までに1フレーム以上経つので停止する）
			float left, right;
			Input.GetGamepadVibration(out left, out right);
			if (left != 0.0f || right != 0.0f) {
				throw new Exception($"RumbleTest: Vibration timer failed to clear vibration at frame 8. Left: {left}, Right: {right}");
			}
			Debug.Log("RumbleTest: Verified vibration is automatically cleared via timer at frame 8. Test Passed!");
		}
	}
}
