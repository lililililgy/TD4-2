public class KingJellyfishMoveSettings : MonoScript
{
    [SerializeField] public int actionLoopCount = 3;
    [SerializeField] public float moveDuration = 1.2f;
    [SerializeField] public float moveDistance = 300.0f;
    [SerializeField] public float moveSpeed = 250.0f;
    [SerializeField] public float moveArcHeight = 120.0f;
    [SerializeField] public float moveInertiaDuration = 0.5f;
    [SerializeField] public float moveInertiaRate = 0.35f;
    [SerializeField] public bool moveTowardTarget = true;
}
