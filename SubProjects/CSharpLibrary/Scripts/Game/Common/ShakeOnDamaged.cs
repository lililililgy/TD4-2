// 被弾したときにカメラを揺らす演出。HP と同じ Entity にアタッチする。
// 与えた側の ShakeOnHit と対になる、受けた側の演出。
//
// 検知は HP.TotalDamageTaken（累計）の差分で行う。理由は2つ:
//   1. CurrentHp の減少では代用できない。プレイヤーの HP は残機(=卵数)と同期されるため、
//      発射で卵を消費したときにも減り、被弾と区別が付かない。
//   2. 「今フレーム被弾した」フラグ方式は、フラグを落とす側と読む側の実行順で取りこぼす。
//
// 中継(DamageRelay)経由の被弾も、最終的には所有者の HP.TakeDamage を通るので拾える。
public class ShakeOnDamaged : MonoScript {

    // referenceDamage_ のダメージを受けたときに referenceAmplitude_ だけ揺れる。
    // プレイヤーの HP は残機(卵数)なので、1発で減る卵の数を基準に入れる。
    [SerializeField] private float referenceDamage_    = 8.0f;
    [SerializeField] private float referenceAmplitude_ = 1600.0f;

    // 揺れ幅の下限・上限。かすり傷を潰さず、大ダメージで画面が破綻しないようにする
    [SerializeField] private float minAmplitude_ = 1000.0f;
    [SerializeField] private float maxAmplitude_ = 2600.0f;

    // referenceDamage_ のときの継続時間(秒)
    [SerializeField] private float referenceDuration_ = 0.35f;

    // 継続時間の下限・上限(秒)。durationScale_ で伸縮した結果をここで頭打ちにする
    [SerializeField] private float minDuration_ = 0.25f;
    [SerializeField] private float maxDuration_ = 0.7f;

    // 継続時間をダメージに追従させる度合い。0 なら常に referenceDuration_ で固定
    [SerializeField] private float durationScale_ = 0.4f;

    // 揺れの速さ。ダメージでは変えない
    [SerializeField] private float frequency_ = 20.0f;

    private HP hp_;
    private float lastTotalDamage_;

    public override void Initialize() {
        hp_ = entity.GetScript<HP>();
        // 開始時点の累計を基準にする（初期化前の分で1発揺れてしまうのを防ぐ）
        lastTotalDamage_ = hp_ != null ? hp_.TotalDamageTaken : 0.0f;
    }

    public override void Update() {
        if (hp_ == null) {
            return;
        }

        float total = hp_.TotalDamageTaken;
        float damage = total - lastTotalDamage_;
        lastTotalDamage_ = total;

        // 同一フレームに複数回被弾した場合は合算されるので、揺れも1回にまとまる
        DamageShake.Publish(
            damage,
            referenceDamage_, referenceAmplitude_, minAmplitude_, maxAmplitude_,
            referenceDuration_, minDuration_, maxDuration_, durationScale_, frequency_);
    }
}
