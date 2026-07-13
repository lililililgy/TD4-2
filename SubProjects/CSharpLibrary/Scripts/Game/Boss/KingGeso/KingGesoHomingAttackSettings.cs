public class KingGesoHomingAttackSettings : MonoScript
{
    [SerializeField] public string projectilePrefabName = "KingGesoHomingBullet";
    [SerializeField] public int projectileCount = 3;
    [SerializeField] public float launchInterval = 0.35f;
    [SerializeField] public float projectileSpeed = 120.0f;
    [SerializeField] public float turnSpeed = 2.0f;
    [SerializeField] public float projectileLifeTime = 8.0f;
    [SerializeField] public float projectileDamage = 1.0f;
    [SerializeField] public Vector2 spawnOffset = Vector2.zero;
}
