using System;

public class CollisionCrashTestMonitor : MonoScript {
	private int frameCount = 0;

	public override void Initialize() {
		System.Console.WriteLine("[CollisionCrashTestMonitor] Initialized.");
	}

	public override void Update() {
		frameCount++;
		if (frameCount == 15) {
			System.Console.WriteLine("[CollisionCrashTestMonitor] 15 frames passed without crash. Test passed!");
		}
	}
}
