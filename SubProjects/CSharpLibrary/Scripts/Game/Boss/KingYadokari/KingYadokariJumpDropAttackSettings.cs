public class KingYadokariJumpDropAttackSettings : MonoScript {
    [SerializeField] public float selectionWeight = 1.0f;
    [SerializeField] public string cameraEntityName = "MainCamera";
    [SerializeField] public float screenHalfHeight = 720.0f;
    [SerializeField] public float offscreenOffset = 700.0f;
    [SerializeField] public float chargeDuration = 0.8f;
    [SerializeField] public float jumpSpeed = 1600.0f;
    [SerializeField] public float aimDuration = 1.2f;
    [SerializeField] public float fallSpeed = 2600.0f;
    [SerializeField] public float fallThroughDistance = 300.0f;
    [SerializeField] public float recoveryDuration = 0.8f;
    [SerializeField] public float damage = 3.0f;
}
