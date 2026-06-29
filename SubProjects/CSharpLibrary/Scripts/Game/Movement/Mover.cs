using System;

// 汎用移動コア（素のクラス。コンポーネントではない）。物理のみを担当する。
// プレイヤー・敵などのドライバが1つ保持し、毎フレーム Move を呼ぶ。
// 「進みたい最終的な方向(desiredDir)」と「移動パラメータ(MoveParam)」を渡すと、
// 加速・最大速度クランプ・減速・旋回・パラメータのアニメーションを行い Transform を動かす。
//
// 入力の「解釈」（移動可否・正面固定走り・正面方向の記録など）はここでは行わない。
// それらはドライバ側（例: PlayerMoveComponent + MoveDirector）で解決し、
// 結果の方向だけを desiredDir として渡すこと。長さ0で停止（減速）扱い。
public class Mover {

    // paramRelease* : 移動パラメータ(accel/maxSpeed)を目標値へ下げるときの追従設定。
    //   上がる時は即時（一瞬の加速）、下がる時はこの平滑化で徐々に（ダッシュの尾）。
    public Mover(float paramReleaseSmoothTime, float paramReleaseMaxSmoothSpeed) {
        paramReleaseSmoothTime_ = paramReleaseSmoothTime;
        paramReleaseMaxSmoothSpeed_ = paramReleaseMaxSmoothSpeed;
    }

    // desiredDir: 進みたい最終的な方向（正規化不要。長さ0で停止扱い）。解釈はドライバ側で済ませる。
    public void Move(Transform transform, Vector2 desiredDir, MoveParam param) {
        if (transform == null || param == null) {
            return;
        }

        const float kThresholdSpeed = 0.01f;

        // 現在の状態の移動パラメーター = 目標値（next）。current を next へアニメーション：
        //   上がる時は即時、下がる時は SmoothDamp（徐々に減速）。
        currentMaxSpeed_ = AnimateParam(currentMaxSpeed_, param.maxSpeed_, ref maxSpeedSmoothVel_);
        currentAccel_    = AnimateParam(currentAccel_,    param.accel_,    ref accelSmoothVel_);
        currentMinSpeed_ = AnimateParam(currentMinSpeed_, param.minSpeed_, ref minSpeedSmoothVel_);

        // 減衰
        if (desiredDir.LengthSq() <= kThresholdSpeed) {
            if (velocity_.LengthSq() < kThresholdSpeed) {
                velocity_ = Vector3.zero;
                return;
            }
            velocity_ = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
                velocity_, Vector3.zero, ref currentDecelSmoothSpeed_, param.decelSmoothTime_, Time.deltaTime, param.decelMaxSmoothSpeed_);
        } else {
            Vector2 dir = desiredDir.Normalized();

            // 速度の大きさ：加速して [minSpeed, maxSpeed] にクランプ。
            // 入力中は minSpeed を下回らないので、真後ろ入力でも速度ベクトルが 0 にならず、
            // 「向きを変える＝旋回」として表現できる（瞬間反転しない）。
            float minSpeed = Mathf.Clamp(currentMinSpeed_, 0.0f, currentMaxSpeed_);
            float speed = velocity_.Length() + currentAccel_ * Time.deltaTime;
            speed = Mathf.Clamp(speed, minSpeed, currentMaxSpeed_);

            // 現在の進行方向（ほぼ停止しているときは直近の進行方向を採用）。
            Vector2 heading = (velocity_.LengthSq() > kThresholdSpeed)
                ? new Vector2(velocity_.x, velocity_.y).Normalized()
                : headingDir_;

            // 進行方向を入力方向へ旋回させる。角度空間で SmoothDamp するので、
            // 真後ろ(180°)入力でも速度ベクトルが 0 を通らず、最短回りで滑らかに回り込む。
            float curAngle    = Mathf.Atan2(heading.y, heading.x);
            float targetAngle = curAngle + WrapPi(Mathf.Atan2(dir.y, dir.x) - curAngle);
            float newAngle    = SpringDamper.SmoothDamp<float, FloatDampTraits>(
                curAngle, targetAngle, ref rotateSmoothVel_, param.rotateSmoothTime_, Time.deltaTime, param.rotateMaxSmoothSpeed_);

            headingDir_ = new Vector2(Mathf.Cos(newAngle), Mathf.Sin(newAngle));
            velocity_ = new Vector3(headingDir_.x, headingDir_.y, 0.0f) * speed;
        }

        float cullRoll = Mathf.Atan2(velocity_.x, velocity_.y);
        transform.rotate = Quaternion.MakeFromAxis(Vector3.back, cullRoll);

        transform.position += velocity_ * Time.deltaTime;
    }

    // 現在速度（アニメや他システムが参照できるように公開）
    public Vector3 Velocity { get { return velocity_; } }

    // current を next へ追従。上がる時は即時、下がる時は SmoothDamp。
    private float AnimateParam(float current, float next, ref float smoothVel) {
        if (next >= current) {
            smoothVel = 0.0f;
            return next;
        }
        return SpringDamper.SmoothDamp<float, FloatDampTraits>(
            current, next, ref smoothVel, paramReleaseSmoothTime_, Time.deltaTime, paramReleaseMaxSmoothSpeed_);
    }

    // 角度差を (-PI, PI] に正規化（最短回転方向を選ぶ）。
    private static float WrapPi(float angle) {
        float twoPi = 2.0f * Mathf.PI;
        while (angle >  Mathf.PI) angle -= twoPi;
        while (angle < -Mathf.PI) angle += twoPi;
        return angle;
    }

    // 実行時の状態
    private Vector3 velocity_ = Vector3.zero;
    private Vector3 currentDecelSmoothSpeed_ = Vector3.zero;
    private float   rotateSmoothVel_ = 0.0f;   // 旋回角の SmoothDamp 速度(rad/s)

    // 直近の進行方向。ほぼ停止状態から動き出すときの旋回の起点に使う（物理の内部状態）。
    private Vector2 headingDir_ = new Vector2(0.0f, 1.0f);

    // 移動パラメーターのアニメーション（current を 目標 next へ追従させる）
    private float currentAccel_ = 0.0f;
    private float currentMaxSpeed_ = 0.0f;
    private float currentMinSpeed_ = 0.0f;

    private float accelSmoothVel_ = 0.0f;
    private float maxSpeedSmoothVel_ = 0.0f;
    private float minSpeedSmoothVel_ = 0.0f;

    private readonly float paramReleaseSmoothTime_;
    private readonly float paramReleaseMaxSmoothSpeed_;
}
