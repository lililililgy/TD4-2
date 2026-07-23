using System;

// 攻撃が命中したときに衝突点へエフェクトのプレハブを生成する演出。
// AttackCollision と同じ Entity にアタッチする。
// 付いていない攻撃はエフェクトが出ない（＝敵の攻撃のプレハブには付けない）ので、
// 「プレイヤーの攻撃だけ出す」といった切り分けはアタッチの有無で行う。
//
// ShakeOnHit / HitStopOnHit と同じ分担で、AttackCollision は「当たった相手」を渡すだけ。
// どのプレハブをどこに出すかはこのクラスの責務。
public class EffectOnHit : MonoScript {

    // 生成するプレハブ名（Assets/Prefabs/ 配下のファイル名から .prefab を除いたもの）
    [SerializeField] private string effectPrefabName_ = "PlayerHitEffect";

    // 生成したエフェクトを何秒で破棄するか。0 以下なら破棄しない（プレハブ側に寿命がある場合）。
    // プレハブの演出（TransformTween など）が終わりきる長さにする
    [SerializeField] private float lifeTime_ = 0.3f;

    // 衝突点から相手側へずらす量。0 なら接触点ちょうど。
    // 自分のコライダー表面に出ると埋まって見えるときに、少し相手へ寄せるために使う。
    [SerializeField] private float depthOffset_ = 0.0f;

    // 命中時に AttackCollision から呼ばれる。target は実際にダメージが通った相手
    public void OnHit(Entity target) {
        if (String.IsNullOrEmpty(effectPrefabName_) || target == null) {
            return;
        }

        Entity effect = ecsGroup.CreateEntity(effectPrefabName_);
        if (!effect) {
            return;
        }

        // position への代入だけだと反映がフレーム末尾のバッチ送信任せになるので、
        // ネイティブへ直接書いて即座に配置する
        Vector3 point = SpawnPosition(target);
        effect.transform.SetPositionImmediate(point);

        // TODO: 位置ズレ調査用の一時ログ。原因が分かったら消す
        Debug.LogWarning("[EffectOnHit]"
            + " self="   + Vector3.ToSimpleString(entity.transform.position)
            + " target=" + Vector3.ToSimpleString(target.transform.position)
            + " point="  + Vector3.ToSimpleString(point)
            + " effect=" + effect.name + "(id " + effect.Id + ")"
            + " readback=" + Vector3.ToSimpleString(effect.transform.GetWorldPosition()));

        // プレハブが既に寿命を持っているなら二重に付けない（Destroy が二度走るのを避ける）
        if (lifeTime_ > 0.0f && effect.GetScript<TimedDestruction>() == null) {
            TimedDestruction timedDestruction = effect.AddScript<TimedDestruction>();
            if (timedDestruction != null) {
                timedDestruction.lifeTime = lifeTime_;
            }
        }
    }

    // 接触点を推定し、必要なら相手方向へ depthOffset_ だけずらす
    private Vector3 SpawnPosition(Entity target) {
        Vector3 point = CollisionPoint.Estimate(entity, target);
        if (depthOffset_ == 0.0f) {
            return point;
        }

        Vector3 toTarget = target.transform.position - point;
        if (toTarget.LengthSq() <= 0.0001f) {
            return point;
        }
        return point + toTarget.Normalized() * depthOffset_;
    }
}
