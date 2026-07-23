public class KingJellyfishChargeAttackSettings : MonoScript
{
    [SerializeField] public float selectionWeight = 1.0f;
    [SerializeField] public float tellDuration = 0.8f;
    [SerializeField] public float moveDuration = 0.6f;
    [SerializeField] public float speed = 500.0f;
    [SerializeField] public float recoveryDuration = 0.8f;
    [SerializeField] public float inertiaRate = 0.5f;
    [SerializeField] public float passThroughDistance = 300.0f;
    [SerializeField] public float damage = 20.0f;
    [SerializeField] public string damageFieldPrefabName = "JellyfishChargeDamageField";
    [SerializeField] public float damageFieldWidth = 180.0f;
    [SerializeField] public float damageFieldDuration = 0.6f;
    [SerializeField] public string sparkParticlePrefabName = "sparkParticle";
    [SerializeField] public int sparkParticleEmitCount = 30;
    [SerializeField] public string effect01EntityName = "Jellyfish_ChargeAttackEffect01";
    [SerializeField] public int effect01EmitCount = 1;
    [SerializeField] public float effect01EmitInterval = 0.05f;
}
