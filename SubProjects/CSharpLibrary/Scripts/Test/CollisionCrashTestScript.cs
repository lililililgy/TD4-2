using System;

public class CollisionCrashTestScript : MonoScript {
	public override void Initialize() {
		System.Console.WriteLine("[CollisionCrashTestScript] Initialized.");
	}

	public override void OnCollisionEnter(Entity collision) {
		System.Console.WriteLine("[CollisionCrashTestScript] OnCollisionEnter triggered!");
		
		if (collision) {
			System.Console.WriteLine("[CollisionCrashTestScript] Destroying collision partner: " + collision.Id);
			collision.Destroy();
		}
		
		System.Console.WriteLine("[CollisionCrashTestScript] Destroying self: " + entity.Id);
		entity.Destroy();
	}
}
