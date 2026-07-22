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
    [SerializeField] private float referenceAmplitude_ = 0.2f;

    // 揺れ幅の下限・上限。弱い当たりを潰さず、強い一撃で画面が破綻しないようにする
    [SerializeField] private float minAmplitude_ = 0.05f;
    [SerializeField] private float maxAmplitude_ = 0.5f;

    // referenceDamage_ のときの継続時間(秒)
    [SerializeField] private float referenceDuration_ = 0.15f;

    // 継続時間をダメージに追従させる度合い。0 なら常に referenceDuration_ で固定、
    // 1 なら揺れ幅と同じ比率で伸縮する（強い一撃ほど長く揺れる）
    [SerializeField] private float durationScale_ = 0.5f;

    // 揺れの速さ。ダメージでは変えない（大きいほど細かく震える）
    [SerializeField] private float frequency_ = 25.0f;

    // 命中時に AttackCollision から呼ばれる。dealtDamage は会心倍率を掛けたあとの実ダメージ
    public void OnHit(float dealtDamage) {
        if (dealtDamage <= 0.0f) {
            return; // ダメージ0の当たりでは揺らさない
        }

        // ダメージ比。referenceDamage_ が未設定(0以下)なら比率を使わず基準値そのままにする
        float ratio = referenceDamage_ > 0.0f ? dealtDamage / referenceDamage_ : 1.0f;

        float amplitude = Mathf.Clamp(referenceAmplitude_ * ratio, minAmplitude_, maxAmplitude_);
        if (amplitude <= 0.0f) {
            return;
        }

        // 継続時間はクランプ後の揺れ幅に追従させる。
        // 上限に張り付いたあとダメージだけ増えても伸び続ける、という事故を防ぐため
        // ratio ではなく「クランプ済みの倍率」を使う。
        float clampedRatio = referenceAmplitude_ > 0.0f ? amplitude / referenceAmplitude_ : 1.0f;
        float duration = referenceDuration_ * Mathf.Lerp(1.0f, clampedRatio, durationScale_);
        if (duration <= 0.0f) {
            return;
        }

        MessageBus.Publish(new CameraShakeEvent(duration, amplitude, frequency_));
    }
}
