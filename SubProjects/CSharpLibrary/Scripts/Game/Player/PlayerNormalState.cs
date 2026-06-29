using System;

// 通常状態。自分の移動パラメーターを SerializeField で持ち、射撃可能。
// 固有の挙動は無く、遷移は PlayerStateComponent の遷移テーブルで制御される。
public class PlayerNormalState : PlayerState {

    [SerializeField] private bool  canMove_              = true;
    [SerializeField] private float accel_                = 256.0f;
    [SerializeField] private float maxSpeed_             = 512.0f;
    [SerializeField] private float minSpeed_             = 0.0f;
    [SerializeField] private float decelSmoothTime_      = 0.2f;
    [SerializeField] private float decelMaxSmoothSpeed_  = 12.0f;
    [SerializeField] private float rotateSmoothTime_     = 0.12f; // 旋回の効き（小さいほど機敏）
    [SerializeField] private float rotateMaxSmoothSpeed_ = 8.0f;  // 最大旋回レート(rad/s)。小さいほど曲がりにくい

    private MoveParam moveParam_ = new MoveParam();

    public override void Initialize() {
        moveParam_ = new MoveParam {
            canMove_              = canMove_,
            accel_                = accel_,
            maxSpeed_             = maxSpeed_,
            minSpeed_             = minSpeed_,
            decelSmoothTime_      = decelSmoothTime_,
            decelMaxSmoothSpeed_  = decelMaxSmoothSpeed_,
            rotateSmoothTime_     = rotateSmoothTime_,
            rotateMaxSmoothSpeed_ = rotateMaxSmoothSpeed_,
        };
    }

    public override MoveParam MoveParam {
        get { return moveParam_; }
    }
}
