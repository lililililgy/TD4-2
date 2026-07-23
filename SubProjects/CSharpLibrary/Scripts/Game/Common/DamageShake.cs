// ダメージ量からカメラの揺れ幅・継続時間を決めて CameraShakeEvent を流す共通処理。
// 「与えた時(ShakeOnHit)」と「受けた時(ShakeOnDamaged)」で同じ算出式を使うためのヘルパで、
// パラメータは呼び出し元の MonoScript が SerializeField で持つ（調整はプレハブ側で行う）。
//
// MonoScript ではないのでアタッチは不要。
public static class DamageShake {

    // referenceDamage_ のダメージで referenceAmplitude だけ揺れる、という基準からの線形比例。
    // 揺れ幅・継続時間ともに min/max で頭打ちにし、
    // 継続時間はクランプ後の揺れ幅の倍率に durationScale の分だけ追従させる。
    public static void Publish(
        float damage,
        float referenceDamage,
        float referenceAmplitude,
        float minAmplitude,
        float maxAmplitude,
        float referenceDuration,
        float minDuration,
        float maxDuration,
        float durationScale,
        float frequency) {

        if (damage <= 0.0f) {
            return; // ダメージ0では揺らさない
        }

        // ダメージ比。referenceDamage が未設定(0以下)なら比率を使わず基準値そのままにする
        float ratio = referenceDamage > 0.0f ? damage / referenceDamage : 1.0f;

        float amplitude = Mathf.Clamp(referenceAmplitude * ratio, minAmplitude, maxAmplitude);
        if (amplitude <= 0.0f) {
            return;
        }

        // 上限に張り付いたあとダメージだけ増えて時間が伸び続ける事故を防ぐため、
        // ratio ではなく「クランプ済みの倍率」に追従させる。
        float clampedRatio = referenceAmplitude > 0.0f ? amplitude / referenceAmplitude : 1.0f;
        float duration = Mathf.Clamp(
            referenceDuration * Mathf.Lerp(1.0f, clampedRatio, durationScale),
            minDuration, maxDuration);
        if (duration <= 0.0f) {
            return;
        }

        MessageBus.Publish(new CameraShakeEvent(duration, amplitude, frequency));
    }
}
