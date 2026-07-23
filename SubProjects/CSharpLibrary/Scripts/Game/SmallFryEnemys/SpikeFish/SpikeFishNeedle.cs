using System;

public class SpikeFishNeedle : MonoScript
{
    /* ----- パラメータ ----- */
    [SerializeField] private float initialSpeed = 60.0f;  // 発射直後の初速
    [SerializeField] private float maxSpeed     = 300.0f; // 最終的に到達する最大速度
    [SerializeField] private float acceleration = 900.0f; // 初速からmaxSpeedへ到達する加速度
    [SerializeField] private float aliveTime    = 2.0f;

    /* ----- 実行時状態 ----- */
    private Vector3 direction_    = Vector3.zero;
    private float   currentSpeed_ = 0.0f;
    private float   lifeTimer_    = 0.0f;

    // FireNeedle から発射方向を設定する
    public void SetDirection(Vector3 dir)
    {
        if (transform == null) { return; }

        direction_    = dir.Normalized();
        currentSpeed_ = initialSpeed;
        float roll = Mathf.Atan2(direction_.x, direction_.y);
        transform.rotate = Quaternion.MakeFromAxis(Vector3.back, roll);
    }

    public override void Update()
    {
        if (transform == null) { return; }

        currentSpeed_ = Math.Min(maxSpeed, currentSpeed_ + acceleration * Time.deltaTime);
        transform.position += direction_ * currentSpeed_ * Time.deltaTime;

        lifeTimer_ += Time.deltaTime;
        if (lifeTimer_ >= aliveTime)
        {
            entity.Destroy();
        }
    }
}
