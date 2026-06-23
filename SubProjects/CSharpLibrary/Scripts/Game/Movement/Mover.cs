using System;

// 汎用移動コア（素のクラス。コンポーネントではない）。
// プレイヤー・敵などのドライバが1つ保持し、毎フレーム Move を呼ぶ。
// 「進みたい方向(desiredDir)」と「移動パラメータ(MoveParam)」を渡すと、
// 加速・最大速度クランプ・減速・旋回・パラメータのアニメーションを行い Transform を動かす。
//
// プレイヤー固有だったのは「方向の供給元(入力)」と「パラメータの供給元(ステート)」だけなので、
// それらを外から渡す形にしてここを共通化している。
public class Mover {

    // paramRelease* : 移動パラメータ(accel/maxSpeed)を目標値へ下げるときの追従設定。
    //   上がる時は即時（一瞬の加速）、下がる時はこの平滑化で徐々に（ダッシュの尾）。
    public Mover(float paramReleaseSmoothTime, float paramReleaseMaxSmoothSpeed) {
        paramReleaseSmoothTime_ = paramReleaseSmoothTime;
        paramReleaseMaxSmoothSpeed_ = paramReleaseMaxSmoothSpeed;
    }

    // desiredDir: 進みたい生の方向（正規化不要。長さ0で停止扱い）。
    public void Move(Transform transform, Vector2 desiredDir, MoveParam param) {
        if (transform == null || param == null) {
            return;
        }

        const float kThresholdSpeed = 0.01f;

        // 現在の状態の移動パラメーター = 目標値（next）。current を next へアニメーション：
        //   上がる時は即時、下がる時は SmoothDamp（徐々に減速）。
        currentMaxSpeed_ = AnimateParam(currentMaxSpeed_, param.maxSpeed_, ref maxSpeedSmoothVel_);
        currentAccel_    = AnimateParam(currentAccel_,    param.accel_,    ref accelSmoothVel_);

        // 通常時は desiredDir を「正面(forward)」として記録する。
        // moveForward_（ダッシュ等）の間は方向入力を無視して、正面へ走り続ける。
        if (param.canMove_ && desiredDir.LengthSq() > kThresholdSpeed) {
            forwardDir_ = desiredDir.Normalized();
        }

        Vector2 input;
        if (param.moveForward_) {
            input = forwardDir_;
        } else if (param.canMove_) {
            input = desiredDir;
        } else {
            input = new Vector2(0.0f, 0.0f);
        }

        // 減衰
        if (input.LengthSq() <= 0f) {
            if (velocity_.LengthSq() < kThresholdSpeed) {
                velocity_ = Vector3.zero;
                return;
            }
            velocity_ = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
                velocity_, Vector3.zero, ref currentDecelSmoothSpeed_, param.decelSmoothTime_, Time.deltaTime, param.decelMaxSmoothSpeed_);
        } else {
            Vector3 moveDir = new Vector3(input.x, input.y, 0.0f);

            // 加速（アニメーション中の current を使う）
            Vector3 newVelo = velocity_ + moveDir * (currentAccel_ * Time.deltaTime);

            velocity_ = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
                velocity_.Normalized(), newVelo.Normalized(), ref currentRotateSmoothSpeed_, param.rotateSmoothTime_, Time.deltaTime, param.rotateMaxSmoothSpeed_) * newVelo.Length();

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

    // 実行時の状態
    private Vector3 velocity_ = Vector3.zero;
    private Vector3 currentDecelSmoothSpeed_ = Vector3.zero;
    private Vector3 currentRotateSmoothSpeed_ = Vector3.zero;

    // 正面（forward）方向。通常時の方向を記録し、moveForward_ 中はこの向きへ走り続ける。
    private Vector2 forwardDir_ = new Vector2(0.0f, 1.0f);

    // 移動パラメーターのアニメーション（current を 目標 next へ追従させる）
    private float currentAccel_ = 0.0f;
    private float currentMaxSpeed_ = 0.0f;
    private float accelSmoothVel_ = 0.0f;
    private float maxSpeedSmoothVel_ = 0.0f;

    private readonly float paramReleaseSmoothTime_;
    private readonly float paramReleaseMaxSmoothSpeed_;
}
