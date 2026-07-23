using System;
using System.Collections.Generic;

/// <summary>
/// ボス共通の死亡演出を管理する。
/// </summary>
public class BossDeathPresentation : MonoScript {
    [SerializeField] private string deathEffectEntityName = "King_DeadEffect01";
    [SerializeField] private int deathEffectEmitCount = 1;
    [SerializeField] private float whiteDuration = 0.2f;
    [SerializeField] private float fadeDuration = 0.8f;

    private readonly List<SpriteRenderer> renderers_ = new List<SpriteRenderer>();
    private readonly List<float> baseAlphas_ = new List<float>();
    private HP hp_;
    private float elapsed_;
    private bool started_;

    public override void Initialize() {
        hp_ = entity.GetScript<HP>();
        if (hp_ != null) {
            // 演出完了時にこのスクリプトから破棄する。
            hp_.DisableAutoDestruction = true;
        }

        elapsed_ = 0.0f;
        started_ = false;
    }

    public override void Update() {
        if (!started_) {
            if (hp_ == null || !hp_.IsDead) {
                return;
            }

            BeginPresentation();
        }

        elapsed_ += Time.deltaTime;

        float holdDuration = NonNegative(whiteDuration);
        float safeFadeDuration = NonNegative(fadeDuration);
        if (elapsed_ <= holdDuration) {
            ApplyWhiteFade(1.0f);
            return;
        }

        float fadeProgress = safeFadeDuration > 0.0f
            ? (elapsed_ - holdDuration) / safeFadeDuration
            : 1.0f;
        ApplyWhiteFade(1.0f - Mathf.Clamp01(fadeProgress));

        if (fadeProgress >= 1.0f) {
            MessageBus.Publish(new EnemyKilledEvent(entity != null ? entity.name : ""));
            entity.Destroy();
        }
    }

    private void BeginPresentation() {
        started_ = true;
        elapsed_ = 0.0f;
        renderers_.Clear();
        baseAlphas_.Clear();
        CollectTargets(entity);
        ApplyWhiteFade(1.0f);
        EmitDeathEffect();
    }

    private void CollectTargets(Entity target) {
        if (target == null) {
            return;
        }

        SpriteRenderer renderer = target.GetComponent<SpriteRenderer>();
        if (renderer != null) {
            renderers_.Add(renderer);
            baseAlphas_.Add(renderer.color.w);
        }

        BoxCollider2D collider = target.GetComponent<BoxCollider2D>();
        if (collider != null) {
            BoxCollider2D.BatchData colliderData = collider.GetBatchData();
            colliderData.enable = 0;
            collider.ApplyBatchData(colliderData);
        }

        Rigidbody2D rigidbody = target.GetComponent<Rigidbody2D>();
        if (rigidbody != null) {
            rigidbody.velocity = Vector2.zero;
        }

        uint childCount = target.GetChildCount();
        for (uint i = 0; i < childCount; i++) {
            CollectTargets(target.GetChild(i));
        }
    }

    private void ApplyWhiteFade(float alphaScale) {
        float clampedAlphaScale = Mathf.Clamp01(alphaScale);
        for (int i = 0; i < renderers_.Count; i++) {
            SpriteRenderer renderer = renderers_[i];
            if (renderer == null) {
                continue;
            }

            renderer.color = new Vector4(
                1.0f,
                1.0f,
                1.0f,
                baseAlphas_[i] * clampedAlphaScale);
        }
    }

    private void EmitDeathEffect() {
        if (String.IsNullOrEmpty(deathEffectEntityName) || deathEffectEmitCount <= 0) {
            return;
        }

        Entity effectEntity = ecsGroup.FindEntity(deathEffectEntityName);
        if (effectEntity == null || effectEntity.transform == null) {
            Debug.LogWarning(
                "BossDeathPresentation: particle entity was not found: "
                + deathEffectEntityName);
            return;
        }

        effectEntity.enable = true;
        effectEntity.transform.position = transform.position;
        effectEntity.transform.rotation = transform.rotation;

        ParticleSystem2D particleSystem = effectEntity.GetComponent<ParticleSystem2D>();
        if (particleSystem != null) {
            particleSystem.Emit(deathEffectEmitCount);
        }
    }

    private static float NonNegative(float value) {
        return value > 0.0f ? value : 0.0f;
    }
}
