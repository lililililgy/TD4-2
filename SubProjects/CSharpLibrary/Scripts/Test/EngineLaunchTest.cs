using System;
using System.Collections.Generic;

public class EngineLaunchTest : MonoScript {
	private int frameCount = 0;

	public override void Initialize() {
		System.Console.WriteLine("[EngineLaunchTest] Game started. Auto-build and launch verified.");
	}

	public override void Update() {
		frameCount++;
		if (frameCount % 30 == 0) {
			System.Console.WriteLine("[EngineLaunchTest] Running frame: " + frameCount);
		}
	}
}
