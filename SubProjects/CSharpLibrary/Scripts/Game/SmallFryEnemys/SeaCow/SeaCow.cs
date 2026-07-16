using System;

public class SeaCow : MonoScript
{
    // 追跡中の加速系パラメータ
    // 追跡速度の上限
    [SerializeField] private float maxChaseSpeed = 900.0f;
    // アニメーションfpsの上限
    [SerializeField] private float maxAnimationFps = 24.0f;
    // 初期値からmaxまでイージングし終えるまでの時間
    [SerializeField] private float rampUpEaseTime = 3.0f;

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

    /* ----- 実行時状態 ----- */
    private HP hp_;
    private ChaseController chaseController_;
    private SpriteRenderer spriteRenderer_;
    private SpriteAnimation spriteAnimation_;
    private TargetFacingFlip facingFlip_;

    private float initialChaseSpeed_ = 0.0f;
    private float initialAnimationFps_ = 0.0f;
    // 追跡開始からの経過時間
    private float rampUpTimer_ = 0.0f;

    // 追跡開始か
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
        hp_ = entity.GetScript<HP>();
        chaseController_ = entity.GetScript<ChaseController>();
        spriteRenderer_ = entity.GetComponent<SpriteRenderer>();
        spriteAnimation_ = entity.GetScript<SpriteAnimation>();
        facingFlip_ = entity.GetScript<TargetFacingFlip>();

        // 初期スケール・初期速度・初期fpsを保持しておく
        initialScale_ = transform.scale;
        initialChaseSpeed_ = chaseController_ != null ? chaseController_.ChaseSpeed : 0.0f;
        initialAnimationFps_ = spriteAnimation_ != null ? spriteAnimation_.fps : 0.0f;

        // 追跡が始まるたびに加速をリセットする
        if (chaseController_ != null)
        {
            chaseController_.OnChaseStart += OnChaseStart;
        }
    }

    public override void Update()
    {
        if (transform == null) { return; }

        // 死んだら更新スキップ
        if (hp_ != null && hp_.CurrentHp <= 0) { return; }

        // 実際の進行方向(速度)を向く。
        Vector3 velocity = chaseController_ != null ? chaseController_.Velocity : Vector3.zero;
        if (velocity.LengthSq() > 0.0001f)
        {
            facingFlip_?.FaceDirection(velocity.Normalized());
        }

        // 追いかけている間だけ連番アニメーションを再生する
        if (IsChasing())
        {
            AnimationPlay();
        }
        else
        {
            AnimationStop();
        }

        // 一度追跡が始まったら、以降は赤点滅・自爆タイマー・速度とfpsの加速を回し続ける
        if (isStarted_)
        {
            UpdateBlink();
            UpdateChaseRampUp();
        }
    }

    private bool IsChasing()
    {
        if (chaseController_ == null) { return false; }
        return chaseController_.CurrentState == ChaseController.State.Chase
            || chaseController_.CurrentState == ChaseController.State.Rush;
    }

    // ChaseControllerが追跡状態に入るたびに呼ばれる
    private void OnChaseStart()
    {
        // 自爆タイマーと点滅は初回の追跡開始時にだけ起動する
        if (isStarted_) { return; }

        isStarted_ = true;
        explosionTimer_ = 0.0f;
        rampUpTimer_ = 0.0f;
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

  
    private void UpdateChaseRampUp()
    {
        rampUpTimer_ += Time.deltaTime;

        float t = rampUpEaseTime > 0.0f ? Mathf.Clamp01(rampUpTimer_ / rampUpEaseTime) : 1.0f;
        float easedT = Ease.InOut.Back(t);

        if (chaseController_ != null)
        {
            chaseController_.ChaseSpeed = Mathf.Lerp(initialChaseSpeed_, maxChaseSpeed, easedT);
        }

        if (spriteAnimation_ != null)
        {
            spriteAnimation_.fps = Mathf.Lerp(initialAnimationFps_, maxAnimationFps, easedT);
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

    private void AnimationStop()
    {
        if (spriteAnimation_ == null || !spriteAnimation_.isPlay) { return; }
        spriteAnimation_.isPlay = false;
    }

    // 追いかけ中だけ2フレームの連番アニメーションをループ再生する
    private void AnimationPlay()
    {
        if (spriteAnimation_ == null || spriteAnimation_.isPlay) { return; }
        spriteAnimation_.startFrame = 0;
        spriteAnimation_.endFrame = 1;
        spriteAnimation_.isLoop = true;
        spriteAnimation_.isPlay = true;
    }
}
