public class KingJellyfishOmnidirectionalBeamSettings : MonoScript
{
    [SerializeField] public float selectionWeight = 1.0f;
    [SerializeField] public string laserPrefabName = "JellyfishLaser";
    [SerializeField] public float tellDuration = 0.8f;
    [SerializeField] public float fireDuration = 0.4f;
    [SerializeField] public float recoveryDuration = 0.6f;
    [SerializeField] public float length = 1200.0f;
    [SerializeField] public float width = 80.0f;
    [SerializeField] public float damage = 15.0f;
    [SerializeField] public int laserCount = 8;
    [SerializeField] public string fireSePath = "./Assets/Sounds/se/boss/KingJellyfish_Laser.mp3";
    [SerializeField] public float fireSeVolume = 1.0f;
    [SerializeField] public float fireSePitch = 1.0f;
}
