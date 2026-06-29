using System;

// 攻撃側のプレハブに付ける「会心(クリティカル)判定」コンポーネント。
// 攻撃者(this)の向きと、攻撃者 → 被弾相手(collidedEntity) を結ぶベクトルとの角度差が
// threshold_ 以下なら会心とみなし、ダメージ倍率 criticalRate_ を返す。
// 例: 正面で捉えた一撃に重みを付ける、突進方向に一致した体当たりを強くする、など。
//
// ダメージの適用そのものは AttackCollision の責務。AttackCollision が衝突時にこのスクリプトを
// 参照し、DamageMultiplierAgainst(相手) で得た倍率を damage_ に掛けて与える。
// （会心判定は衝突相手ごとに角度で決まるため、AttackCollision 側から問い合わせる形にしている。
//   衝突コールバックの実行順に依存させないための設計。 cf. AttackCollision が DamageRelay を引くのと同じ作法）
public class Critical : MonoScript {

    [SerializeField] private float threshold_    = 30.0f; // 会心とみなす角度差(度)。0で完全一致のみ
    [SerializeField] private float criticalRate_ = 2.0f;  // 会心時のダメージ倍率

    // 攻撃者の前方向(2D・ローカル +Y が前)と、攻撃者 → target のベクトルの角度差が threshold_ 以下なら
    // criticalRate_、そうでなければ 1.0(補正なし)を返す。target が無効/同一位置なら補正なし。
    public float DamageMultiplierAgainst(Entity target) {
        return IsCritical(target) ? criticalRate_ : 1.0f;
    }

    // 会心(クリティカル)かどうかの判定だけが欲しい場合に使う。
    public bool IsCritical(Entity target) {
        if (target == null) {
            return false;
        }

        // 攻撃者 → 相手 のベクトル。同一位置では向きが定義できないので会心としない。
        Vector3 toTarget = target.GetComponent<Transform>().position - transform.position;
        if (toTarget.LengthSq() <= 0.0f) {
            return false;
        }

        // 攻撃者の前方向（2D: ローカル +Y を現在の回転で回したもの）。
        Vector3 forward = Quaternion.RotateVector(transform.rotate, Vector3.up);

        // 2ベクトルの内積から角度差(度)を求める。
        float cos = Vector3.Dot(forward.Normalized(), toTarget.Normalized());
        cos = Mathf.Clamp(cos, -1.0f, 1.0f);
        float angleDeg = Mathf.Acos(cos) * Mathf.Rad2Deg;

        return angleDeg <= threshold_;
    }

    public float Threshold    { get { return threshold_;    } set { threshold_ = value;    } }
    public float CriticalRate { get { return criticalRate_; } set { criticalRate_ = value; } }
}
