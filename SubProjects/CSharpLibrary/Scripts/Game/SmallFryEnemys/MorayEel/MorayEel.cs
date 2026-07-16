public class MorayEel : MonoScript {
    /* ----- パラメータ ----- */


    private HP hp_;
    private ChaseController chaseController_;
    private SpriteAnimation spriteAnimation_;
    private TargetFacingFlip facingFlip_;

    public override void Initialize() {
        // スクリプト・コンポーネント取得
        hp_ = entity.GetScript<HP>();
        chaseController_ = entity.GetScript<ChaseController>();
        spriteAnimation_ = entity.GetScript<SpriteAnimation>();
        facingFlip_ = entity.GetScript<TargetFacingFlip>();
    }

    public override void Update() {

        // 死んだら更新スキップ
        if (hp_ != null && hp_.CurrentHp <= 0) {
            return;
        }

        // プレイヤー座標を直接見るのではなく、実際の進行方向(速度)を向く。
        // プレイヤー座標を直接見て向きを決めると、プレイヤーが反対側に回り込んだ瞬間に
        // UV反転が急に切り替わって不自然に見えるため。速度が無い(≒Wait中)は向きを変えない。
        Vector3 velocity = chaseController_ != null ? chaseController_.Velocity : Vector3.zero;
        if (velocity.LengthSq() > 0.0001f) {
            facingFlip_?.FaceDirection(velocity.Normalized());
        }

        // 追いかけている(Chase/Rush)間だけ連番アニメーションを再生する
        if (IsChasing()) {
            AnimationPlay();
        } else {
            AnimationStop();
        }
    }

    private bool IsChasing() {
        if (chaseController_ == null) { return false; }
        return chaseController_.CurrentState == ChaseController.State.Chase
            || chaseController_.CurrentState == ChaseController.State.Rush;
    }

    private void AnimationStop() {
        // isPlayが既にfalseならここで何もしない。
        // SetFrame()はSpriteAnimation側でUVTransformを常に正のscaleで丸ごと上書きするため、
        // 毎フレーム呼ぶとTargetFacingFlipが設定したUV反転(負のscale)を毎回潰してしまう。
        if (spriteAnimation_ == null || !spriteAnimation_.isPlay) { return; }
        spriteAnimation_.isPlay = false;
    }

    private void AnimationPlay() {
        if (spriteAnimation_ == null || spriteAnimation_.isPlay) { return; }
        spriteAnimation_.startFrame = 0;
        spriteAnimation_.endFrame = 1;
        spriteAnimation_.isLoop = true;
        spriteAnimation_.isPlay = true;
    }

    private void FireMorayEel() {

    }
}
