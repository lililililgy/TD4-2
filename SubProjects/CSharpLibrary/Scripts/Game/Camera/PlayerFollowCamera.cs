using System;

// プレイヤー（指定名の Entity）を追従するカメラ。カメラ用 Entity に付ける。
// シーン内の対象 Entity は ecsGroup.FindEntity(名前) で取得する。
public class PlayerFollowCamera : MonoScript {

    [SerializeField] private string  targetName_     = "Player";
    [SerializeField] private Vector3 offset_         = new Vector3(0.0f, 0.0f, -144.3f); // 対象からの相対位置
    [SerializeField] private float   smoothTime_     = 0.15f;        // 追従の滑らかさ（0で即時）
    [SerializeField] private float   maxSmoothSpeed_ = 100000.0f;    // 追従速度の上限（実質無制限）

    private Entity target_;
    private Vector3 smoothVel_ = Vector3.zero;
    private Vector3 shakeOffset_ = Vector3.zero;
    private float shakeDuration_;
    private float shakeRemaining_;
    private float shakeIntensity_;
    private float shakeFrequency_;
    private float shakePhase_;

    public override void Initialize() {
        // シーン内の対象 Entity を名前で取得
        target_ = ecsGroup.FindEntity(targetName_);
    }

    public override void Update() {
        Vector3 followedPosition = transform.position - shakeOffset_;
        if (target_ != null) {
            Transform targetTransform = target_.GetComponent<Transform>();
            if (targetTransform != null) {
                Vector3 desired = targetTransform.position + offset_;
                followedPosition = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
                    followedPosition, desired, ref smoothVel_, smoothTime_, Time.deltaTime, maxSmoothSpeed_);
            }
        }

        UpdateShake();
        transform.position = followedPosition + shakeOffset_;
    }

    public void Shake(float duration, float intensity, float frequency) {
        if (duration <= 0.0f || intensity <= 0.0f) {
            return;
        }

        shakeDuration_ = duration;
        shakeRemaining_ = duration;
        shakeIntensity_ = intensity;
        shakeFrequency_ = frequency > 0.0f ? frequency : 1.0f;
    }

    private void UpdateShake() {
        if (shakeRemaining_ <= 0.0f) {
            shakeOffset_ = Vector3.zero;
            return;
        }

        shakeRemaining_ -= Time.deltaTime;
        if (shakeRemaining_ < 0.0f) {
            shakeRemaining_ = 0.0f;
        }

        shakePhase_ += shakeFrequency_ * Mathf.PI * 2.0f * Time.deltaTime;
        float damping = Mathf.Clamp01(shakeRemaining_ / shakeDuration_);
        float amplitude = shakeIntensity_ * damping;
        shakeOffset_ = new Vector3(
            Mathf.Sin(shakePhase_) * amplitude,
            Mathf.Sin(shakePhase_ * 1.37f + 1.0f) * amplitude,
            0.0f);
    }
}
