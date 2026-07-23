using System;

public class SpikeFish : MonoScript
{
    [SerializeField] private float expandanimationDuration = 1.0f;
    [SerializeField] private float expandScaleRate = 1.0f;
    [SerializeField] private float returnanimationDuration = 1.0f;

    /* ----- 実行時状態 ----- */
    private SpriteRenderer spriteRenderer_;
    private Transform targetTransform_;
    private SpriteAnimation spriteAnimation_;
    private Action animationAction_;
    private float animationTimer_;
    private Vector3 initialScale_;
    private bool isExpandEnded_ = false;


    public override void Initialize()
    {
        spriteRenderer_ = entity.GetComponent<SpriteRenderer>();
        spriteAnimation_ = entity.GetScript<SpriteAnimation>();
        Entity playerEntity = ecsGroup.FindEntity("Player");
        targetTransform_ = playerEntity.GetComponent<Transform>();
        initialScale_ = transform.scale;
        animationAction_ = Wait;


        if (spriteAnimation_ != null)
        {
            spriteAnimation_.startFrame = 0;
            spriteAnimation_.endFrame = 1;
            spriteAnimation_.isLoop = true;
            spriteAnimation_.Play();
        }
    }

    public override void Update()
    {
        Vector3 toTarget = targetTransform_.position - transform.position;

        // ベクトルの長さが0以下の場合は処理を終了
        if (toTarget.LengthSq() <= 0.0f)
        {
            return;
        }

        // ふぐからプレイヤーへのベクトルと右方向ベクトルの内積を計算
        bool isRight = Vector3.Dot(Vector3.right, toTarget.Normalized()) >= 0.0f;

        // 向きに合わせて連番フレームのUVを反転/復元する
        ApplyFlip(isRight);
        // 表示中の絵に合わせて上下方向の回転を乗せる
        FaceTargetDirection(toTarget, isRight);

        // アニメーションの実行
        animationAction_?.Invoke();
    }

    // SpriteAnimationはフレームが切り替わった時しかUVを書き直さないため、
    // 「今のUVをそのまま反転」だと反転していない/している状態を判定できず、
    // 毎フレーム反転→復元→反転…を繰り返してチラつく。
    // なので必ず一旦「反転前(正のscale)」の基準値へ戻してから、向きに応じて反転をかけ直す(冪等にする)。
    private void ApplyFlip(bool isRight)
    {
        if (spriteRenderer_ == null) { return; }

        UVTransform uv = spriteRenderer_.uvTransform;

        float baseScaleX = Mathf.Abs(uv.scale.x);
        float basePositionX = uv.scale.x >= 0.0f ? uv.position.x : uv.position.x - baseScaleX;

        uv.scale.x = isRight ? baseScaleX : -baseScaleX;
        uv.position.x = isRight ? basePositionX : basePositionX + baseScaleX;

        spriteRenderer_.uvTransform = uv;
    }

    private void FaceTargetDirection(Vector3 toTarget, bool isRight)
    {
        Vector3 dir = toTarget.Normalized();

        // 現在表示している絵の正面方向
        Vector3 baseDir = Vector3.right;
        if (!isRight)
        {
            baseDir = -Vector3.right;
        }

        // baseDir から見たターゲット方向の符号付き角度
        float angle = Mathf.Atan2(dir.y, dir.x) - Mathf.Atan2(baseDir.y, baseDir.x);
        angle = WrapPi(angle);

        // ±90°にクランプする
        float clampedAngle = Mathf.Clamp(angle, -Mathf.PI * 0.5f, Mathf.PI * 0.5f);

        // TargetRotateの計算
        Quaternion targetRotate = Quaternion.MakeFromAxis(Vector3.forward, clampedAngle);

        // 回転の適用
        transform.rotate = Quaternion.Slerp(transform.rotate, targetRotate, 0.3f);
    }

    // 角度差を (-PI, PI] に正規化
    private static float WrapPi(float angle)
    {
        float twoPi = 2.0f * Mathf.PI;
        while (angle > Mathf.PI) angle -= twoPi;
        while (angle < -Mathf.PI) angle += twoPi;
        return angle;
    }

    ///-----------------------------------------------------------------------------
    /// アニメーション関連
    ///-----------------------------------------------------------------------------
    private void Wait() {
        animationTimer_ = 0.0f;
        isExpandEnded_ = false;
    }

    private void PlayingExpandAnimation()
    {

        // 拡大アニメーションの進行度を計算
        animationTimer_ += Time.deltaTime;
        float time = Mathf.Clamp01(animationTimer_ / expandanimationDuration);

        // 拡大アニメーションの進行度に応じてスケールを計算
        float scale = Mathf.Lerp(1.0f, expandScaleRate, Ease.Out.Back(time));
        transform.scale = initialScale_ * scale;

        if (animationTimer_ >= expandanimationDuration)
        {
            // 針を発射
            isExpandEnded_ = true;

            animationTimer_  = 0.0f;
            animationAction_ = PlayingReturnAnimation;
        }
    }

    // 拡大しきったスケールから元のスケールへ戻す
    private void PlayingReturnAnimation()
    {

        // 縮小アニメーションの進行度を計算
        animationTimer_ += Time.deltaTime;
        float time = Mathf.Clamp01(animationTimer_ / returnanimationDuration);

        // 縮小アニメーションの進行度に応じてスケールを計算
        float scale = Mathf.Lerp(expandScaleRate, 1.0f, time);
        transform.scale = initialScale_ * scale;

        // 縮小アニメーションが終了したら元のスケールに戻す
        if (animationTimer_ >= returnanimationDuration)
        {
            transform.scale  = initialScale_;
            animationAction_ = Wait;
        }
    }

    public void StartPlayAnimation() {
        // アニメーションを開始する
        animationTimer_ = 0.0f;
        animationAction_ = PlayingExpandAnimation;
    }

    public bool IsFireNeedle() { 
        return isExpandEnded_;
    }
    public void ResetExpandState()
    {
        isExpandEnded_ = false;
    }
}
