using System;

public class KingYadokari : MonoScript {
    [SerializeField] public string targetEntityName = "Player";
    [SerializeField] public string shellBulletPrefabName = "YadokariShellBullet";

    [SerializeField] public float idleDuration = 1.5f;
    [SerializeField] public float attackTellDuration = 0.8f;
    [SerializeField] public float attackRecoveryDuration = 0.8f;
    [SerializeField] public float knockDownDuration = 4.0f;
    [SerializeField] public float getUpDuration = 1.0f;

    [SerializeField] public Vector2 bulletSpawnOffset = new Vector2(0.0f, -180.0f);
    [SerializeField] public float bulletSpeed = 420.0f;
    [SerializeField] public float reflectedBulletSpeed = 720.0f;
    [SerializeField] public float bulletLifeTime = 7.0f;
    [SerializeField] public float bulletDamage = 1.0f;
    [SerializeField] public float reflectionMinDamage = 1.0f;

    private HP hp_;
    private Entity targetEntity_;
    private IKingYadokariState state_;
    private bool bulletResolved_;
    private bool reflectedBulletHit_;
    private bool isKnockedDown_;
    private Quaternion standingRotation_;

    public override void Initialize() {
        hp_ = entity.GetScript<HP>();
        if (hp_ != null) {
            hp_.IsDirectlyDamageable = false;
        }

        ResolveTarget();
        standingRotation_ = transform.rotation;
        bulletResolved_ = false;
        reflectedBulletHit_ = false;
        isKnockedDown_ = false;
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

        if (targetEntity_ == null || targetEntity_.transform == null || String.IsNullOrEmpty(shellBulletPrefabName)) {
            return false;
        }

        Entity bulletEntity = ecsGroup.CreateEntity(shellBulletPrefabName);
        if (bulletEntity == null || bulletEntity.transform == null) {
            return false;
        }

        Vector3 spawnPosition = transform.position + new Vector3(bulletSpawnOffset.x, bulletSpawnOffset.y, 0.0f);
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
            bulletSpeed,
            reflectedBulletSpeed,
            bulletLifeTime,
            bulletDamage,
            reflectionMinDamage);
        return true;
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
    public float AttackTellDuration { get { return attackTellDuration; } }
    public float AttackRecoveryDuration { get { return attackRecoveryDuration; } }
    public float KnockDownDuration { get { return knockDownDuration; } }
    public float GetUpDuration { get { return getUpDuration; } }
}
