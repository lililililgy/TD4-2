using System;
using System.Collections.Generic;

public class BloomTestScript : MonoScript {
	private int frameCount = 0;
	private bool applied = false;

	public override void Initialize() {
		try {
			System.Console.WriteLine("[BloomTestScript] Loading GameScene...");
			SceneManager.Add("GameScene");
		} catch (Exception ex) {
			System.Console.WriteLine("[BloomTestScript] Exception in Initialize: " + ex.ToString());
		}
	}

	public override void Update() {
		try {
			frameCount++;

			if (this.ecsGroup == null) {
				System.Console.WriteLine("[BloomTestScript] ecsGroup is null!");
				return;
			}

			if (!applied) {
				var playerEntity = this.ecsGroup.FindEntity("Player");
				if (playerEntity != null) {
					var spriteRenderer = playerEntity.GetComponent<SpriteRenderer>();
					if (spriteRenderer != null) {
						System.Console.WriteLine("[BloomTestScript] Found Player, applying Bloom parameters!");
						spriteRenderer.postEffectFlags |= (uint)PostEffectFlags.Bloom;
						spriteRenderer.bloomIntensity = 2.5f;
						spriteRenderer.bloomThreshold = 0.5f;
						spriteRenderer.bloomRadius = 15.0f;
						applied = true;
					}
				}
			}

			if (applied && frameCount >= 20 && frameCount < 40) {
				var playerEntity = this.ecsGroup.FindEntity("Player");
				if (playerEntity != null) {
					var spriteRenderer = playerEntity.GetComponent<SpriteRenderer>();
					if (spriteRenderer != null) {
						spriteRenderer.SyncFromNative(this.ecsGroup.groupName);
						if (Math.Abs(spriteRenderer.bloomIntensity - 2.5f) > 0.01f ||
							Math.Abs(spriteRenderer.bloomThreshold - 0.5f) > 0.01f ||
							Math.Abs(spriteRenderer.bloomRadius - 15.0f) > 0.01f) {
							System.Console.WriteLine($"[BloomTestScript] Assertion failed: intensity={spriteRenderer.bloomIntensity}, threshold={spriteRenderer.bloomThreshold}, radius={spriteRenderer.bloomRadius}");
							throw new Exception("Sync values mismatch!");
						}
					}
				}
			}

			if (frameCount >= 50) {
				System.Console.WriteLine("[BloomTestScript] Test passed.");
			}
		} catch (Exception ex) {
			System.Console.WriteLine("[BloomTestScript] Exception in Update: " + ex.ToString());
		}
	}
}
