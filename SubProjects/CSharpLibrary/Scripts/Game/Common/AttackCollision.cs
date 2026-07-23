// 汎用の攻撃判定。衝突相手の HP を取得してダメージを与えるだけに専念する。
// 誰が誰に当たるか（敵味方）は engine のコライダ設定（衝突マトリクス）に任せ、
// このスクリプトは「HP を持つ相手ならダメージを与える」という1点だけを担う。
// 弾・プレイヤーの頭・敵の体当たりなど、攻撃する側のプレハブに付ければ使い回せる。
//
// ダメージを動的に変えたい場合（例: プレイヤー速度依存）は、外部のスクリプトが
// Damage プロパティを書き換える。速度→ダメージの算出はそちら側の責務とする。
public class AttackCollision : MonoScript {

    [SerializeField] private float damage_       = 10;    // 与えるダメージ
    [SerializeField] private bool  destroyOnHit_ = false; // 命中したら自分を破棄する（弾向け）

    public override void OnCollisionEnter(Entity collision) {
        if (collision == null || collision.Id == entity.Id) {
            return; // 自分自身は無視
        }

        // 会心(クリティカル)補正。Critical を持っていれば、攻撃者の向きと相手への角度差に
        // 応じた倍率を取得してダメージに掛ける。持っていなければ等倍。
        Critical critical = entity.GetScript<Critical>();
        float multiplier = critical != null ? critical.DamageMultiplierAgainst(collision) : 1.0f;
        float dealtDamage = damage_ * multiplier;

        GesoWeakPoint weakPoint = collision.GetScript<GesoWeakPoint>();
        if (weakPoint != null) {
            weakPoint.Damage(dealtDamage);
            OnDamageDealt(dealtDamage);
            return;
        }

        YadokariWeakPoint yadokariWeakPoint = collision.GetScript<YadokariWeakPoint>();
        if (yadokariWeakPoint != null) {
            yadokariWeakPoint.Damage(dealtDamage);
            OnDamageDealt(dealtDamage);
            return;
        }

        // 相手が DamageRelay(中継)を持っていれば、その所有者(頭)の共有 HP へダメージを送る。
        // 持っていなければ従来通り相手の HP を直接削る（敵など）。
        DamageRelay relay = collision.GetScript<DamageRelay>();
        HP hp = relay != null ? relay.OwnerHp : collision.GetScript<HP>();
        if (hp == null) {
            return; // ダメージを送れる相手が居ない
        }

        // 中継経由でない直接被弾で、directlyDamageable_=false の HP(=共有ライフの本体/頭)は無視。
        if (relay == null && !hp.IsDirectlyDamageable) {
            return;
        }

        hp.TakeDamage(dealtDamage);

        OnDamageDealt(dealtDamage);
    }

    // ダメージが実際に通ったときの共通後処理。
    // 空振り（HP を持たない相手・無敵中の弾き）では呼ばれないので、当たった瞬間だけ演出が出る。
    private void OnDamageDealt(float dealtDamage) {
        // 命中演出。ShakeOnHit が付いている攻撃だけカメラが揺れる。
        // 揺れ幅の決定はあちらの責務なので、ここは実ダメージを渡すだけ。
        // 破棄より先に呼ぶ（Publish は同期呼び出しなので、この場で演出まで走る）。
        ShakeOnHit shake = entity.GetScript<ShakeOnHit>();
        if (shake != null) {
            shake.OnHit(dealtDamage);
        }

        if (destroyOnHit_) {
            entity.Destroy();
        }
    }

    public float Damage {
        get { return damage_; }
        set { damage_ = value; }
    }
}
