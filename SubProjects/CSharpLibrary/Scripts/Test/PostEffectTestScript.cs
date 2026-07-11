using System;
using System.Collections.Generic;

public class PostEffectTestScript : MonoScript {
	private int frameCount = 0;

	public override void Initialize() {
		System.Console.WriteLine("[PostEffectTestScript] Loading GameScene...");
		SceneManager.Add("GameScene");
	}

	public override void Update() {
		frameCount++;
		if (frameCount == 30) {
			System.Console.WriteLine("[PostEffectTestScript] Adding PauseScene...");
			SceneManager.Add("PauseScene");
		}
	}
}
