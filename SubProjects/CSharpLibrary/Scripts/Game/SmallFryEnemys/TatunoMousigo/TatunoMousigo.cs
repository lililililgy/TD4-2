using System;


public class TatunoMousigo : MonoScript
{

    /* ----- ターゲット ----- */
    [SerializeField] private string targetEntityName = "Player";

    /* ----- 横移動 ----- */
    [SerializeField] private float moveSpeed = 330.0f;    // 追いかける速さ
    [SerializeField] private float stepDistance = 110.0f; // 着地フレームのたびに進む距離
    [SerializeField] private float turnDelay = 0.35f;     // 逆方向へ切り替わるまで待つ時間

    /* ----- BaseYの微調整 ----- */
    [SerializeField] private float baseYOffset = 0.0f;

    /* ----- ジャンプ判定 ----- */
    // ジャンプトリガーのX距離とY高さ
    [SerializeField] private float jumpTriggerDistance = 80.0f;
    [SerializeField] private float jumpTriggerHeight = 20.0f;
    // // ジャンプの最高到達高度
    [SerializeField] private float jumpHeight = 120.0f;
    // 時間関連
    [SerializeField] private float jumpDuration = 0.6f;
    [SerializeField] private float jumpCooldown = 1.5f;

    /* ----- カメラシェイク ----- */
    [SerializeField] private string cameraEntityName = "MainCamera";
    // 振幅はワールド単位なので数百オーダーになる(プレハブ側で設定)
    [SerializeField] private float moveShakeIntensity = 2.0f;
    [SerializeField] private float landShakeIntensity = 8.0f;
    [SerializeField] private float moveShakeDuration  = 0.12f;
    [SerializeField] private float landShakeDuration  = 0.25f;
    [SerializeField] private float shakeFrequency     = 28.0f;

    /* ----- パーティクル ----- */
    // 歩行時・着地時に再生する土埃
    [SerializeField] private string dustParticlePrefabName = "dustParticle";
    [SerializeField] private float dustParticleLifeTime = 0.6f;

    private enum JumpState { None, Jumping }

    /* ----- 実行時状態 ----- */
    private Entity targetEntity_;
    private SpriteRenderer spriteRenderer_;
    private TargetRangeDetector rangeDetector_;
    private CameraShake cameraShake_;
    private SpriteAnimation spriteAnimation_;
    // 敵単位でパーティクルの発生位置を調整するオフセットスクリプト
    private ParticleOffset particleOffset_;
    private RigidbodyMotion motion_ = new RigidbodyMotion(6.0f);

    // 歩きアニメーションのフレーム範囲
    private int walkStartFrame = 0; // 停止ポーズ
    private int walkEndFrame = 2; // 着地ポーズ
    private int stepFrame = 2; // このフレームになった瞬間に1歩進める

    // 通常時に維持するY座標
    private float baseY_ = 0.0f;

    // 横移動
    private int moveDir_ = 0;
    private float turnDelayTimer_ = 0.0f;

    // 振り向き
    private int facingDir_ = 1; // 確定している向き

    // ジャンプ
    private JumpState jumpState_ = JumpState.None;
    private float jumpTimer_ = 0.0f;
    private float jumpYOffset_ = 0.0f;
    private float jumpCooldownTimer_ = 0.0f;

    private const float kDirEpsilon = 1.0f;

    public override void Initialize()
    {
        motion_.Attach(entity);

        // Entity,Component,Scriptの取得
        targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        spriteRenderer_ = entity.GetComponent<SpriteRenderer>();
        rangeDetector_ = entity.GetScript<TargetRangeDetector>();
        spriteAnimation_ = entity.GetScript<SpriteAnimation>();
        particleOffset_ = entity.GetScript<ParticleOffset>();

        Entity cameraEntity = ecsGroup.FindEntity(cameraEntityName);
        cameraShake_ = cameraEntity != null ? cameraEntity.GetScript<CameraShake>() : null;

        baseY_ = transform.position.y + baseYOffset;

        // 着地フレームに乗ったタイミングで1歩進める
        if (spriteAnimation_ != null)
        {
            spriteAnimation_.OnFrameChanged += OnWalkFrameChanged;
        }
    }

    public override void Update()
    {
        if (!TryFindTarget())
        {
            return;
        }

        UpdateJump();
        UpdateDirection();

        // 横方向だけ Rigidbody2D 経由で連続移動する。Y はこの後 ApplyY() が baseY_ に固定する。
        motion_.Apply(transform, new Vector3(moveDir_ * moveSpeed, 0.0f, 0.0f));

        UpdateWalkAnimation();
        ApplyY();
        UpdateTurnUV();
    }

  
    private void UpdateDirection()
    {
        bool inRange = rangeDetector_ == null || rangeDetector_.IsInRange;
        float toTargetX = inRange ? targetEntity_.transform.position.x - transform.position.x : 0.0f;
        int desiredDir = toTargetX > kDirEpsilon ? 1 : (toTargetX < -kDirEpsilon ? -1 : 0);

        if (desiredDir == 0)
        {
            // ターゲット方向なし
            moveDir_ = 0;
            turnDelayTimer_ = 0.0f;
            return;
        }

        if (moveDir_ == 0 || desiredDir == moveDir_)
        {
            // 静止からの始動、または現在の進行方向と同じ
            moveDir_ = desiredDir;
            turnDelayTimer_ = 0.0f;
            return;
        }

        // turnDelay 待ってからそのまま向きを切り替える
        turnDelayTimer_ += Time.deltaTime;
        if (turnDelayTimer_ >= turnDelay)
        {
            moveDir_ = desiredDir;
            turnDelayTimer_ = 0.0f;
        }
    }

