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

        Critical critical = entity.GetScript<Critical>();
        float multiplier = critical != null ? critical.DamageMultiplierAgainst(collision) : 1.0f;

        GesoWeakPoint weakPoint = collision.GetScript<GesoWeakPoint>();
        if (weakPoint != null) {
            weakPoint.Damage(damage_ * multiplier);
            if (destroyOnHit_) {
                entity.Destroy();
            }
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

        // 会心(クリティカル)補正。Critical を持っていれば、攻撃者の向きと相手への角度差に
        // 応じた倍率を取得してダメージに掛ける。持っていなければ等倍。
        hp.TakeDamage(damage_ * multiplier);

        if (destroyOnHit_) {
            entity.Destroy();
        }
    }

    public float Damage {
        get { return damage_; }
        set { damage_ = value; }
    }
}
