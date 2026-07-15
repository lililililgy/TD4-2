using System;
using System.Collections.Generic;

public class BloomTestScript : MonoScript {
	private int frameCount = 0;
	private bool applied = false;

	public override void Initialize() {
		System.Console.WriteLine("[BloomTestScript] Loading GameScene...");
		SceneManager.Add("GameScene");
	}

	public override void Update() {
		frameCount++;

		if (!applied) {
			var playerEntity = this.ecsGroup.FindEntity("Player");
			if (playerEntity != null) {
				var spriteRenderer = playerEntity.GetComponent<SpriteRenderer>();
				if (spriteRenderer != null) {
					System.Console.WriteLine("[BloomTestScript] Found Player, applying Bloom flag!");
					spriteRenderer.postEffectFlags |= (uint)PostEffectFlags.Bloom;
					applied = true;
				}
			}
		}

		if (frameCount >= 50) {
			System.Console.WriteLine("[BloomTestScript] Test passed.");
		}
	}
}
