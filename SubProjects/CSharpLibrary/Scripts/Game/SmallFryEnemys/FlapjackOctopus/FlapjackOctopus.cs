using System;
using System.Collections.Generic;

public class FlapjackOctopus : MonoScript
{

    /* ----- パラメータ ----- */
    [SerializeField] private float chaseInterval = 1.0f;
    [SerializeField] private float chasePower = 5.0f; // 蹴り出し初速
    [SerializeField] private float chaseDrag = 3.0f; // 水の抵抗による速度減衰の強さ
    [SerializeField] private float chaseTime = 0.5f;
    // chasePowerに対する比率
    [SerializeField] private float inertiaStopRatio = 0.1f;
    // charge
    [SerializeField] private float chargeTime = 5.0f;
    // animation
    [SerializeField] private float chargeScaleAnimationTime = 0.5f;
    [SerializeField] private float chargeScaleRate = 0.5f;
    [SerializeField] private float chaseScaleAnimationTime = 0.5f;
    [SerializeField] private float chaseScaleRate = 0.5f;
    [SerializeField] private int chaseScaleAnimationCount = 5;

    [SerializeField] private string targetEntityName = "Player";

    private enum State { Wait, Charge, Chase }

    // アニメーションフレーム番号
    private const int IdleFrameStart = 0; // 1番目
    private const int IdleFrameEnd = 1; // 2番目
    private const int ChargeFrame = 2; // 3番目
    private const int ChaseFrame = 3; // 4番目

    /* ----- 実行時状態 ----- */
    private State state_ = State.Wait;
    // ターゲットEntity
    private Entity targetEntity_;
    // スクリプト
    private SpriteAnimation spriteAnimation_;
    private TargetRangeDetector rangeDetector_;
    // 初期スケール、速度
    private Vector3 initialScale_;
    private Vector3 velocity_ = Vector3.zero;
    // 状態遷移タイマー
    private float stateTimer_ = 0.0f;
    // 待機中のアイドルアニメーションが再生中かどうか
    private bool isIdleAnimationActive_ = false;
    // 現在の状態に応じた更新処理
    private Action stateUpdateAction_;
    // 状態ごとの突入処理
    private Dictionary<State, Action> enterActions_;

    public override void Initialize()
    {

        // 初期スケールを保持しておく
        initialScale_ = transform.scale;
        // スクリプト取得
        spriteAnimation_ = entity.GetScript<SpriteAnimation>();
        rangeDetector_ = entity.GetScript<TargetRangeDetector>();

        // 状態ごとの突入処理を登録
        enterActions_ = new Dictionary<State, Action>
        {
            { State.Wait,   EnterWait },
            { State.Charge, EnterCharge },
            { State.Chase,  EnterChase },
        };

        // 初期状態は待機
        TransitionTo(State.Wait);
    }

    public override void Update()
    {
        // ターゲット未検出なら何もしない
        if (!TryFindTarget()) { return; }

        // プレイヤー方向を向くのは、待機とチャージ中のみ
        bool isTrulyWaiting = state_ == State.Wait && isIdleAnimationActive_;
        if (isTrulyWaiting || state_ == State.Charge)
        {
            FaceTarget();
        }

        // 蹴りで得た慣性を抵抗で減衰させながら移動し続ける
        ApplyInertiaMovement();

        // 状態ごとの更新処理
        stateUpdateAction_?.Invoke();
    }

    private void TransitionTo(State next)
    {
        // 次のState
        state_ = next;
        stateTimer_ = 0.0f;

        enterActions_[next]?.Invoke();
    }

    private void EnterWait()
    {
        transform.scale = initialScale_;
        // まだ勢いが残っていればキックのフレームを維持し、収まっていればアイドルへ
        UpdateWaitAnimation();
        stateUpdateAction_ = UpdateWait;
    }

    private void EnterCharge()
    {
        // 溜め中は慣性も含めて完全に停止する
        velocity_ = Vector3.zero;
        SetStaticFrame(ChargeFrame);
        stateUpdateAction_ = UpdateCharge;
    }

    private void EnterChase()
    {
        // 今向いている方向（＝溜めで固めた進行方向）へ一気に蹴り出す
        velocity_ = transform.up * chasePower;
        SetStaticFrame(ChaseFrame);
        stateUpdateAction_ = UpdateChase;
    }

    ///-----------------------------------------------------------------------------
    /// アニメーション関連
    ///-----------------------------------------------------------------------------
    // 待機中: 1,2番目のフレームをループ再生
    private void PlayIdleAnimation()
    {
        isIdleAnimationActive_ = true;

        if (spriteAnimation_ == null) { return; }

        spriteAnimation_.startFrame = IdleFrameStart;
        spriteAnimation_.endFrame = IdleFrameEnd;
        spriteAnimation_.isLoop = true;
        spriteAnimation_.isPlay = true;
        spriteAnimation_.ResetAnimation();
    }

    // 溜め・突進中、または待機中でも慣性が残っている間: 指定フレームで静止
    private void SetStaticFrame(int frameIndex)
    {
        isIdleAnimationActive_ = false;

        if (spriteAnimation_ == null) { return; }

        spriteAnimation_.isPlay = false;
        spriteAnimation_.startFrame = frameIndex;
        spriteAnimation_.endFrame = frameIndex;
        spriteAnimation_.SetFrame(frameIndex);
    }

