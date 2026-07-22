// 攻撃が命中したときにカメラを揺らす演出。AttackCollision と同じ Entity にアタッチする。
// 付いていない攻撃は揺れない（＝敵の攻撃のプレハブには付けない）ので、
// 「プレイヤーの攻撃だけ揺らす」といった切り分けはアタッチの有無で行う。
//
// ダメージ→揺れ幅の算出はこのクラスの責務で、AttackCollision は与えたダメージを渡すだけ。
// PlayerAttackComponent（速度→ダメージ）と同じ分担で、AttackCollision は演出を知らない。
//
// 自分ではカメラを触らず、CameraShakeEvent を流すだけ。受け取るのはカメラ側の CameraShake。
public class ShakeOnHit : MonoScript {

    // referenceDamage_ のダメージを与えたときに referenceAmplitude_ だけ揺れる。
    // 実際の揺れ幅はここから線形（ダメージ2倍なら揺れ幅2倍）に伸縮し、min/max で頭打ちにする。
    // プレイヤーの体当たりは速度でダメージが変わるため、代表値を入れておく。
    [SerializeField] private float referenceDamage_    = 50.0f;
    [SerializeField] private float referenceAmplitude_ = 900.0f;

    // 揺れ幅の下限・上限。弱い当たりを潰さず、強い一撃で画面が破綻しないようにする
    [SerializeField] private float minAmplitude_ = 500.0f;
    [SerializeField] private float maxAmplitude_ = 1600.0f;

    // referenceDamage_ のときの継続時間(秒)
    [SerializeField] private float referenceDuration_ = 0.15f;

    // 継続時間の下限・上限(秒)。durationScale_ で伸縮した結果をここで頭打ちにする
    [SerializeField] private float minDuration_ = 0.1f;
    [SerializeField] private float maxDuration_ = 0.3f;

    // 継続時間をダメージに追従させる度合い。0 なら常に referenceDuration_ で固定、
    // 1 なら揺れ幅と同じ比率で伸縮する（強い一撃ほど長く揺れる）
    [SerializeField] private float durationScale_ = 0.4f;

    // 揺れの速さ。ダメージでは変えない（大きいほど細かく震える）
    [SerializeField] private float frequency_ = 28.0f;

    // 命中時に AttackCollision から呼ばれる。dealtDamage は会心倍率を掛けたあとの実ダメージ
    public void OnHit(float dealtDamage) {
        DamageShake.Publish(
            dealtDamage,
            referenceDamage_, referenceAmplitude_, minAmplitude_, maxAmplitude_,
            referenceDuration_, minDuration_, maxDuration_, durationScale_, frequency_);
    }
}
