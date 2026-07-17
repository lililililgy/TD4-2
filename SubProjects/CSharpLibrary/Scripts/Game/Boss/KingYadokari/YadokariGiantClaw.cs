using System;

public class YadokariGiantClaw : MonoScript {
    [SerializeField] private string ownerEntityName_ = "KingYadokari";
    [SerializeField] private float returnDuration_ = 0.3f;
    [SerializeField] private float destroyDuration_ = 0.2f;

    private KingYadokari owner_;
    private AttackCollision attackCollision_;
    private HP hp_;
    private IYadokariClawState state_;
    private Vector3 homePosition_;
    private Quaternion homeRotation_;
    private Vector3 returnStartPosition_;
    private Quaternion returnStartRotation_;
    private Vector3 attackDirection_;
    private float attackSpeed_;
    private float attackDistance_;
    private float attackTravelled_;
    private bool destructionNotified_;

    public override void Initialize() {
        ResolveOwner();
        attackCollision_ = entity.GetScript<AttackCollision>();
        hp_ = entity.GetScript<HP>();
        if (hp_ != null) {
            hp_.DisableAutoDestruction = true;
        }

        homePosition_ = transform.position;
        homeRotation_ = transform.rotation;
        destructionNotified_ = false;
        ChangeState(new YadokariClawIdleState());
    }

    public override void Update() {
        if (hp_ != null && hp_.IsDead && CurrentState != YadokariClawStateType.Destroyed) {
            ChangeState(new YadokariClawDestroyedState());
        }

        if (state_ != null) {
            state_.Update(this);
        }
    }

    public override void OnDestroy() {
        NotifyOwnerDestroyed();
    }

    public bool CommandAttack(Entity target, float damage, float speed, float distance) {
        if (CurrentState != YadokariClawStateType.Idle || target == null || target.transform == null) {
            return false;
        }

        ChangeState(new YadokariClawAttackingState(target, damage, speed, distance));
        return CurrentState == YadokariClawStateType.Attacking;
    }

    public bool CommandReturn() {
        if (CurrentState == YadokariClawStateType.Destroyed) {
            return false;
        }

        if (CurrentState == YadokariClawStateType.Idle
            || CurrentState == YadokariClawStateType.Returning) {
            return true;
        }

        ChangeState(new YadokariClawReturningState());
        return true;
    }

    internal void ChangeState(IYadokariClawState nextState) {
        if (state_ != null) {
            state_.Exit(this);
        }

        state_ = nextState;
        if (state_ != null) {
            state_.Enter(this);
        }
    }

