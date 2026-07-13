public class KingJellyfishRotatingBeamSettings : MonoScript
{
    [SerializeField] public float selectionWeight = 1.0f;
    [SerializeField] public string laserPrefabName = "JellyfishLaser";
    [SerializeField] public int laserCount = 4;
    [SerializeField] public float tellDuration = 1.0f;
    [SerializeField] public float duration = 4.0f;
    [SerializeField] public float recoveryDuration = 0.5f;
    [SerializeField] public float rotationSpeed = 0.7f;
    [SerializeField] public float length = 1200.0f;
    [SerializeField] public float width = 60.0f;
    [SerializeField] public float damage = 10.0f;
}
