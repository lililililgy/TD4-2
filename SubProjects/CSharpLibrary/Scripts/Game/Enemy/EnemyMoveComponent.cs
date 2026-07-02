using System;

// 敵の移動ドライバ（汎用 Mover の利用例）。指定ターゲットへ向かう方向を Mover に渡すだけ。
// 移動パラメータはこのコンポーネントの SerializeField から1つ構築する（ステート機械は不要）。
// プレイヤーと同じ物理（加速・減速・旋回・最大速度）がそのまま効く。
public class EnemyMoveComponent : MonoScript {

    [SerializeField] private string targetName_   = "Player"; // 追尾対象の Entity 名
    [SerializeField] private float  stopDistance_ = 0.0f;     // この距離以内では止まる

    // 移動パラメータ（プレイヤーの各 State と同じ項目）
    [SerializeField] private float accel_                = 256.0f;
    [SerializeField] private float maxSpeed_             = 256.0f;
    [SerializeField] private float decelSmoothTime_      = 0.2f;
    [SerializeField] private float decelMaxSmoothSpeed_  = 12.0f;
    [SerializeField] private float rotateSmoothTime_     = 0.2f; // 旋回の効き（小さいほど機敏）
    [SerializeField] private float rotateMaxSmoothSpeed_ = 3.0f; // 最大旋回レート(rad/s)。敵は重め

    [SerializeField] private float paramReleaseSmoothTime_     = 0.25f;
    [SerializeField] private float paramReleaseMaxSmoothSpeed_ = 100000.0f;

    private Mover mover_;
    private MoveParam moveParam_;

    public override void Initialize() {
        mover_ = new Mover(paramReleaseSmoothTime_, paramReleaseMaxSmoothSpeed_);
        // 敵は常にターゲット方向（最終的な進行方向）を直接 Mover へ渡すので、
        // canMove/moveForward の解釈は不要（Mover も解釈しない）。
        moveParam_ = new MoveParam {
            accel_                = accel_,
            maxSpeed_             = maxSpeed_,
            decelSmoothTime_      = decelSmoothTime_,
            decelMaxSmoothSpeed_  = decelMaxSmoothSpeed_,
            rotateSmoothTime_     = rotateSmoothTime_,
            rotateMaxSmoothSpeed_ = rotateMaxSmoothSpeed_,
        };
    }

    public override void Update() {
        Transform transform = entity.GetComponent<Transform>();
        Vector2 desiredDir = new Vector2(0.0f, 0.0f);

        // ターゲットへの方向を求める（停止距離より近ければ止まる）
        Entity target = ecsGroup.FindEntity(targetName_);
        if (target != null && transform != null) {
            Transform targetTrans = target.GetComponent<Transform>();
            if (targetTrans != null) {
                Vector3 to = targetTrans.position - transform.position;
                Vector2 to2 = new Vector2(to.x, to.y);
                if (to2.Length() > stopDistance_) {
                    desiredDir = to2;
                }
            }
        }

        mover_.Move(transform, desiredDir, moveParam_);
    }
}
