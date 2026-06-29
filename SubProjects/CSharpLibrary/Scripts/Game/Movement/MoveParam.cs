using System;

// 1状態ぶんの移動パラメーター（実行時 DTO）。プレイヤー・敵など移動するもの全般で共通。
// シリアライズはされず、各ドライバ（PlayerState / EnemyMoveComponent 等）が
// フラットな SerializeField から構築して Mover に渡す（ネストした SerializeField はエディタに出ないため）。
public class MoveParam {
    // --- 物理パラメータ（Mover が解釈する） ---
    public float accel_                = 256.0f;   // 加速度
    public float maxSpeed_             = 512.0f;   // 最大速度
    public float minSpeed_             = 0.0f;     // 入力中の最低速度。0 にならないので真後ろ入力でも速度が消えず旋回できる
    public float decelSmoothTime_      = 0.2f;     // 減速の平滑化時間
    public float decelMaxSmoothSpeed_  = 12.0f;    // 減速の最大平滑化速度
    public float rotateSmoothTime_     = 3.0f;     // 旋回の平滑化時間（小さいほど機敏に向きを変える）
    public float rotateMaxSmoothSpeed_ = 64.0f;    // 最大旋回レート(rad/s)。小さいほど曲がりにくい（大きいと実質即旋回）

    // --- 入力の意図フラグ（Mover ではなくドライバ/MoveDirector が解釈する） ---
    public bool  canMove_              = true;     // この状態で移動入力を受け付けるか
    public bool  moveForward_          = false;    // true: 入力を無視して正面（forward）へ走り続ける
}
