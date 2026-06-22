using System;

// 卵(roe)。プレイヤーの RoeTrail を「自分の順番 × 間隔」ぶん後ろでサンプリングして追従する。
// 卵は Prefab から生成される別 Entity なので、このスクリプトは卵プレハブ側に付ける。
//
// 隊列の順番(order_)は RoeManager から自分の entityId をキーに引く（生成直後に push されないため）。
// 手動でシーンに置く場合は inspector の order_ がそのまま使われる。
public class RoeFollower : MonoScript {

    [SerializeField] private string leaderName_     = "Player";  // トレイルと順番の取得元
    [SerializeField] private int    order_          = 1;         // 隊列の順番（1が先頭の卵）
    [SerializeField] private float  playerOffset_   = 8.0f;      // プレイヤー → 先頭卵 の距離
    [SerializeField] private float  roeOffset_      = 8.0f;      // 卵 → 卵 の距離
    [SerializeField] private float  smoothTime_     = 0.08f;     // 追従の滑らかさ（0で即時）
    [SerializeField] private float  maxSmoothSpeed_ = 100000.0f;

    private RoeTrail trail_;
    private RoeManager manager_;
    private Vector3 smoothVel_ = Vector3.zero;

    public override void Initialize() {
        Entity leader = ecsGroup.FindEntity(leaderName_);
        if (leader == null) {
            return;
        }

        trail_ = leader.GetScript<RoeTrail>();
        manager_ = leader.GetScript<RoeManager>();
    }

    public override void Update() {
        if (trail_ == null) {
            return;
        }

        // 隊列順は毎フレーム RoeManager から entityId で引く（卵が発射で抜けると詰め直される）。
        // 取れなければ inspector の order_ をそのまま使う。
        if (manager_ != null) {
            int assigned = manager_.GetOrder(entity.Id);
            if (assigned > 0) {
                order_ = assigned;
            }
        }

        // 先頭卵は playerOffset_、以降は roeOffset_ ずつ後ろ（order_ は1始まり）
        float distance = playerOffset_ + roeOffset_ * (order_ - 1);
        Vector3 target = trail_.SampleBehind(distance);
        transform.position = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
            transform.position, target, ref smoothVel_, smoothTime_, Time.deltaTime, maxSmoothSpeed_);
    }
}
