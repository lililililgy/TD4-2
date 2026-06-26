using System.Collections.Generic;

// 汎用の引き寄せ判定。衝突範囲に入っている相手を、毎フレーム自分の位置へ少しずつ移動させる。
// 重力井戸・吸い込み・経験値マグネットなどに使い回せる。
//
// 【このエンジンでの実装上の注意】
// OnCollisionStay は CollisionSystem 内で呼ばれ、その時点ではこのフレームの
// バッチ同期(SendAllBatches: C#→native)が既に終わっている。そのため
// OnCollisionStay の中で transform.position を書いてもネイティブへ反映されず、
// 次フレームの ReceiveAllBatches で上書きされて消える。
// → 実際の移動は Update() で行う（同期前なので反映される）。
//   OnCollisionStay は「今フレーム重なっている相手の記録」だけに使う。
//
// また、引き寄せ用コライダは Trigger にしておくこと。非トリガー同士だと
// CollisionSystem の PushBack(押し戻し) が毎フレーム効いて引き寄せと喧嘩する。
//
// 【座標について】
// transform.position は親基準の「ローカル座標」。距離・方向の計算に使うと親があるとき破綻する。
// ワールド座標はワールド行列の平行移動成分 matrix.m30/m31/m32 から取る。
// 移動量はワールド空間で求め、ローカルの position に加算して反映する
// （親に回転・スケールが無い前提。経験値オブジェクト等は通常ルートなので成立する）。
public class Attractor : MonoScript {

    [SerializeField] private float pullSpeed_    = 5.0f; // 引き寄せる速さ（単位/秒）
    [SerializeField] private float stopDistance_ = 0.0f; // この距離まで近づいたら止める

    // 今フレーム範囲内にいる相手の EntityId。OnCollisionStay で記録し Update で消費する。
    private readonly HashSet<int> targetIds_ = new HashSet<int>();

    // ワールド行列の平行移動成分＝ワールド座標。
    private static Vector3 WorldPosition(Transform t) {
        return new Vector3(t.matrix.m30, t.matrix.m31, t.matrix.m32);
    }

    public override void Update() {
        if (targetIds_.Count == 0) {
            return;
        }

        float step = pullSpeed_ * Time.deltaTime;
        Vector3 selfWorld = WorldPosition(entity.parent.transform);

        foreach (int id in targetIds_) {
            Entity target = ecsGroup.GetEntity(id);
            if (target == null || target.Id == entity.Id) {
                continue;
            }

            Transform targetTransform = target.transform;
            if (targetTransform == null) {
                continue;
            }

            // 方向・距離はワールド座標で計算する。
            Vector3 toSelf = selfWorld - WorldPosition(targetTransform);
            float distance = toSelf.Length();

            // 既に十分近い／同じ位置なら何もしない（ゼロ除算も回避）
            if (distance <= stopDistance_ || distance == 0.0f) {
                continue;
            }

            // 残り距離を超えないようクランプして行き過ぎ（往復ブレ）を防ぐ。
            float moveAmount = step;
            float remaining = distance - stopDistance_;
            if (moveAmount > remaining) {
                moveAmount = remaining;
            }

            // ワールド空間の移動量を、ローカルの position に反映する。
            targetTransform.position += toSelf.Normalized() * moveAmount;
        }

        // 記録は1フレーム限り。次フレームも重なっていれば OnCollisionStay が再登録する。
        targetIds_.Clear();
    }

    public override void OnCollisionStay(Entity collision) {
        if (collision == null || collision.Id == entity.Id) {
            return; // 自分自身は無視
        }
        targetIds_.Add(collision.Id);
    }
}
