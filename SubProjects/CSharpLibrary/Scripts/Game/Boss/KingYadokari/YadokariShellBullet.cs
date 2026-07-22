public enum YadokariShellBulletState {
    EnemyBullet,
    Reflected
}

public class YadokariShellBullet : MonoScript {
    private KingYadokari owner_;
    private Rigidbody2D rigidbody_;
    private Vector3 direction_ = Vector3.up;
    private float speed_;
    private float reflectedSpeed_;
    private float lifeTime_;
    private float damage_;
    private float reflectionMinDamage_;
    private float elapsed_;
    private bool configured_;
    private bool resolved_;
    private bool destroyRequested_;
    private YadokariShellBulletState state_;

    public override void Initialize() {
        rigidbody_ = entity.GetComponent<Rigidbody2D>();
        SetVelocity(Vector2.zero);
        if (configured_) {
            return;
        }

        elapsed_ = 0.0f;
        resolved_ = false;
        destroyRequested_ = false;
        state_ = YadokariShellBulletState.EnemyBullet;
    }

    public void Configure(
        KingYadokari owner,
        Vector3 direction,
        float speed,
        float reflectedSpeed,
        float lifeTime,
        float damage,
        float reflectionMinDamage) {
        owner_ = owner;
        direction_ = direction.LengthSq() > 0.0001f ? direction.Normalized() : Vector3.down;
        speed_ = speed > 0.0f ? speed : 1.0f;
        reflectedSpeed_ = reflectedSpeed > 0.0f ? reflectedSpeed : speed_;
        lifeTime_ = lifeTime > 0.0f ? lifeTime : 0.01f;
        damage_ = damage;
        reflectionMinDamage_ = reflectionMinDamage > 0.0f ? reflectionMinDamage : 0.0f;
        elapsed_ = 0.0f;
        configured_ = true;
        resolved_ = false;
        destroyRequested_ = false;
        state_ = YadokariShellBulletState.EnemyBullet;
        UpdateVelocity();
        ApplyRotation();
        SetColor(new Vector4(1.0f, 0.3f, 0.15f, 1.0f));
    }

    public override void Update() {
        if (destroyRequested_) {
            entity.Destroy();
            return;
        }

        if (!configured_ || resolved_) {
            return;
        }

        elapsed_ += Time.deltaTime;
        if (elapsed_ >= lifeTime_) {
            Resolve(false);
            return;
        }

        Vector2 velocity = UpdateVelocity();
        transform.position += new Vector3(velocity.x, velocity.y, 0.0f) * Time.deltaTime;
        ApplyRotation();
    }

    public override void OnCollisionEnter(Entity collision) {
        if (!configured_ || resolved_ || collision == null) {
            return;
        }

        if (IsOwnerEntity(collision)) {
            if (state_ == YadokariShellBulletState.Reflected) {
                Resolve(true);
            }
            return;
        }

        if (state_ == YadokariShellBulletState.Reflected) {
            return;
        }

        PlayerAttackComponent playerAttack = collision.GetScript<PlayerAttackComponent>();
        AttackCollision playerAttackCollision = collision.GetScript<AttackCollision>();
        if (playerAttack != null && playerAttackCollision != null
            && playerAttackCollision.Damage >= reflectionMinDamage_) {
            ReflectTowardOwner();
            return;
        }

        DamagePlayer(collision);
        Resolve(false);
    }

    private bool IsOwnerEntity(Entity collision) {
        if (owner_ == null || owner_.entity == null) {
            return false;
        }

        Entity current = collision;
        while (current != null) {
            if (current.Id == owner_.entity.Id) {
                return true;
            }
            current = current.parent;
        }

        return false;
    }

    private void ReflectTowardOwner() {
        if (owner_ == null || owner_.entity == null || owner_.entity.transform == null) {
            Resolve(false);
            return;
        }

        Vector3 toOwner = owner_.entity.transform.position - transform.position;
        toOwner.z = 0.0f;
        if (toOwner.LengthSq() <= 0.0001f) {
            Resolve(true);
            return;
        }

        direction_ = toOwner.Normalized();
        state_ = YadokariShellBulletState.Reflected;
        UpdateVelocity();
        ApplyRotation();
        SetColor(new Vector4(0.2f, 1.0f, 1.0f, 1.0f));
    }

    private void DamagePlayer(Entity collision) {
        DamageRelay relay = collision.GetScript<DamageRelay>();
        HP hp = relay != null ? relay.OwnerHp : collision.GetScript<HP>();
        if (hp != null && damage_ > 0.0f) {
            hp.TakeDamage(damage_);
        }
    }

    private void Resolve(bool reflectedHit) {
        if (resolved_) {
            return;
        }

        resolved_ = true;
        destroyRequested_ = true;
        SetVelocity(Vector2.zero);
        if (owner_ != null) {
            owner_.NotifyBulletResolved(reflectedHit);
        }
    }

    private void ApplyRotation() {
        float angle = Mathf.Atan2(direction_.x, direction_.y);
        transform.rotation = Quaternion.MakeFromAxis(Vector3.back, angle);
    }

    private Vector2 UpdateVelocity() {
        float currentSpeed = state_ == YadokariShellBulletState.Reflected ? reflectedSpeed_ : speed_;
        Vector2 velocity = new Vector2(direction_.x, direction_.y) * currentSpeed;
        SetVelocity(velocity);
        return velocity;
    }

    private void SetVelocity(Vector2 velocity) {
        if (rigidbody_ == null) {
            rigidbody_ = entity.GetComponent<Rigidbody2D>();
        }

        if (rigidbody_ != null) {
            rigidbody_.velocity = velocity;
        }
    }

    private void SetColor(Vector4 color) {
        SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
        if (renderer != null) {
            renderer.color = color;
        }
    }
}
