using System;
using System.Collections.Generic;

public class RadialBlurTestScript : MonoScript {
	private int frameCount = 0;
	private bool applied = false;

	public override void Initialize() {
		try {
			System.Console.WriteLine("[RadialBlurTestScript] Loading GameScene...");
			SceneManager.Add("GameScene");
		} catch (Exception ex) {
			System.Console.WriteLine("[RadialBlurTestScript] Exception in Initialize: " + ex.ToString());
		}
	}

	public override void Update() {
		try {
			frameCount++;

			if (this.ecsGroup == null) {
				System.Console.WriteLine("[RadialBlurTestScript] ecsGroup is null!");
				return;
			}

			if (!applied) {
				var playerEntity = this.ecsGroup.FindEntity("Player");
				if (playerEntity != null) {
					var spriteRenderer = playerEntity.GetComponent<SpriteRenderer>();
					if (spriteRenderer != null) {
						System.Console.WriteLine("[RadialBlurTestScript] Found Player, applying Radial Blur parameters!");
						spriteRenderer.postEffectFlags |= (uint)PostEffectFlags.RadialBlur;
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
						if ((spriteRenderer.postEffectFlags & (uint)PostEffectFlags.RadialBlur) == 0) {
							System.Console.WriteLine($"[RadialBlurTestScript] Assertion failed: flags={spriteRenderer.postEffectFlags}");
							throw new Exception("Sync values mismatch!");
						}
					}
				}
			}

			if (frameCount >= 50) {
				System.Console.WriteLine("[RadialBlurTestScript] Test passed.");
			}
		} catch (Exception ex) {
			System.Console.WriteLine("[RadialBlurTestScript] Exception in Update: " + ex.ToString());
		}
	}
}
