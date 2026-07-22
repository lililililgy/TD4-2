using System;

// 汎用移動コア（素のクラス。コンポーネントではない）。物理のみを担当する。
// プレイヤー・敵などのドライバが1つ保持し、毎フレーム Move を呼ぶ。
// 「進みたい最終的な方向(desiredDir)」と「移動パラメータ(MoveParam)」を渡すと、
// 加速・最大速度クランプ・減速・旋回・パラメータのアニメーションを行い Transform を動かす。
//
// 入力の「解釈」（移動可否・正面固定走り・正面方向の記録など）はここでは行わない。
// それらはドライバ側（例: PlayerMoveComponent + MoveDirector）で解決し、
// 結果の方向だけを desiredDir として渡すこと。長さ0で停止（減速）扱い。
//
// 速度は Rigidbody2D にも書き出す（他システムから velocity を読めるようにするため）。
// ただし position の更新は従来どおりこのクラスが行う。エンジンの Rigidbody2DUpdateSystem は
// velocity への重力・軸固定の適用しかしておらず、位置の積分を行わないため、
// Rigidbody2D に movement を委ねると何も動かなくなる。
// Rigidbody2D 側で積分するようになったら、ここの transform.position 更新を外すこと。
public class Mover {

    // paramRelease* : 移動パラメータ(accel/maxSpeed)を目標値へ下げるときの追従設定。
    //   上がる時は即時（一瞬の加速）、下がる時はこの平滑化で徐々に（ダッシュの尾）。
    public Mover(float paramReleaseSmoothTime, float paramReleaseMaxSmoothSpeed) {
        paramReleaseSmoothTime_ = paramReleaseSmoothTime;
        paramReleaseMaxSmoothSpeed_ = paramReleaseMaxSmoothSpeed;
    }

    // desiredDir: 進みたい最終的な方向（正規化不要。長さ0で停止扱い）。解釈はドライバ側で済ませる。
    // rigidbody: 速度の書き出し先。持たないエンティティ（多くの敵）は null でよい。
    public void Move(Transform transform, Rigidbody2D rigidbody, Vector2 desiredDir, MoveParam param) {
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
            } else {
                velocity_ = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
                    velocity_, Vector3.zero, ref currentDecelSmoothSpeed_, param.decelSmoothTime_, Time.deltaTime, param.decelMaxSmoothSpeed_);
            }
        } else {
            // 入力（スティック）から「目標角度」を求める。角度は +Y 軸から +X 側へ測る（描画の cullRoll と同じ規約）。
            float targetHeading = Mathf.Atan2(desiredDir.x, desiredDir.y);

            // 「現在の向き」を「目標角度」へ、フレームごとに一定角速度で回す（角度補間）。
            // 反対を向いた時でも velocity を 0 にせず、旋回しながら推進し続ける。
            if (!headingInitialized_) {
                heading_ = targetHeading; // 初回入力はその方向にスナップ（0 からの空回りを防ぐ）
                headingInitialized_ = true;
            } else {
                float diff = WrapPi(targetHeading - heading_);           // 最短回転方向の角度差
                float maxStep = param.rotateMaxSmoothSpeed_ * Time.deltaTime; // このフレームで回せる最大角(rad)
                heading_ = WrapPi(heading_ + Mathf.Clamp(diff, -maxStep, maxStep));
            }

            // 「現在の向き」に基づいた推進力を「現在の速度ベクトル」に加算する（ベクトル加算）。
            Vector3 headingDir = new Vector3(Mathf.Sin(heading_), Mathf.Cos(heading_), 0.0f);
            velocity_ += headingDir * (currentAccel_ * Time.deltaTime);

            if (velocity_.LengthSq() > currentMaxSpeed_ * currentMaxSpeed_) {
                velocity_ = velocity_.Normalized() * currentMaxSpeed_;
            }
        }

        // 機体は「現在の向き」を向く。初回入力前は velocity 由来で補完（heading 未確定のため）。
        float cullRoll = headingInitialized_ ? heading_ : Mathf.Atan2(velocity_.x, velocity_.y);
        transform.rotate = Quaternion.MakeFromAxis(Vector3.back, cullRoll);

        transform.position += velocity_ * Time.deltaTime;

        // 確定した速度を Rigidbody2D へ書き出す。setter が毎回 PushBatch() で native を叩くので、
        // 途中経過は書かずにフレームの最後に1回だけ書く。
        if (rigidbody != null) {
            rigidbody.velocity = new Vector2(velocity_.x, velocity_.y);
        }
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

    // 現在の向き（rad）。+Y 軸から +X 側へ測る。一定角速度で目標角度へ旋回する。
    private float heading_ = 0.0f;
    private bool headingInitialized_ = false;

    // 移動パラメーターのアニメーション（current を 目標 next へ追従させる）
    private float currentAccel_ = 0.0f;
    private float currentMaxSpeed_ = 0.0f;

    private float accelSmoothVel_ = 0.0f;
    private float maxSpeedSmoothVel_ = 0.0f;

    private readonly float paramReleaseSmoothTime_;
    private readonly float paramReleaseMaxSmoothSpeed_;
}
