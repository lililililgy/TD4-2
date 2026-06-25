using System;

// 1状態ぶんの移動パラメーター（実行時 DTO）。プレイヤー・敵など移動するもの全般で共通。
// シリアライズはされず、各ドライバ（PlayerState / EnemyMoveComponent 等）が
// フラットな SerializeField から構築して Mover に渡す（ネストした SerializeField はエディタに出ないため）。
public class MoveParam {
    public bool  canMove_              = true;     // この状態で移動入力を受け付けるか
    public float accel_                = 256.0f;   // 加速度
    public float maxSpeed_             = 512.0f;   // 最大速度
    public float decelSmoothTime_      = 0.2f;     // 減速の平滑化時間
    public float decelMaxSmoothSpeed_  = 12.0f;    // 減速の最大平滑化速度
    public float rotateSmoothTime_     = 3.0f;     // 旋回の平滑化時間
    public float rotateMaxSmoothSpeed_ = 64.0f;    // 旋回の最大平滑化速度
    public bool  moveForward_          = false;    // true: 入力を無視して正面（forward）へ走り続ける
}
