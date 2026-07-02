using System;

// ダッシュ状態。移動パラメーターと dash 固有値（継続時間・クールダウン）を SerializeField で持ち、
// dash ロジック（開始入力・終了判定・クールダウン・射撃不可）を担う。
public class PlayerDashState : PlayerState {

    [SerializeField] private bool  canMove_              = true;
    [SerializeField] private bool  moveForward_          = false; // true: 入力を無視して正面へ走り続ける（ドッジ）。false: 入力方向へ普通に移動
    [SerializeField] private float accel_                = 4096.0f;
    [SerializeField] private float maxSpeed_             = 1024.0f;
    [SerializeField] private float decelSmoothTime_      = 0.2f;
    [SerializeField] private float decelMaxSmoothSpeed_  = 12.0f;
    [SerializeField] private float rotateSmoothTime_     = 0.12f; // 旋回の効き（※moveForward_中は旋回しないので無効）
    [SerializeField] private float rotateMaxSmoothSpeed_ = 8.0f;  // 最大旋回レート(rad/s)（※同上）


    private MoveParam moveParam_ = new MoveParam();

    public override void Initialize() {
        moveParam_ = new MoveParam {
            canMove_              = canMove_,
            accel_                = accel_,
            maxSpeed_             = maxSpeed_,
            decelSmoothTime_      = decelSmoothTime_,
            decelMaxSmoothSpeed_  = decelMaxSmoothSpeed_,
            rotateSmoothTime_     = rotateSmoothTime_,
            rotateMaxSmoothSpeed_ = rotateMaxSmoothSpeed_,
            moveForward_          = moveForward_,
        };
    }

    public override MoveParam MoveParam {
        get { return moveParam_; }
    }

    // 通常 → ダッシュ の遷移条件
    public bool WantsToStart() {
        PlayerInputComponent input = entity.GetScript<PlayerInputComponent>();
        return input != null && input.IsDashButtonPressed;
    }

    // ダッシュ → 通常 の遷移条件
    public bool IsFinished() {
        PlayerInputComponent input = entity.GetScript<PlayerInputComponent>();
        return input != null && !input.IsDashButtonPressed;
    }

    // ダッシュ中は射撃不可
    public override bool CanShoot() {
        return false;
    }
}
