using System;
using System.Collections.Generic;

public enum KingYadokariAttackTypeEnum {
    GiantClaw,
    ShellBullet
}

public class KingYadokari : MonoScript {
    [SerializeField] public string targetEntityName = "Player";
    [SerializeField] public float idleDuration = 1.5f;
    [SerializeField] public float knockDownDuration = 4.0f;
    [SerializeField] public float getUpDuration = 1.0f;
    [SerializeField] public bool randomizeAttackType = true;
    [SerializeField] public KingYadokariAttackTypeEnum fixedAttackType = KingYadokariAttackTypeEnum.GiantClaw;

    private HP hp_;
    private Entity targetEntity_;
    private IKingYadokariState state_;
    private KingYadokariGiantClawAttackSettings giantClawSettings_;
    private KingYadokariShellBulletAttackSettings shellBulletSettings_;
    private readonly List<YadokariGiantClaw> giantClaws_ = new List<YadokariGiantClaw>();
    private YadokariGiantClaw activeGiantClaw_;
    private bool bulletResolved_;
    private bool reflectedBulletHit_;
    private bool isKnockedDown_;
    private bool giantClawDestroyed_;
    private Quaternion standingRotation_;

    public override void Initialize() {
        hp_ = entity.GetScript<HP>();
        if (hp_ != null) {
            hp_.IsDirectlyDamageable = false;
        }

        giantClawSettings_ = GetOrAddSettings<KingYadokariGiantClawAttackSettings>();
        shellBulletSettings_ = GetOrAddSettings<KingYadokariShellBulletAttackSettings>();
        ResolveGiantClaws();
        ResolveTarget();
        standingRotation_ = transform.rotation;
        bulletResolved_ = false;
        reflectedBulletHit_ = false;
        isKnockedDown_ = false;
        giantClawDestroyed_ = giantClaws_.Count == 0;
        SetVisualState(new Vector4(1.0f, 1.0f, 1.0f, 1.0f), standingRotation_);
        ChangeState(new KingYadokariIdleState());
    }

    public override void Update() {
        if (state_ != null) {
            state_.Update(this);
        }
    }

    internal void ChangeState(IKingYadokariState nextState) {
        if (state_ != null) {
            state_.Exit(this);
        }

        state_ = nextState;
        if (state_ != null) {
            state_.Enter(this);
        }
    }

    internal bool FireShellBullet() {
        ResolveTarget();
        bulletResolved_ = false;
        reflectedBulletHit_ = false;

        if (!giantClawDestroyed_ || targetEntity_ == null || targetEntity_.transform == null
            || String.IsNullOrEmpty(shellBulletSettings_.prefabName)) {
            return false;
        }

        Entity bulletEntity = ecsGroup.CreateEntity(shellBulletSettings_.prefabName);
        if (bulletEntity == null || bulletEntity.transform == null) {
            return false;
        }

        Vector2 spawnOffset = shellBulletSettings_.spawnOffset;
        Vector3 spawnPosition = transform.position + new Vector3(spawnOffset.x, spawnOffset.y, 0.0f);
        bulletEntity.transform.position = spawnPosition;

        YadokariShellBullet bullet = bulletEntity.GetScript<YadokariShellBullet>();
        if (bullet == null) {
            bulletEntity.Destroy();
            return false;
        }

        Vector3 toTarget = targetEntity_.transform.position - spawnPosition;
        toTarget.z = 0.0f;
        bullet.Configure(
            this,
            toTarget,
            shellBulletSettings_.speed,
            shellBulletSettings_.reflectedSpeed,
            shellBulletSettings_.lifeTime,
            shellBulletSettings_.damage,
            shellBulletSettings_.reflectionMinDamage);
        return true;
    }

    internal KingYadokariAttackTypeEnum SelectAttackType() {
        float giantClawWeight = giantClawDestroyed_ ? 0.0f : NonNegative(giantClawSettings_.selectionWeight);
        float shellBulletWeight = giantClawDestroyed_ ? NonNegative(shellBulletSettings_.selectionWeight) : 0.0f;

        if (!randomizeAttackType && IsAttackAvailable(fixedAttackType)) {
            return fixedAttackType;
        }

        float totalWeight = giantClawWeight + shellBulletWeight;
        if (totalWeight <= 0.0f) {
            return giantClawDestroyed_
                ? KingYadokariAttackTypeEnum.ShellBullet
                : KingYadokariAttackTypeEnum.GiantClaw;
        }

        float lottery = RandomUtil.NextFloat() * totalWeight;
        if (lottery < giantClawWeight) {
            return KingYadokariAttackTypeEnum.GiantClaw;
        }

        return KingYadokariAttackTypeEnum.ShellBullet;
    }

    internal bool BeginGiantClawAttack() {
        ResolveTarget();
        if (targetEntity_ == null || targetEntity_.transform == null) {
            return false;
        }

        activeGiantClaw_ = null;
        for (int i = 0; i < giantClaws_.Count; i++) {
            YadokariGiantClaw claw = giantClaws_[i];
            if (claw == null || claw.CurrentState != YadokariClawStateType.Idle) {
                continue;
            }

            if (claw.CommandAttack(
                targetEntity_,
                giantClawSettings_.damage,
                giantClawSettings_.launchSpeed,
                giantClawSettings_.launchDistance)) {
                activeGiantClaw_ = claw;
                break;
            }
        }

        if (activeGiantClaw_ == null) {
            return false;
        }

        SetVisualState(new Vector4(1.0f, 0.25f, 0.15f, 1.0f), standingRotation_);
        return true;
    }

