using System;

public class ParticleEmitCountTestScript : MonoScript {
    private ParticleSystem2D particleSystem;
    private int frameCount = 0;
    private bool emitted = false;

    public override void Initialize() {
        particleSystem = entity.GetComponent<ParticleSystem2D>();
        Console.WriteLine("[ParticleEmitCountTestScript] Initialized");
    }

    public override void Update() {
        frameCount++;

        if (frameCount == 5 && particleSystem != null && !emitted) {
            particleSystem.Emit(1);
            emitted = true;
            Console.WriteLine("[ParticleEmitCountTestScript] Emitted 1 particle");
        }
    }
}
