using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;


public class PlayerMoveComponent
    : MonoScript {

    public override void Initialize() { }

    public override void Update() {
        const float kThresholdSpeed = 0.01f;

        Transform transform = entity.GetComponent<Transform>();
        PlayerInputComponent inputComp = entity.GetScript<PlayerInputComponent>();
        PlayerStateComponent stateComp = entity.GetScript<PlayerStateComponent>();

        // 現在の状態の移動パラメーター = 目標値（next）
        PlayerMoveParam param = stateComp.CurrentMoveParam();
        float nextAccel_ = param.accel_;
        float nextMaxSpeed_ = param.maxSpeed_;

        // current を next へアニメーション：
        //   上がる時（ダッシュ開始）は即時 ＝ 一瞬の加速（attack）
        //   下がる時（通常へ復帰）は SmoothDamp ＝ 徐々に減速（release）＝ Dodge の尾
        currentMaxSpeed_ = AnimateParam(currentMaxSpeed_, nextMaxSpeed_, ref maxSpeedSmoothVel_);
        currentAccel_ = AnimateParam(currentAccel_, nextAccel_, ref accelSmoothVel_);

        // 通常時は入力方向を「正面（forward）」として記録する。
        // moveForward_（ダッシュ等）の間は入力を無視して、正面へ走り続ける。
        if (param.canMove_ && inputComp.MoveDir.LengthSq() > kThresholdSpeed) {
            forwardDir_ = inputComp.MoveDir.Normalized();
        }

        Vector2 input;
        if (param.moveForward_) {
            input = forwardDir_;
        } else if (param.canMove_) {
            input = inputComp.MoveDir;
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

    // current を next へ追従。上がる時は即時、下がる時は SmoothDamp。
    private float AnimateParam(float current, float next, ref float smoothVel) {
        if (next >= current) {
            smoothVel = 0.0f;
            return next;
        }
        return SpringDamper.SmoothDamp<float, FloatDampTraits>(
            current, next, ref smoothVel, paramReleaseSmoothTime_, Time.deltaTime, paramReleaseMaxSmoothSpeed_);
    }

    // 現状管理変数（実行時のみ。状態からは注入しない）
    private Vector3 velocity_ = Vector3.zero;
    private Vector3 currentDecelSmoothSpeed_ = Vector3.zero;
    private Vector3 currentRotateSmoothSpeed_ = Vector3.zero;

    // 正面（forward）方向。通常時の入力方向を記録し、moveForward_ 中はこの向きへ走り続ける。
    private Vector2 forwardDir_ = new Vector2(0.0f, 1.0f);

    // 移動パラメーターのアニメーション（current を 状態の next へ追従させる）
    private float currentAccel_ = 0.0f;
    private float currentMaxSpeed_ = 0.0f;
    private float accelSmoothVel_ = 0.0f;
    private float maxSpeedSmoothVel_ = 0.0f;

    [SerializeField] private float paramReleaseSmoothTime_ = 0.25f;       // 減速側の追従時間（Dodge の尾の長さ）
    [SerializeField] private float paramReleaseMaxSmoothSpeed_ = 100000.0f;   // 追従の最大速度（実質クランプ無し）
}
