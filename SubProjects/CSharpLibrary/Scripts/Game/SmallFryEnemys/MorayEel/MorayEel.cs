public class MorayEel : MonoScript {
    /* ----- パラメータ ----- */

    /* ----- パーティクル ----- */
    // 常時再生する泳ぎの泡
    [SerializeField] private string swimParticlePrefabName = "enemySwimParticle";

    private HP hp_;
    private ChaseController chaseController_;
    private SpriteAnimation spriteAnimation_;
    private TargetFacingFlip facingFlip_;

    // 親の巨大スケールを継承させないよう、親子付けせず座標だけ追従させる泳ぎの泡
    private Entity swimParticleEntity_;
    // 敵単位でパーティクルの発生位置を調整するオフセットスクリプト
    private ParticleOffset particleOffset_;

    public override void Initialize() {
        if (entity == null) return;

        // スクリプト・コンポーネント取得
        hp_ = entity.GetScript<HP>();
        chaseController_ = entity.GetScript<ChaseController>();
        spriteAnimation_ = entity.GetScript<SpriteAnimation>();
        facingFlip_ = entity.GetScript<TargetFacingFlip>();
        particleOffset_ = entity.GetScript<ParticleOffset>();

        // 常時再生する泳ぎの泡を生成する
        SpawnSwimParticle();
    }

    public override void Update() {
        if (transform == null) return;

        // 親子付けしていない泳ぎの泡の座標を追従させる
        SyncSwimParticlePosition();

        // 死んだら更新スキップ
        if (hp_ != null && hp_.CurrentHp <= 0) {
            return;
        }

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

    // 常時再生する泳ぎの泡を生成する
    private void SpawnSwimParticle() {
        if (string.IsNullOrEmpty(swimParticlePrefabName) || ecsGroup == null || transform == null) { return; }

        Entity swimParticle = ecsGroup.CreateEntity(swimParticlePrefabName);
        if (!swimParticle || swimParticle.transform == null) { return; }

       
        swimParticle.transform.position = transform.position + GetParticleOffset();
        swimParticleEntity_ = swimParticle;
    }

    // 泳ぎの泡の座標をこの敵の現在位置に追従させる
    private void SyncSwimParticlePosition() {
        if (!swimParticleEntity_ || transform == null || swimParticleEntity_.transform == null) { return; }
        swimParticleEntity_.transform.position = transform.position + GetParticleOffset();
    }

   
    private Vector3 GetParticleOffset() {
        if (transform == null) { return Vector3.zero; }
        if (particleOffset_ == null) { return Vector3.zero; }

        Vector3 offset = particleOffset_.offset;
        bool isFacingRight = facingFlip_ == null || facingFlip_.IsFacingRight;
        offset.x = isFacingRight ? offset.x : -offset.x;
        return transform.rotate * offset;
    }

    public override void OnDestroy() {
        if (swimParticleEntity_) {
            swimParticleEntity_.Destroy();
            swimParticleEntity_ = null;
        }
    }
}
