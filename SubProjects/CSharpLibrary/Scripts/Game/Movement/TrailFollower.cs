using System;

// リーダーの Trail を「自分の順番 × 間隔」ぶん後ろでサンプリングして追従する汎用追従コンポーネント。
// プレイヤー(頭)に対する胴体・触手や、母体に対する卵など、トレイルを持つ任意のリーダーに付けられる。
//
// 隊列の順番(order_)は inspector 値を使う。動的な隊列（卵など）は管理側が SetOrder() で上書きする。
public class TrailFollower : MonoScript {

    [SerializeField] private string leaderName_     = "Player";  // トレイル(RoeTrail)の取得元
    [SerializeField] private int    order_          = 1;         // 隊列の順番（1が先頭）
    [SerializeField] private float  leadOffset_     = 8.0f;      // リーダー → 先頭 の距離
    [SerializeField] private float  unitOffset_     = 8.0f;      // 各要素間の距離
    [SerializeField] private float  smoothTime_     = 0.08f;     // 追従の滑らかさ（0で即時）
    [SerializeField] private float  maxSmoothSpeed_ = 100000.0f;

    private RoeTrail trail_;
    private Vector3 smoothVel_ = Vector3.zero;

    // 隊列の順番。動的な隊列（卵など）は外部マネージャが SetOrder() で push する。
    public int Order { get { return order_; } }
    public void SetOrder(int order) { order_ = order; }

    public override void Initialize() {
        Entity leader = ecsGroup.FindEntity(leaderName_);
        if (leader == null) {
            return;
        }

        trail_ = leader.GetScript<RoeTrail>();
    }

    public override void Update() {
        if (trail_ == null) {
            return;
        }

        // 先頭は leadOffset_、以降は unitOffset_ ずつ後ろ（order_ は1始まり）
        float distance = leadOffset_ + unitOffset_ * (order_ - 1);
        Vector3 target = trail_.SampleBehind(distance);

        Vector3 prev = transform.position;
        transform.position = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
            prev, target, ref smoothVel_, smoothTime_, Time.deltaTime, maxSmoothSpeed_);

        // 進行方向を向く（2D・Z軸回り）。ほぼ静止しているフレームは今の向きを保持する。
        Vector3 move = transform.position - prev;
        if (move.x * move.x + move.y * move.y > FaceEpsilonSq) {
            float roll = Mathf.Atan2(move.x, move.y);
            transform.rotate = Quaternion.MakeFromAxis(Vector3.back, roll);
        }
    }

    // この距離(二乗)未満しか進んでいないフレームは回転を更新しない（向きのちらつき防止）
    private const float FaceEpsilonSq = 0.0001f;
}