    internal void ApplyIdle() {
        SetAttackDamage(0.0f);
        SetDamageInvincible(false);
        SetColliderEnabled(true);
        transform.position = homePosition_;
        transform.rotation = homeRotation_;
        SetColor(new Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    internal bool ApplyAttack(Entity target, float damage, float speed, float distance) {
        if (target == null || target.transform == null) {
            return false;
        }

        Vector3 toTarget = WorldPosition(target.transform) - WorldPosition(transform);
        toTarget.z = 0.0f;
        if (toTarget.LengthSq() <= 0.0001f) {
            return false;
        }

        attackDirection_ = toTarget.Normalized();
        attackSpeed_ = speed > 0.0f ? speed : 1.0f;
        attackDistance_ = distance > 0.0f ? distance : 0.0f;
        attackTravelled_ = 0.0f;
        SetAttackDamage(damage);
        SetDamageInvincible(true);
        SetColliderEnabled(true);
        SetColor(new Vector4(1.0f, 0.2f, 0.1f, 1.0f));
        float angle = Mathf.Atan2(attackDirection_.x, attackDirection_.y);
        transform.rotation = Quaternion.MakeFromAxis(Vector3.back, angle);
        return true;
    }

    internal bool UpdateAttack() {
        float worldStep = attackSpeed_ * Time.deltaTime;
        if (attackDistance_ > 0.0f) {
            float remaining = attackDistance_ - attackTravelled_;
            if (worldStep > remaining) {
                worldStep = remaining;
            }
        }

        Vector3 localStep = WorldDirectionToLocalStep(attackDirection_, worldStep);
        transform.position += localStep;
        attackTravelled_ += worldStep;
        return attackDistance_ > 0.0f && attackTravelled_ >= attackDistance_;
    }

    internal void BeginReturn() {
        SetAttackDamage(0.0f);
        SetDamageInvincible(false);
        returnStartPosition_ = transform.position;
        returnStartRotation_ = transform.rotation;
        SetColor(new Vector4(1.0f, 0.75f, 0.25f, 1.0f));
    }

    internal void UpdateReturn(float progress) {
        float ratio = Mathf.Clamp01(progress);
        transform.position = returnStartPosition_ + (homePosition_ - returnStartPosition_) * ratio;
        transform.rotation = Quaternion.Slerp(returnStartRotation_, homeRotation_, ratio);
    }

    internal void ApplyDestroyed() {
        SetAttackDamage(0.0f);
        SetDamageInvincible(false);
        SetColliderEnabled(false);
        SetColor(new Vector4(1.0f, 0.15f, 0.15f, 0.35f));
        NotifyOwnerDestroyed();
    }

    internal void DestroyEntity() {
        entity.Destroy();
    }

    private void NotifyOwnerDestroyed() {
        if (destructionNotified_) {
            return;
        }

        destructionNotified_ = true;
        if (owner_ == null) {
            ResolveOwner();
        }

        if (owner_ != null) {
            owner_.NotifyGiantClawDestroyed(this);
        }
    }

    private void SetAttackDamage(float damage) {
        if (attackCollision_ == null) {
            attackCollision_ = entity.GetScript<AttackCollision>();
        }

        if (attackCollision_ != null) {
            attackCollision_.Damage = damage > 0.0f ? damage : 0.0f;
        }
    }

    private void SetDamageInvincible(bool invincible) {
        if (hp_ == null) {
            hp_ = entity.GetScript<HP>();
        }

        if (hp_ != null) {
            hp_.IsInvincible = invincible;
        }
    }

    private Vector3 WorldDirectionToLocalStep(Vector3 worldDirection, float worldStep) {
        Entity parent = entity.parent;
        if (parent == null || parent.transform == null) {
            return worldDirection * worldStep;
        }

        Vector3 parentScale = parent.transform.scale;
        float scaleX = Mathf.Abs(parentScale.x) > 0.0001f ? Mathf.Abs(parentScale.x) : 1.0f;
        float scaleY = Mathf.Abs(parentScale.y) > 0.0001f ? Mathf.Abs(parentScale.y) : 1.0f;
        return new Vector3(
            worldDirection.x * worldStep / scaleX,
            worldDirection.y * worldStep / scaleY,
            0.0f);
    }

    private static Vector3 WorldPosition(Transform targetTransform) {
        return new Vector3(
            targetTransform.matrix.m30,
            targetTransform.matrix.m31,
            targetTransform.matrix.m32);
    }

    private void SetColliderEnabled(bool enabled) {
        BoxCollider2D collider = entity.GetComponent<BoxCollider2D>();
        if (collider != null) {
            collider.enable = enabled ? 1 : 0;
        }
    }

    private void SetColor(Vector4 color) {
        SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
        if (renderer != null) {
            renderer.color = color;
        }
    }

    private void ResolveOwner() {
        owner_ = null;
        Entity parent = entity.parent;
        if (parent != null) {
            owner_ = parent.GetScript<KingYadokari>();
        }

        if (owner_ != null) {
            return;
        }

        if (!String.IsNullOrEmpty(ownerEntityName_)) {
            Entity ownerEntity = ecsGroup.FindEntity(ownerEntityName_);
            owner_ = ownerEntity != null ? ownerEntity.GetScript<KingYadokari>() : null;
        }
    }

    public YadokariClawStateType CurrentState {
        get { return state_ != null ? state_.StateType : YadokariClawStateType.Idle; }
    }

    internal float ReturnDuration { get { return returnDuration_ > 0.0f ? returnDuration_ : 0.0f; } }
    internal float DestroyDuration { get { return destroyDuration_ > 0.0f ? destroyDuration_ : 0.0f; } }
}
