public class JellyfishChargeDamageField : MonoScript
{
    [SerializeField] public float damage = 20.0f;
    [SerializeField] public float duration = 0.6f;
    [SerializeField] public float width = 180.0f;

    private float elapsed_;

    public override void Initialize()
    {
        elapsed_ = 0.0f;
    }

    public override void Update()
    {
        elapsed_ += Time.deltaTime;
        if (elapsed_ >= duration)
        {
            entity.Destroy();
        }
    }

    public void Configure(Vector2 start, Vector2 end, float fieldWidth, float fieldDamage, float fieldDuration, float depth)
    {
        Vector2 move = end - start;
        float length = move.Length();
        if (length <= 0.001f)
        {
            entity.Destroy();
            return;
        }

        Vector2 direction = move.Normalized();
        Vector2 center = start + direction * (length * 0.5f);

        width = fieldWidth;
        damage = fieldDamage;
        duration = fieldDuration > 0.0f ? fieldDuration : 0.01f;
        elapsed_ = 0.0f;

        transform.position = new Vector3(center.x, center.y, depth);
        transform.scale = new Vector3(width, length, 1.0f);

        float angle = Mathf.Atan2(direction.x, direction.y);
        transform.rotation = Quaternion.MakeFromAxis(Vector3.back, angle);

        AttackCollision attackCollision = entity.GetScript<AttackCollision>();
        if (attackCollision != null)
        {
            attackCollision.Damage = damage;
        }
    }
}
