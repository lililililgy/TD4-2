// 攻撃が命中したときにヒットストップ（一時的な減速）を出す演出。
// AttackCollision と同じ Entity にアタッチする。付いていない攻撃では止まらないので、
// 「プレイヤーの攻撃だけ止める」といった切り分けはアタッチの有無で行う（ShakeOnHit と同じ作法）。
//
// 呼ばれるのは実際にダメージが通ったときだけ（AttackCollision.OnDamageDealt）。
// HP を持たない相手への空振りや、無敵で弾かれた当たりでは止まらない。
//
// ダメージ→止め時間の算出がこのクラスの責務で、AttackCollision は実ダメージを渡すだけ。
// 減速そのものは触らず HitStop へ要求を出すだけなので、弾のように命中と同時に破棄される
// 攻撃でも解除が取り残されない。
public class HitStopOnHit : MonoScript {

    // 減速率。0 に近いほど強く止まる（0 = 完全停止）。
    // 止まりの強さは当たりの「重さ」ではなく作品全体の手触りなので、ダメージでは変えない。
    [SerializeField] private float timeScale_ = 0.05f;

    // referenceDamage_ のダメージを与えたときに referenceDuration_ 秒止まる。
    // 実際の時間はここから線形に伸縮し、min/max で頭打ちにする。
    // プレイヤーの体当たりは速度でダメージが変わるため、代表値を入れておく。
    [SerializeField] private float referenceDamage_   = 50.0f;
    [SerializeField] private float referenceDuration_ = 0.07f;

    // 止め時間の下限・上限(秒・実時間)。弱い当たりでも手応えを残しつつ、
    // 強い一撃で操作が長く奪われないようにする。
    [SerializeField] private float minDuration_ = 0.03f;
    [SerializeField] private float maxDuration_ = 0.14f;

    // 止め時間をダメージに追従させる度合い。0 なら常に referenceDuration_ で固定、
    // 1 ならダメージに正比例（強い一撃ほど長く止まる）。
    [SerializeField] private float durationScale_ = 0.6f;

    // 命中時に AttackCollision から呼ばれる。dealtDamage は会心倍率を掛けたあとの実ダメージ。
    public void OnHit(float dealtDamage) {
        if (dealtDamage <= 0.0f) {
            return; // ダメージ0（速度が乗っていない体当たり等）では止めない
        }

        // ダメージ比。referenceDamage_ が未設定(0以下)なら比率を使わず基準値そのままにする。
        float ratio = referenceDamage_ > 0.0f ? dealtDamage / referenceDamage_ : 1.0f;

        float duration = Mathf.Clamp(
            referenceDuration_ * Mathf.Lerp(1.0f, ratio, durationScale_),
            minDuration_, maxDuration_);

        HitStop.Request(timeScale_, duration);
    }
}
