using System;

public class SpikeFishAttack : MonoScript
{
    /* ----- パラメータ ----- */
    [SerializeField] private int   needleNum      = 8;    // 発射する針の数
    [SerializeField] private float attackDuration = 1.5f; // 回転チャージにかける秒数

    /* ----- 実行時状態 ----- */
    private Action                attackAction_;
    private SpikeFishAroundNeedle aroundNeedle_;
    private TargetRangeDetector   rangeDetector_;
    private SpriteAnimation       spriteAnim_;
    private ChaseController       chaseController_;
    private float                 attackTimer_;
    private bool                  isFiring_;

    public override void Initialize()
    {
        rangeDetector_   = entity.GetScript<TargetRangeDetector>();
        spriteAnim_      = entity.GetScript<SpriteAnimation>();
        chaseController_ = entity.GetScript<ChaseController>();
        attackAction_    = Wait;

        if (spriteAnim_ != null)
        {
            spriteAnim_.OnAnimationFinished += OnFireAnimationFinished;
        }
    }

    // 子エンティティは Initialize タイミングで未登録の場合があるため Wait で遅延取得
    private void TryGetAroundNeedle()
    {
        if (aroundNeedle_ != null) return;
        Entity child = entity.GetChild(0);
        if (child == null) return;
        aroundNeedle_ = child.GetScript<SpikeFishAroundNeedle>();
    }

    public override void Update()
    {
        attackAction_?.Invoke();
    }

    // 攻撃可能になるまで待機
    private void Wait()
    {
        TryGetAroundNeedle();
        if (aroundNeedle_ == null)     return;
        if (!aroundNeedle_.IsAppear)   return;
        if (rangeDetector_ == null)    return;
        if (!rangeDetector_.IsInRange) return;

        // 追跡を止めて回転チャージを開始する（スプライトアニメーションはまだ再生しない）
        attackTimer_ = 0.0f;
        chaseController_?.SetPaused(true);
        aroundNeedle_.StartCharging(attackDuration);
        attackAction_ = ChargeRotation;
    }

    // attackDuration 秒かけて回転チャージ。終わったらスプライトアニメーションを再生する
    private void ChargeRotation()
    {
        attackTimer_ += Time.deltaTime;
        if (attackTimer_ >= attackDuration)
        {
            isFiring_ = true;
            spriteAnim_?.ResetAnimation();
            spriteAnim_?.Play();
            aroundNeedle_.PlayFireAnimation();
            attackAction_ = PlayingAnimation;
        }
    }

    // スプライトアニメーション再生中。発射自体は OnFireAnimationFinished で行う
    private void PlayingAnimation()
    {
    }

    // アニメーションが1ループ再生し終わったタイミングで針を発射する
    private void OnFireAnimationFinished()
    {
        if (!isFiring_) return;
        isFiring_ = false;

        // SpriteAnimation は最終フレームのまま止まるため、明示的に元のUV位置へ戻す
        spriteAnim_?.ResetAnimation();
        aroundNeedle_.ResetFireAnimation();

        aroundNeedle_.FireNeedle(needleNum);
        chaseController_?.SetPaused(false);
        attackAction_ = Wait;
    }
}
