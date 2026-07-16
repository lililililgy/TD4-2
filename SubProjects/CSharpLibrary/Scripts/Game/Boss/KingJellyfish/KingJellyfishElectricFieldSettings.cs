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
}
