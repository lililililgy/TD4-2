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
}
