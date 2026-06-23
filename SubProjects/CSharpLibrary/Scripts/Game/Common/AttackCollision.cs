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

        HP hp = collision.GetScript<HP>();
        if (hp == null) {
            return; // HP を持たない相手には何もしない
        }

        hp.TakeDamage(damage_);

        if (destroyOnHit_) {
            entity.Destroy();
        }
    }

    public float Damage {
        get { return damage_; }
        set { damage_ = value; }
    }
}
