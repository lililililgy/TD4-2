public class KingGesoInkBullet : MonoScript
{
    [SerializeField] public float speed = 220.0f;
    [SerializeField] public float lifeTime = 7.0f;
    [SerializeField] public float damage = 8.0f;

    private Vector3 direction_ = Vector3.up;
    private float elapsed_;
    private bool configured_;

    public override void Initialize()
    {
        // Configureは生成フレーム、Initializeは次フレームに呼ばれる。
        if (!configured_)
        {
            direction_ = Vector3.up;
            elapsed_ = 0.0f;
        }
    }

    public override void Update()
    {
        if (!configured_)
        {
            return;
        }

        elapsed_ += Time.deltaTime;
        if (elapsed_ >= lifeTime)
        {
            entity.Destroy();
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

        float roll = Mathf.Atan2(direction_.x, direction_.y);
        transform.rotation = Quaternion.MakeFromAxis(Vector3.back, roll);

        AttackCollision attack = entity.GetScript<AttackCollision>();
        if (attack != null)
        {
            attack.Damage = damage;
        }
    }

    public override void OnCollisionEnter(Entity collision)
    {
        if (configured_ && collision != null && collision.Id != entity.Id)
        {
            entity.Destroy();
        }
    }
}
