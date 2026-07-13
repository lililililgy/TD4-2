using System;
using System.Collections.Generic;

public class SeaCow : MonoScript
{
    /* ----- 突進系のパラメータ ----- */
    // 照準中に進行方向へ乗せる加速度
    [SerializeField] private float acceleration = 700.0f;
    // 速度の上限
    [SerializeField] private float maxSpeed = 500.0f;
    // 照準を止めている間、慣性のみで進む際の減衰係数
    [SerializeField] private float drag = 1.5f;
    // これより近づいたら照準の更新を止める
    [SerializeField] private float lockOnRange = 180.0f;
    // 速度がこれを下回ったら照準を再開する
    [SerializeField] private float resumeAimSpeed = 60.0f;

    /* ----- 自爆系のパラメータ ----- */
    // 自爆までの時間
    [SerializeField] private float explotionTime = 0.0f;
    [SerializeField] private Vector4 normalColor = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    [SerializeField] private Vector4 blinkColor = new Vector4(1.0f, 0.0f, 0.0f, 1.0f);
    // 点滅間隔
    [SerializeField] private float blinkIntervalStart = 0.6f;
    [SerializeField] private float blinkIntervalEnd = 0.08f;
    // 赤くなる瞬間に再生するスケールパルスの倍率・合計時間
    [SerializeField] private float blinkPulseScale = 1.3f;
    [SerializeField] private float blinkPulseDuration = 0.15f;

    private enum ChargeState { Aiming, Locked, Decelerating }

    /* ----- 実行時状態 ----- */
    private TargetRangeDetector targetRangeDetector_;
    private SpriteRenderer spriteRenderer_;
    private SpriteAnimation spriteAnimation_;
    private TargetFacingFlip facingFlip_;

    private Vector3 velocity_ = Vector3.zero;
    private ChargeState chargeState_ = ChargeState.Aiming;
    // 状態ごとの更新処理
    private Dictionary<ChargeState, Action<Entity>> chargeUpdateActions_;
    private Action<Entity> chargeUpdateAction_;

    // 攻撃開始か
    private bool isStarted_ = false;

    private float explosionTimer_ = 0.0f;
    private float blinkTimer_ = 0.0f;
    private bool isBlinkOn_ = false;

    // 赤くなる瞬間のスケールパルス
    private Vector3 initialScale_ = Vector3.one;
    private float pulseTimer_ = 0.0f;
    private bool isPulsing_ = false;

    public override void Initialize()
    {
        // スクリプト・コンポーネント取得
        targetRangeDetector_ = entity.GetScript<TargetRangeDetector>();
        spriteRenderer_ = entity.GetComponent<SpriteRenderer>();
        spriteAnimation_ = entity.GetScript<SpriteAnimation>();
        facingFlip_ = entity.GetScript<TargetFacingFlip>();
        // 初期スケールを保持しておく
        initialScale_ = transform.scale;

        // 状態ごとの更新処理を登録
        chargeUpdateActions_ = new Dictionary<ChargeState, Action<Entity>>
        {
            { ChargeState.Aiming,       UpdateAiming },
            { ChargeState.Locked,       UpdateLocked },
            { ChargeState.Decelerating, UpdateDecelerating },
        };
        chargeUpdateAction_ = chargeUpdateActions_[chargeState_];

        // 範囲内に入った瞬間に時限爆弾と突進を開始する
        if (targetRangeDetector_ != null)
        {
            targetRangeDetector_.OnEnter += StartEncounter;
        }
    }

    public override void Update()
    {

        if (transform == null) { return; }

        // 範囲内に一度も入っていなければ、点滅も突進も始めない
        if (!isStarted_) {
            AnimationStop();
            return; 
        }

        // ターゲットの取得
        Entity target = targetRangeDetector_?.Target;
        if (target == null || target.transform == null) { return; }

        // 追いかけている間だけ連番アニメーションを再生する
        AnimationPlay();

        // 現在の突進状態の更新処理を実行
        chargeUpdateAction_?.Invoke(target);

        transform.position += velocity_ * Time.deltaTime;

        // 赤点滅と自爆タイマーの更新
        UpdateBlink();
    }

    // targetRangeDetector_ の範囲内に入った瞬間の開始処理
    private void StartEncounter()
    {
        if (isStarted_) { 
            return;
        }

        // 時限爆弾と突進を開始する
        isStarted_ = true;
        TransitionCharge(ChargeState.Aiming);
        velocity_ = Vector3.zero;
        explosionTimer_ = 0.0f;
        blinkTimer_ = 0.0f;
        isBlinkOn_ = false;
        isPulsing_ = false;
        pulseTimer_ = 0.0f;
        transform.scale = initialScale_;

        // 通常色から始める
        if (spriteRenderer_ != null)
        {
            spriteRenderer_.color = normalColor;
        }
    }

    private void TransitionCharge(ChargeState next)
    {
        chargeState_ = next;
        chargeUpdateAction_ = chargeUpdateActions_[next];
    }

    ///-----------------------------------------------------------------------------
    /// 突進状態ごとの更新処理
    ///-----------------------------------------------------------------------------

    private void UpdateAiming(Entity target)
    {
        // プレイヤー座標そのものへ向けて加速する
        AccelerateTowardTarget(target);

        // 見た目(向き・UV反転)はプレイヤー座標ではなく、実際の進行方向(速度)に合わせる。
        // プレイヤー座標を直接見て向きを決めると、プレイヤーが反対側に回り込んだ瞬間に
        // UV反転が急に切り替わって不自然に見えるため。
        if (velocity_.LengthSq() > 0.0001f)
        {
            facingFlip_?.FaceDirection(velocity_.Normalized());
        }

        // lockOnRange内に入ったら、向きを固定して突っ切る
        if (IsWithinLockOnRange(target))
        {
            TransitionCharge(ChargeState.Locked);
        }
    }

