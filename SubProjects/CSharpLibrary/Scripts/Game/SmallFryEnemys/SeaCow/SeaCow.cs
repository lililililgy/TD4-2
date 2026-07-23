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

    /* ----- パーティクル ----- */
    // 常時再生する泳ぎの泡
    [SerializeField] private string swimParticlePrefabName = "enemySwimParticle";
    // 自爆時に再生する爆発エフェクト
    [SerializeField] private string explosionParticlePrefabName = "seaCowExplosion";
    [SerializeField] private float explosionParticleLifeTime = 1.0f;

    /* ----- 自爆前の膨張演出 ----- */
    // 膨らみきるまでにかかる時間
    [SerializeField] private float preExplosionSwellDuration = 1.5f;
    // 最大まで膨らんだ時のスケール倍率
    [SerializeField] private float preExplosionSwellScale = 1.6f;
    // 膨らみきってから実際に爆発するまで、最大サイズを維持する時間
    [SerializeField] private float postSwellHoldDuration = 2.0f;

    /* ----- 実行時状態 ----- */
    // 親の巨大スケールを継承させないよう、親子付けせず座標だけ追従させる泳ぎの泡
    private Entity swimParticleEntity_;
    // 敵単位でパーティクルの発生位置を調整するオフセットスクリプト
    private ParticleOffset particleOffset_;
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

    // 自爆前の膨張倍率(1.0で通常サイズ)
    private float swellMultiplier_ = 1.0f;

    public override void Initialize()
    {
        // スクリプト・コンポーネント取得
        hp_ = entity.GetScript<HP>();
        chaseController_ = entity.GetScript<ChaseController>();
        spriteRenderer_ = entity.GetComponent<SpriteRenderer>();
        spriteAnimation_ = entity.GetScript<SpriteAnimation>();
        facingFlip_ = entity.GetScript<TargetFacingFlip>();
        particleOffset_ = entity.GetScript<ParticleOffset>();

        // 初期スケール・初期速度・初期fpsを保持しておく
        initialScale_ = transform.scale;
        initialChaseSpeed_ = chaseController_ != null ? chaseController_.ChaseSpeed : 0.0f;
        initialAnimationFps_ = spriteAnimation_ != null ? spriteAnimation_.fps : 0.0f;

        // 追跡が始まるたびに加速をリセットする
        if (chaseController_ != null)
        {
            chaseController_.OnChaseStart += OnChaseStart;
        }

        // 常時再生する泳ぎの泡を子エンティティとして生成する
        SpawnSwimParticle();
    }

    public override void Update()
    {
        if (transform == null) { return; }

        // 親子付けしていない泳ぎの泡の座標を追従させる
        SyncSwimParticlePosition();

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
        swellMultiplier_ = 1.0f;
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

        // 自爆直前の膨張演出の更新
        UpdateSwell();

        // スケールパルスの更新
        UpdatePulseScale();

        // タイマー満了で自爆。これがUpdate内で最後に触れる自身の状態になるようにする。
        if (explotionTime > 0.0f && explosionTimer_ >= explotionTime)
        {
            SpawnExplosionParticle();
            entity.Destroy();
        }
    }

    // 自爆が近づくにつれて体を膨らませるイージング
    private void UpdateSwell()
    {
        if (preExplosionSwellDuration <= 0.0f || explotionTime <= 0.0f)
        {
            swellMultiplier_ = 1.0f;
            return;
        }

        float remainingTime = Math.Max(0.0f, explotionTime - explosionTimer_);
        // 爆発直前の保持時間を差し引いた残り時間を基準に膨張率を計算する。
        // これにより、膨らみきった後は爆発するまでの間、最大サイズを維持し続ける。
        float remainingAfterHold = Math.Max(0.0f, remainingTime - postSwellHoldDuration);
        float swellT = Mathf.Clamp01(1.0f - remainingAfterHold / preExplosionSwellDuration);
        swellMultiplier_ = Mathf.Lerp(1.0f, preExplosionSwellScale, Ease.In.Cubic(swellT));

        // パルス中でない間もこの倍率をベーススケールとして反映する
        if (!isPulsing_)
        {
            transform.scale = initialScale_ * swellMultiplier_;
        }
    }

    ///-----------------------------------------------------------------------------
    /// パーティクル
    ///-----------------------------------------------------------------------------
    // 常時再生する泳ぎの泡を生成する
    private void SpawnSwimParticle()
    {
        if (String.IsNullOrEmpty(swimParticlePrefabName)) { return; }

        Entity swimParticle = ecsGroup.CreateEntity(swimParticlePrefabName);
        if (!swimParticle) { return; }

        // SeaCowはスケールが極端に大きいため、親子付けすると泡もつられて巨大化し画面外に出てしまう。
        // そのため親子付けはせず、座標だけを毎フレーム追従させる。
        swimParticle.transform.position = transform.position + GetParticleOffset();
        swimParticleEntity_ = swimParticle;
    }

    // 泳ぎの泡の座標をこの敵の現在位置に追従させる
    private void SyncSwimParticlePosition()
    {
        if (!swimParticleEntity_) { return; }
        swimParticleEntity_.transform.position = transform.position + GetParticleOffset();
    }

    // ParticleOffsetスクリプトが付いていればそのオフセットを、無ければゼロを返す。
    // TargetFacingFlipはUVのX反転で左右(ミラー)を表現しつつ、transform.rotateで泳ぎの傾きも付けている。
    // そのためオフセットはまずUV反転に合わせてX成分をミラーし、その後に現在の傾き(transform.rotate)を掛ける。
    private Vector3 GetParticleOffset()
    {
        if (particleOffset_ == null) { return Vector3.zero; }

        Vector3 offset = particleOffset_.offset;
        bool isFacingRight = facingFlip_ == null || facingFlip_.IsFacingRight;
        offset.x = isFacingRight ? offset.x : -offset.x;
        return transform.rotate * offset;
    }

    public override void OnDestroy()
    {
        if (swimParticleEntity_)
        {
            swimParticleEntity_.Destroy();
            swimParticleEntity_ = null;
        }
    }

    // 自爆時の爆発エフェクトを現在位置に生成する
    private void SpawnExplosionParticle()
    {
        if (String.IsNullOrEmpty(explosionParticlePrefabName)) { return; }

        Entity explosion = ecsGroup.CreateEntity(explosionParticlePrefabName);
        if (!explosion) { return; }

        explosion.transform.position = transform.position + GetParticleOffset();

        TimedDestruction timedDestruction = explosion.AddScript<TimedDestruction>();
        timedDestruction.lifeTime = explosionParticleLifeTime;
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

        transform.scale = initialScale_ * swellMultiplier_ * scale;

        if (pulseTimer_ >= blinkPulseDuration)
        {
            isPulsing_ = false;
            transform.scale = initialScale_ * swellMultiplier_;
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
