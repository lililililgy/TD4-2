
// KingGesoの攻撃行動「墨弾」の弾丸の挙動を制御
public class KingGesoInkBullet : MonoScript
{
    [SerializeField] public float speed = 220.0f;
    [SerializeField] public float lifeTime = 7.0f;
    [SerializeField] public float damage = 8.0f;

    private Vector3 direction_ = Vector3.up;
    private float elapsed_;
    private bool configured_;
    private bool destroyRequested_;
    private KingGeso poolOwner_;

    public override void Initialize()
    {
        // Configureは生成フレーム、Initializeは次フレームに呼ばれる。
        if (!configured_)
        {
            direction_ = Vector3.up;
            elapsed_ = 0.0f;
            destroyRequested_ = false;
        }
    }

    public override void Update()
    {
        if (destroyRequested_)
        {
            Release();
            return;
        }

        if (!configured_)
        {
            return;
        }

        elapsed_ += Time.deltaTime;
        if (elapsed_ >= lifeTime)
        {
            Release();
            return;
        }

        transform.position += direction_ * speed * Time.deltaTime;
    }

    public void Configure(Vector3 direction, float bulletSpeed, float bulletLifeTime, float bulletDamage)
    {
        direction_ = direction.LengthSq() > 0.0001f ? direction.Normalized() : Vector3.up;
        speed = bulletSpeed > 0.0f ? bulletSpeed : 1.0f;
        lifeTime = bulletLifeTime > 0.0f ? bulletLifeTime : 0.01f;
        damage = bulletDamage;
        elapsed_ = 0.0f;
        configured_ = true;
        destroyRequested_ = false;

        float roll = Mathf.Atan2(direction_.x, direction_.y);
        transform.rotation = Quaternion.MakeFromAxis(Vector3.back, roll);

        AttackCollision attack = entity.GetScript<AttackCollision>();
        if (attack != null)
        {
            attack.Damage = damage;
        }
    }

    public void BindPool(KingGeso owner)
    {
        poolOwner_ = owner;
        Deactivate();
    }

    public void Deactivate()
    {
        configured_ = false;
        destroyRequested_ = false;
        elapsed_ = 0.0f;
        direction_ = Vector3.up;
        entity.enable = false;
    }

    public override void OnCollisionEnter(Entity collision)
    {
        if (configured_ && collision != null)
        {
            destroyRequested_ = true;
        }
    }

    private void Release()
    {
        if (poolOwner_ != null)
        {
            poolOwner_.ReturnInkBullet(entity);
            return;
        }

        entity.Destroy();
    }
}
