using System;


public class CameraShake : MonoScript {

    [SerializeField] private int   shakeCycles          = 2;      // 上下に揺れる回数(1〜2往復想定)
    [SerializeField] private float shakeDuration         = 0.25f;  // 揺れが収まるまでの時間
    [SerializeField] private float offsetSmoothTime       = 0.03f; // 目標オフセットへ追従させる時間(0でスナップ)
    [SerializeField] private float offsetMaxSmoothSpeed   = 100000.0f;

    private bool    isShaking_     = false;
    private float   shakeTimer_    = 0.0f;
    private float   intensity_     = 0.0f;

    private Vector3 currentOffset_ = Vector3.zero;
    private Vector3 smoothVel_     = Vector3.zero;

    // 追従先がこの Offset を自身の位置に加算する
    public Vector3 Offset {
        get { return currentOffset_; }
    }

    public override void Update() {
        Vector3 targetOffset = Vector3.zero;

        if (isShaking_) {
            shakeTimer_ += Time.deltaTime;
            float t = Mathf.Clamp01(shakeTimer_ / shakeDuration);

            // 時間経過で振幅が収まっていく減衰カーブ
            float envelope = 1.0f - Ease.In.Quad(t);
            // 指定回数だけ上下に揺れるサイン波
            float wave = Mathf.Sin(t * shakeCycles * Mathf.PI * 2.0f);
            targetOffset = new Vector3(0.0f, wave * intensity_ * envelope, 0.0f);

            if (t >= 1.0f) {
                isShaking_ = false;
            }
        }

        // 位置がパッと切り替わらないよう、目標オフセットへ滑らかに追従させる
        currentOffset_ = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
            currentOffset_, targetOffset, ref smoothVel_, offsetSmoothTime, Time.deltaTime, offsetMaxSmoothSpeed);
    }

    // intensity: 揺れの強さ(縦方向の振幅)。0以下は無視する。
    public void Shake(float intensity) {
        if (intensity <= 0.0f) {
            return;
        }

        intensity_  = intensity;
        shakeTimer_ = 0.0f;
        isShaking_  = true;
    }
}
