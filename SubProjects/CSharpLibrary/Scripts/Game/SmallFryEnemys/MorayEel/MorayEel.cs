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

    // 常時再生する泳ぎの泡を生成する
    private void SpawnSwimParticle() {
        if (string.IsNullOrEmpty(swimParticlePrefabName) || ecsGroup == null || transform == null) { return; }

        Entity swimParticle = ecsGroup.CreateEntity(swimParticlePrefabName);
        if (!swimParticle || swimParticle.transform == null) { return; }

        // 敵のスケールを継承すると泡もつられて拡大縮小し画面外に出てしまうため、
        // 親子付けはせず座標だけを毎フレーム追従させる。
        swimParticle.transform.position = transform.position + GetParticleOffset();
        swimParticleEntity_ = swimParticle;
    }

    // 泳ぎの泡の座標をこの敵の現在位置に追従させる
    private void SyncSwimParticlePosition() {
        if (!swimParticleEntity_ || transform == null || swimParticleEntity_.transform == null) { return; }
        swimParticleEntity_.transform.position = transform.position + GetParticleOffset();
    }

    // ParticleOffsetスクリプトが付いていればそのオフセットを、無ければゼロを返す。
    // TargetFacingFlipはUVのX反転で左右(ミラー)を表現しつつ、transform.rotateで泳ぎの傾きも付けている。
    // そのためオフセットはまずUV反転に合わせてX成分をミラーし、その後に現在の傾き(transform.rotate)を掛ける。
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
