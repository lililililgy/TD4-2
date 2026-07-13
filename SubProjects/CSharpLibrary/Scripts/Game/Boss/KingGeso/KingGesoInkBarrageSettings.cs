public class KingGesoInkBarrageSettings : MonoScript
{
    [SerializeField] public float selectionWeight = 1.0f;
    [SerializeField] public string bulletPrefabName = "KingGesoInkBullet";
    [SerializeField] public int bulletCountPerWave = 10;
    [SerializeField] public int waveCount = 7;
    [SerializeField] public float waveInterval = 0.25f;
    [SerializeField] public float angleOffset = 0.14f;
    [SerializeField] public bool reverseHalfway = true;
    [SerializeField] public float bulletSpeed = 220.0f;
    [SerializeField] public float bulletLifeTime = 7.0f;
    [SerializeField] public float bulletDamage = 8.0f;
    [SerializeField] public float recoveryDuration = 0.6f;
    [SerializeField] public Vector2 spawnOffset = Vector2.zero;
}