    internal void EndGiantClawAttack() {
        if (activeGiantClaw_ != null) {
            activeGiantClaw_.CommandReturn();
            activeGiantClaw_ = null;
        }

        RestoreNormalVisual();
    }

    public void NotifyGiantClawDestroyed(YadokariGiantClaw giantClaw) {
        if (giantClaw == null) {
            return;
        }

        giantClaws_.Remove(giantClaw);
        if (activeGiantClaw_ == giantClaw) {
            activeGiantClaw_ = null;
        }
        giantClawDestroyed_ = giantClaws_.Count == 0;
    }

    internal void NotifyBulletResolved(bool reflectedHit) {
        if (bulletResolved_) {
            return;
        }

        bulletResolved_ = true;
        reflectedBulletHit_ = reflectedHit;
    }

    internal bool ConsumeBulletResult(out bool reflectedHit) {
        reflectedHit = false;
        if (!bulletResolved_) {
            return false;
        }

        reflectedHit = reflectedBulletHit_;
        bulletResolved_ = false;
        reflectedBulletHit_ = false;
        return true;
    }

    public void TakeWeakPointDamage(float damage) {
        if (!isKnockedDown_ || hp_ == null || damage <= 0.0f) {
            return;
        }

        hp_.TakeDamage(damage);
    }

    internal void BeginAttackTell() {
        SetVisualState(new Vector4(1.0f, 0.55f, 0.15f, 1.0f), standingRotation_);
    }

    internal void BeginKnockDown() {
        isKnockedDown_ = true;
        Quaternion flipped = standingRotation_ * Quaternion.MakeFromAxis(Vector3.back, Mathf.PI);
        SetVisualState(new Vector4(0.35f, 1.0f, 1.0f, 1.0f), flipped);
    }

    internal void BeginGetUp() {
        isKnockedDown_ = false;
        SetVisualState(new Vector4(0.65f, 0.65f, 1.0f, 1.0f), transform.rotation);
    }

    internal void UpdateGetUp(float progress) {
        Quaternion flipped = standingRotation_ * Quaternion.MakeFromAxis(Vector3.back, Mathf.PI);
        transform.rotation = Quaternion.Slerp(flipped, standingRotation_, Mathf.Clamp01(progress));
    }

    internal void FinishGetUp() {
        isKnockedDown_ = false;
        SetVisualState(new Vector4(1.0f, 1.0f, 1.0f, 1.0f), standingRotation_);
    }

    internal void RestoreNormalVisual() {
        if (!isKnockedDown_) {
            SetVisualState(new Vector4(1.0f, 1.0f, 1.0f, 1.0f), standingRotation_);
        }
    }

    private void ResolveTarget() {
        if (!String.IsNullOrEmpty(targetEntityName)) {
            targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        }
    }

    private void ResolveGiantClaws() {
        giantClaws_.Clear();
        activeGiantClaw_ = null;
        uint childCount = entity.GetChildCount();
        for (uint i = 0; i < childCount; i++) {
            Entity child = entity.GetChild(i);
            if (child == null) {
                continue;
            }

            YadokariGiantClaw giantClaw = child.GetScript<YadokariGiantClaw>();
            if (giantClaw != null) {
                giantClaws_.Add(giantClaw);
            }
        }
    }

    private bool IsAttackAvailable(KingYadokariAttackTypeEnum attackType) {
        if (attackType == KingYadokariAttackTypeEnum.GiantClaw) {
            return !giantClawDestroyed_;
        }

        return giantClawDestroyed_;
    }

    private T GetOrAddSettings<T>() where T : MonoScript {
        T settings = entity.GetScript<T>();
        return settings != null ? settings : entity.AddScript<T>();
    }

    private static float NonNegative(float value) {
        return value > 0.0f ? value : 0.0f;
    }

    private void SetVisualState(Vector4 color, Quaternion rotation) {
        transform.rotation = rotation;
        SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
        if (renderer != null) {
            renderer.color = color;
        }
    }

    public Entity TargetEntity { get { return targetEntity_; } }
    public bool IsKnockedDown { get { return isKnockedDown_; } }
    public float IdleDuration { get { return idleDuration; } }
    public float GiantClawTellDuration { get { return NonNegative(giantClawSettings_.tellDuration); } }
    public float GiantClawActiveDuration { get { return NonNegative(giantClawSettings_.activeDuration); } }
    public float GiantClawRecoveryDuration { get { return NonNegative(giantClawSettings_.recoveryDuration); } }
    public float ShellBulletTellDuration { get { return NonNegative(shellBulletSettings_.tellDuration); } }
    public float ShellBulletRecoveryDuration { get { return NonNegative(shellBulletSettings_.recoveryDuration); } }
    public float KnockDownDuration { get { return knockDownDuration; } }
    public float GetUpDuration { get { return getUpDuration; } }
}