    private void OnWalkFrameChanged(int frame)
    {
        if (frame != stepFrame || moveDir_ == 0)
        {
            return;
        }

        Vector3 pos = transform.position;
        pos.x += moveDir_ * stepDistance;
        transform.position = pos;

        //cameraShake_?.Shake(moveShakeIntensity);
        cameraShake_?.Shake(moveShakeDuration, moveShakeIntensity, shakeFrequency);
        SpawnDustParticle();
    }

 
    private void UpdateWalkAnimation()
    {
        if (spriteAnimation_ == null)
        {
            return;
        }

        bool isWalking = moveDir_ != 0;
        if (isWalking)
        {
            if (!spriteAnimation_.isPlay)
            {
                spriteAnimation_.startFrame = walkStartFrame;
                spriteAnimation_.endFrame = walkEndFrame;
                spriteAnimation_.isLoop = true;
                spriteAnimation_.isPlay = true;
            }
            spriteAnimation_.fps = CalcWalkFps();
        }
        else if (spriteAnimation_.isPlay)
        {
            spriteAnimation_.isPlay = false;
            spriteAnimation_.SetFrame(walkStartFrame);
        }
    }

    private float CalcWalkFps()
    {
        int framesPerLoop = walkEndFrame - walkStartFrame + 1;
        if (framesPerLoop <= 0 || stepDistance <= 0.0f)
        {
            return 0.0f;
        }
        return moveSpeed * framesPerLoop / stepDistance;
    }

    /// ==============================================================================
    ///  振り向き
    /// ==============================================================================

    private void UpdateTurnUV()
    {
        if (spriteRenderer_ == null || spriteAnimation_ == null || spriteAnimation_.cols <= 0)
        {
            return;
        }

        int cols = spriteAnimation_.cols;
        float baseScaleX = 1.0f / cols;

        UVTransform uv = spriteRenderer_.uvTransform;

        if (moveDir_ == 0)
        {
            // 停止中はUV位置を(0,0)に固定する
            uv.position.x = 0.0f;
            uv.position.y = 0.0f;
            uv.scale.x = baseScaleX;
            spriteRenderer_.uvTransform = uv;
            return;
        }

        facingDir_ = moveDir_;

        int colIndex = spriteAnimation_.CurrentFrame % cols;
        float baseU = colIndex * baseScaleX;

        // 元絵は左向きなので、右を向く時だけUVを反転する
        bool isRight = facingDir_ > 0;

        uv.scale.x = isRight ? -baseScaleX : baseScaleX;
        uv.position.x = isRight ? baseU + baseScaleX : baseU;
        spriteRenderer_.uvTransform = uv;
    }

    /// ==============================================================================
    ///  ジャンプ
    /// ==============================================================================

    private void UpdateJump()
    {
        if (jumpCooldownTimer_ > 0.0f)
        {
            jumpCooldownTimer_ -= Time.deltaTime;
        }

        if (jumpState_ == JumpState.None)
        {
            jumpYOffset_ = 0.0f;
            if (jumpCooldownTimer_ <= 0.0f && ShouldJump())
            {
                jumpState_ = JumpState.Jumping;
                jumpTimer_ = 0.0f;
            }
            return;
        }

        jumpTimer_ += Time.deltaTime;
        float t = Mathf.Clamp01(jumpTimer_ / jumpDuration);

        // サインカーブで跳び上がって降りてくる
        jumpYOffset_ = Mathf.Sin(t * Mathf.PI) * jumpHeight;

        if (t >= 1.0f)
        {
            // 着地
            jumpYOffset_ = 0.0f;
            jumpState_ = JumpState.None;
            jumpCooldownTimer_ = jumpCooldown;
            //cameraShake_?.Shake(landShakeIntensity);
            cameraShake_?.Shake(landShakeDuration, landShakeIntensity, shakeFrequency);
            SpawnDustParticle();
        }
    }

    // 歩行・着地時の土埃エフェクトを足元に生成する
    private void SpawnDustParticle()
    {
        if (String.IsNullOrEmpty(dustParticlePrefabName)) { return; }

        Entity dust = ecsGroup.CreateEntity(dustParticlePrefabName);
        if (!dust) { return; }

        // TatunoMousigoはtransform.rotateを回転させず、UVのX反転(facingDir_)だけで左右を表現しているため、
        // オフセットも回転ではなくfacingDir_でX成分をミラーさせる。
        Vector3 offset = particleOffset_ != null ? particleOffset_.offset : Vector3.zero;
        Vector3 pos = transform.position;
        pos.x += offset.x * facingDir_;
        pos.y = baseY_ + offset.y;
        pos.z += offset.z;
        dust.transform.position = pos;

        TimedDestruction timedDestruction = dust.AddScript<TimedDestruction>();
        timedDestruction.lifeTime = dustParticleLifeTime;
    }

    private bool ShouldJump()
    {
        bool inRange = rangeDetector_ == null || rangeDetector_.IsInRange;
        if (!inRange)
        {
            return false;
        }

        Vector3 toTarget = targetEntity_.transform.position - transform.position;
        bool isClose = Mathf.Abs(toTarget.x) <= jumpTriggerDistance;
        bool isAbove = toTarget.y >= jumpTriggerHeight;
        return isClose && isAbove;
    }

    /// ==============================================================================
    ///  共通
    /// ==============================================================================

    private void ApplyY()
    {
        Vector3 pos = transform.position;
        pos.y = baseY_ + jumpYOffset_;
        transform.position = pos;
    }

    private bool TryFindTarget()
    {
        if (targetEntity_ == null)
        {
            targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        }
        return targetEntity_ != null;
    }
}