    // 待機中の慣性残量に応じたアニメーション切り替え
    // 慣性(速度)が chasePower * inertiaStopRatio を下回るまでは、キックのフレーム(連番3 = ChaseFrame)を維持する
    private void UpdateWaitAnimation()
    {
        float threshold = Mathf.Abs(chasePower) * Mathf.Clamp01(inertiaStopRatio);
        bool hasResidualInertia = velocity_.Length() > threshold;

        if (hasResidualInertia)
        {
            if (isIdleAnimationActive_)
            {
                SetStaticFrame(ChaseFrame);
            }
        }
        else if (!isIdleAnimationActive_)
        {
            PlayIdleAnimation();
        }
    }

    private void UpdateWait()
    {
        // 慣性が収まるまではキックのフレームを維持し、収まったらアイドルへ切り替える
        UpdateWaitAnimation();

        // まだ伸びている（連番3のまま慣性が残っている）間は待機としてカウントしない
        if (!isIdleAnimationActive_)
        {
            stateTimer_ = 0.0f;
            return;
        }

        // ターゲットが範囲外なら追いかけずに待機を続ける
        if (rangeDetector_ != null && !rangeDetector_.IsInRange)
        {
            stateTimer_ = 0.0f;
            return;
        }

        // 完全に止まってから待機時間が経過したら次の溜めへ
        stateTimer_ += Time.deltaTime;
        if (stateTimer_ >= chaseInterval)
        {
            TransitionTo(State.Charge);
        }
    }

    private void UpdateCharge()
    {
        stateTimer_ += Time.deltaTime;

        // 溜め中は移動せずその場に停止する（position は変更しない）

        // 縮こまるように体を溜めるスケールアニメーション
        float scaleT = Mathf.Clamp01(stateTimer_ / chargeScaleAnimationTime);
        float scale = Mathf.Lerp(1.0f, chargeScaleRate, Ease.Out.Back(scaleT));
        transform.scale = initialScale_ * scale;

        if (stateTimer_ >= chargeTime)
        {
            TransitionTo(State.Chase);
        }
    }

    private void UpdateChase()
    {
        stateTimer_ += Time.deltaTime;

        // 蹴伸びの「勢いある蹴り」を繰り返すパルスアニメーション
        UpdateChaseScalePulse();

        if (stateTimer_ >= chaseTime)
        {
            transform.scale = initialScale_;
            TransitionTo(State.Wait);
        }
    }

    // 水の抵抗を受けて速度が指数関数的に減衰していく慣性移動
    // dv/dt = -chaseDrag * v  →  v(t+dt) = v(t) * e^(-chaseDrag * dt)
    // 状態を問わず毎フレーム適用することで、Chase を抜けた後も勢いがじわっと残る
    private void ApplyInertiaMovement()
    {
        if (velocity_.LengthSq() <= 0.0001f) { return; }

        float drag = chaseDrag > 0.0001f ? chaseDrag : 0.0001f;
        velocity_ *= (float)Math.Exp(-drag * Time.deltaTime);
        transform.position += velocity_ * Time.deltaTime;
    }

    private void UpdateChaseScalePulse()
    {
        if (chaseScaleAnimationCount <= 0 || chaseScaleAnimationTime <= 0.0f)
        {
            transform.scale = initialScale_;
            return;
        }

        // 指定回数分の蹴りを終えたら、元のスケールで突進を続ける
        float totalPulseTime = chaseScaleAnimationTime * chaseScaleAnimationCount;
        if (stateTimer_ >= totalPulseTime)
        {
            transform.scale = initialScale_;
            return;
        }

        // 1回分の蹴りアニメーションの周期内での経過時間を求める
        float cycleTime = stateTimer_ % chaseScaleAnimationTime;
        float pulseT = Mathf.Clamp01(cycleTime / chaseScaleAnimationTime);

        float scale;
        if (pulseT < 0.5f)
        {
            // 前半: 勢いよく蹴り出す（膨張）
            scale = Mathf.Lerp(1.0f, chaseScaleRate, Ease.Out.Back(pulseT / 0.5f));
        }
        else
        {
            // 後半: 元のスケールへ戻す
            scale = Mathf.Lerp(chaseScaleRate, 1.0f, Ease.Out.Back((pulseT - 0.5f) / 0.5f));
        }

        transform.scale = initialScale_ * scale;
    }

    private bool TryFindTarget()
    {
        // 未キャッシュなら検索
        if (targetEntity_ == null)
        {
            targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        }

        return targetEntity_ != null;
    }

    // プレイヤー方向へ緩やかに向きを合わせる
    private void FaceTarget()
    {
        Vector3 toTarget = targetEntity_.transform.position - transform.position;
        if (toTarget.LengthSq() <= 0.0001f) { return; }

        Quaternion targetRotate = Quaternion.LookRotation(-Vector3.forward, toTarget.Normalized());
        transform.rotate = Quaternion.Slerp(transform.rotate, targetRotate, 0.3f);
    }

}
