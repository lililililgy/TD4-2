public class KingJellyfishElectricFieldSettings : MonoScript
{
    [SerializeField] public float selectionWeight = 1.0f;
    [SerializeField] public string fieldPrefabName = "JellyfishElectricField";
    [SerializeField] public int fieldCount = 4;
    [SerializeField] public float tellDuration = 1.0f;
    [SerializeField] public float spawnInterval = 0.2f;
    [SerializeField] public float activeDuration = 1.2f;
    [SerializeField] public float recoveryDuration = 0.5f;
    [SerializeField] public float radius = 140.0f;
    [SerializeField] public float spreadRadius = 300.0f;
    [SerializeField] public float damage = 12.0f;
    [SerializeField] public string thunderboltParticlePrefabName = "thunderboltParticle";
    [SerializeField] public int thunderboltParticleEmitCount = 1;
    [SerializeField] public float thunderboltParticleDuration = 1.0f;
    [SerializeField] public string deploySePath = "./Assets/Sounds/se/boss/KingJellyfish_elecField.mp3";
    [SerializeField] public float deploySeVolume = 1.0f;
    [SerializeField] public float deploySePitch = 1.0f;
}
