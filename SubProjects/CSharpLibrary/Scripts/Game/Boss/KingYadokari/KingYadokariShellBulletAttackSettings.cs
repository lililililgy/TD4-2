public class KingYadokariShellBulletAttackSettings : MonoScript {
    [SerializeField] public float selectionWeight = 1.0f;
    [SerializeField] public string prefabName = "YadokariShellBullet";
    [SerializeField] public Vector2 spawnOffset = new Vector2(0.0f, -180.0f);
    [SerializeField] public float tellDuration = 0.8f;
    [SerializeField] public float recoveryDuration = 0.8f;
    [SerializeField] public float speed = 420.0f;
    [SerializeField] public float reflectedSpeed = 720.0f;
    [SerializeField] public float lifeTime = 7.0f;
    [SerializeField] public float damage = 1.0f;
    [SerializeField] public float reflectionMinDamage = 1.0f;
}