    private void UpdateLocked(Entity target)
    {
        // 向きは固定したまま、そのまま加速し続けて突っ切る
        Accelerate();

        // lockOnRangeの外に出たら減速を始める
        if (!IsWithinLockOnRange(target))
        {
            TransitionCharge(ChargeState.Decelerating);
        }
    }

    private void UpdateDecelerating(Entity target)
    {
        // lockOnRangeの外に出た後は、慣性のみで減速していく
        velocity_ *= (float)Math.Exp(-drag * Time.deltaTime);

        // 再びlockOnRange内に戻ったら向きは固定のまま突っ切りへ
        if (IsWithinLockOnRange(target))
        {
            TransitionCharge(ChargeState.Locked);
        }
        // 十分減速したら、再びプレイヤーへ向き直して突進を再開する
        else if (velocity_.Length() <= resumeAimSpeed)
        {
            TransitionCharge(ChargeState.Aiming);
        }
    }

    private bool IsWithinLockOnRange(Entity target)
    {
        if (target == null || target.transform == null) { return false; }
        Vector3 toTarget = target.transform.position - transform.position;
        return toTarget.Length() <= lockOnRange;
    }

    // プレイヤー座標へ向けて加速する(Aiming中の狙い直し用)
    private void AccelerateTowardTarget(Entity target)
    {
        if (target == null || target.transform == null) { return; }

        Vector3 toTarget = target.transform.position - transform.position;
        if (toTarget.LengthSq() > 0.0001f)
        {
            velocity_ += toTarget.Normalized() * acceleration * Time.deltaTime;
        }
        ClampSpeed();
    }

    // 現在向いている方向(見た目)へそのまま加速する。Locked/Decelerating中、向きを変えずに突っ切る用。
    private void Accelerate()
    {
        Vector3 facingDir = facingFlip_ != null ? facingFlip_.FacingDirection : transform.up;
        velocity_ += facingDir * acceleration * Time.deltaTime;
        ClampSpeed();
    }

    private void ClampSpeed()
    {
        if (velocity_.Length() > maxSpeed)
        {
            // max速度で進む
            velocity_ = velocity_.Normalized() * maxSpeed;
        }
    }

    ///-----------------------------------------------------------------------------
    /// 赤点滅・自爆
    ///-----------------------------------------------------------------------------
    private void UpdateBlink()
    {
        if (spriteRenderer_ == null) { return; }

        // 自爆タイマーが有効なら経過時間を計測
        if (explotionTime > 0.0f)
        {
            explosionTimer_ += Time.deltaTime;
        }

        // 残り時間の割合
        float remainRatio = 1.0f;
        if (explotionTime > 0.0f)
        {
            remainRatio = Mathf.Clamp01(1.0f - explosionTimer_ / explotionTime);
        }

        // 残り時間が迫るほど点滅間隔を短くしていく
        float interval = Mathf.Lerp(blinkIntervalEnd, blinkIntervalStart, remainRatio);

        // 点滅の更新
        blinkTimer_ += Time.deltaTime;
        if (blinkTimer_ >= interval)
        {
            // 点滅の切り替え
            blinkTimer_ = 0.0f;
            isBlinkOn_ = !isBlinkOn_;
            // 点滅色の切り替え
            spriteRenderer_.color = isBlinkOn_ ? blinkColor : normalColor;

            // 赤くなる瞬間にスケールパルスを開始する
            if (isBlinkOn_)
            {
                isPulsing_ = true;
                pulseTimer_ = 0.0f;
            }
        }

        // スケールパルスの更新
        UpdatePulseScale();

        // タイマー満了で自爆。これがUpdate内で最後に触れる自身の状態になるようにする。
        if (explotionTime > 0.0f && explosionTimer_ >= explotionTime)
        {
            entity.Destroy();
        }
    }

    // 赤くなる瞬間に再生する拡縮イージング
    private void UpdatePulseScale()
    {
        if (!isPulsing_) { return; }

        pulseTimer_ += Time.deltaTime;
        float halfDuration = blinkPulseDuration * 0.5f;

        float scale;
        if (pulseTimer_ < halfDuration)
        {
            // 前半: 一気に拡大
            float t = Mathf.Clamp01(pulseTimer_ / halfDuration);
            scale = Mathf.Lerp(1.0f, blinkPulseScale, Ease.Out.Back(t));
        }
        else
        {
            // 後半: 元のスケールへ戻す
            float t = Mathf.Clamp01((pulseTimer_ - halfDuration) / halfDuration);
            scale = Mathf.Lerp(blinkPulseScale, 1.0f, Ease.Out.Back(t));
        }

        transform.scale = initialScale_ * scale;

        if (pulseTimer_ >= blinkPulseDuration)
        {
            isPulsing_ = false;
            transform.scale = initialScale_;
        }
    }

    // isPlayが既にfalseならここで何もしない。SetFrame()はUVTransformを常に正のscaleで
    // 丸ごと上書きするため、毎フレーム呼ぶとTargetFacingFlipのUV反転を潰してしまう。
    void AnimationStop() {
        if (spriteAnimation_ == null || !spriteAnimation_.isPlay) { return; }
        spriteAnimation_.isPlay = false;
    }

    // 追いかけ(isStarted_)中だけ2フレームの連番アニメーションをループ再生する
    void AnimationPlay() {
        if (spriteAnimation_ == null || spriteAnimation_.isPlay) { return; }
        spriteAnimation_.startFrame = 0;
        spriteAnimation_.endFrame = 1;
        spriteAnimation_.isLoop = true;
        spriteAnimation_.isPlay = true;
    }

}
