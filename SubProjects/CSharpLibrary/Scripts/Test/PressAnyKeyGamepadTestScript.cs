using System;
using ONEngine;

public class PressAnyKeyGamepadTestScript : MonoScript {
	private int frameCount_ = 0;

	public override void Update() {
		frameCount_++;

		if (frameCount_ == 5) {
			// Frame 5: Keyboard A key down
			if (!Input.PressAnykey()) {
				throw new Exception($"[Test Error] Frame 5: PressAnykey expected true, got false.");
			}
			if (Input.PressAnyGamepad()) {
				throw new Exception($"[Test Error] Frame 5: PressAnyGamepad expected false, got true.");
			}
			Console.WriteLine("[PressAnyKeyGamepadTest] Frame 5 Keyboard check passed.");
		}
		else if (frameCount_ == 15) {
			// Frame 15: Gamepad Button A down
			if (Input.PressAnykey()) {
				throw new Exception($"[Test Error] Frame 15: PressAnykey expected false, got true.");
			}
			if (!Input.PressAnyGamepad()) {
				throw new Exception($"[Test Error] Frame 15: PressAnyGamepad expected true, got false.");
			}
			Console.WriteLine("[PressAnyKeyGamepadTest] Frame 15 Gamepad button check passed.");
		}
		else if (frameCount_ == 25) {
			// Frame 25: Gamepad Stick input
			if (Input.PressAnykey()) {
				throw new Exception($"[Test Error] Frame 25: PressAnykey expected false, got true.");
			}
			if (!Input.PressAnyGamepad()) {
				throw new Exception($"[Test Error] Frame 25: PressAnyGamepad expected true, got false (stick input).");
			}
			Console.WriteLine("[PressAnyKeyGamepadTest] Frame 25 Gamepad stick check passed.");
		}
		else if (frameCount_ == 35) {
			// Frame 35: Gamepad Trigger (LT) input
			if (Input.PressAnykey()) {
				throw new Exception($"[Test Error] Frame 35: PressAnykey expected false, got true.");
			}
			if (!Input.PressAnyGamepad()) {
				throw new Exception($"[Test Error] Frame 35: PressAnyGamepad expected true, got false (trigger input).");
			}
			Console.WriteLine("[PressAnyKeyGamepadTest] Frame 35 Gamepad trigger check passed.");
		}
		else if (frameCount_ == 45) {
			// Frame 45: No input
			if (Input.PressAnykey()) {
				throw new Exception($"[Test Error] Frame 45: PressAnykey expected false, got true.");
			}
			if (Input.PressAnyGamepad()) {
				throw new Exception($"[Test Error] Frame 45: PressAnyGamepad expected false, got true.");
			}
			Console.WriteLine("[PressAnyKeyGamepadTest] Frame 45 No input check passed.");
		}
	}
}
