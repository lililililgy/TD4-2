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
        currentAccel_ = AnimateParam(currentAccel_, param.accel_, ref accelSmoothVel_);

        // 減衰
        if (desiredDir.LengthSq() <= kThresholdSpeed) {
            if (velocity_.LengthSq() < kThresholdSpeed) {
                velocity_ = Vector3.zero;
                return;
            }
            velocity_ = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
                velocity_, Vector3.zero, ref currentDecelSmoothSpeed_, param.decelSmoothTime_, Time.deltaTime, param.decelMaxSmoothSpeed_);
        } else {
            // 加速（アニメーション中の current を使う）
            Vector3 moveDir = new Vector3(desiredDir.x, desiredDir.y, 0.0f).Normalized();
            Vector3 newVelo = velocity_ + moveDir * (currentAccel_ * Time.deltaTime);

            velocity_ = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
                velocity_.Normalized(), newVelo.Normalized(), ref rotateSmoothVel_, param.rotateSmoothTime_, Time.deltaTime, param.rotateMaxSmoothSpeed_) * newVelo.Length();

            if (velocity_.LengthSq() > currentMaxSpeed_ * currentMaxSpeed_) {
                velocity_ = velocity_.Normalized() * currentMaxSpeed_;
            }
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
        while (angle > Mathf.PI) angle -= twoPi;
        while (angle < -Mathf.PI) angle += twoPi;
        return angle;
    }

    // 実行時の状態
    private Vector3 velocity_ = Vector3.zero;
    private Vector3 currentDecelSmoothSpeed_ = Vector3.zero;
    private Vector3 rotateSmoothVel_ = Vector3.zero;

    // 移動パラメーターのアニメーション（current を 目標 next へ追従させる）
    private float currentAccel_ = 0.0f;
    private float currentMaxSpeed_ = 0.0f;

    private float accelSmoothVel_ = 0.0f;
    private float maxSpeedSmoothVel_ = 0.0f;

    private readonly float paramReleaseSmoothTime_;
    private readonly float paramReleaseMaxSmoothSpeed_;
}
