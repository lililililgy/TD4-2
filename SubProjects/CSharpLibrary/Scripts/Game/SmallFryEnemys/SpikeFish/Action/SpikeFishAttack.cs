using System;

public class SpikeFishAttack : MonoScript
{
    /* ----- パラメータ ----- */
    [SerializeField] private int needleNum = 8;           // 発射する針の数
    [SerializeField] private float attackDuration = 1.5f; // 回転チャージにかける秒数

    /* ----- 実行時状態 ----- */
    private Action attackAction_;
    private SpikeFishAroundNeedle aroundNeedle_;
    private TargetRangeDetector rangeDetector_;
    private ChaseController chaseController_;
    private SpikeFish spikeFish_;
    private float attackTimer_;
    private bool isFiring_;

    public override void Initialize()
    {
        rangeDetector_ = entity.GetScript<TargetRangeDetector>();
        chaseController_ = entity.GetScript<ChaseController>();
        spikeFish_ = entity.GetScript<SpikeFish>();
        attackAction_ = Wait;
    }


    private void TryGetAroundNeedle()
    {
        if (aroundNeedle_ != null)
        {
            return;
        }
        Entity child = entity.GetChild(0);
        if (child == null)
        {
            return;
        }
        aroundNeedle_ = child.GetScript<SpikeFishAroundNeedle>();
    }

    public override void Update()
    {
        attackAction_?.Invoke();
    }

    ///-----------------------------------------------------------------------------
    /// 攻撃アクション関連
    ///-----------------------------------------------------------------------------

    // 攻撃可能になるまで待機
    private void Wait()
    {
        TryGetAroundNeedle();

        // 攻撃可能条件を満たしていなければ何もしない
        if (aroundNeedle_ == null) { return; }
        if (!aroundNeedle_.IsAppear) { return; }
        if (rangeDetector_ == null) { return; }
        if (!rangeDetector_.IsInRange) { return; }

        // 追跡を止めて回転チャージを開始する
        attackTimer_ = 0.0f;
        chaseController_?.SetPaused(true);
        aroundNeedle_.StartCharging(attackDuration);
        attackAction_ = ChargeRotation;
    }


    private void ChargeRotation()
    {

        // 回転チャージ中の時間を計測する
        attackTimer_ += Time.deltaTime;
        if (attackTimer_ >= attackDuration)
        {
            // 攻撃の準備が整ったら針を発射するアニメーション再生
            isFiring_ = true;
            aroundNeedle_.PlayFireAnimation();
            spikeFish_?.StartPlayAnimation();
            attackAction_ = PlayingAnimation;
        }
    }

    // アニメーションが1ループ再生し終わったタイミングで針を発射する
    private void OnFireAnimationFinished()
    {
        if (!isFiring_) { return; }
        isFiring_ = false;

        // 針を発射して追跡を再開する
        aroundNeedle_.ResetFireAnimation();
        aroundNeedle_.FireNeedle(needleNum);
        chaseController_?.SetPaused(false);
        attackAction_ = Wait;
    }

    private void PlayingAnimation()
    {
        // アニメーションが終了したかどうかを判定する
        if (spikeFish_ != null && spikeFish_.IsFireNeedle())
        {
            OnFireAnimationFinished();
            spikeFish_.ResetExpandState();
        }
    }
}
