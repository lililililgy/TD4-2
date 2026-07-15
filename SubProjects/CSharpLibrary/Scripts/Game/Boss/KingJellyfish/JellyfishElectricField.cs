public class JellyfishElectricField : MonoScript
{
    [SerializeField] public float damage = 12.0f;
    [SerializeField] public float activationDelay = 1.0f;
    [SerializeField] public float activeDuration = 1.2f;
    [SerializeField] public float radius = 140.0f;

    private float elapsed_;
    private bool configured_;
    private bool activated_;

    public override void Initialize()
    {
        if (!configured_)
        {
            elapsed_ = 0.0f;
            activated_ = false;
        }
    }

    public override void Update()
    {
        if (!configured_)
        {
            return;
        }

        elapsed_ += Time.deltaTime;
        if (!activated_ && elapsed_ >= activationDelay)
        {
            SetActive(true);
        }

        if (elapsed_ >= activationDelay + activeDuration)
        {
            entity.Destroy();
        }
    }

    public void Configure(
        Vector2 position,
        float fieldRadius,
        float fieldDamage,
        float delay,
        float duration,
        float depth)
    {
        radius = fieldRadius > 0.0f ? fieldRadius : 1.0f;
        damage = fieldDamage;
        activationDelay = delay > 0.0f ? delay : 0.01f;
        activeDuration = duration > 0.0f ? duration : 0.01f;
        elapsed_ = 0.0f;
        configured_ = true;
        activated_ = false;

        transform.position = new Vector3(position.x, position.y, depth);
        transform.scale = new Vector3(radius * 2.0f, radius * 2.0f, 1.0f);

        AttackCollision attack = entity.GetScript<AttackCollision>();
        if (attack != null)
        {
            attack.Damage = damage;
        }

        SetActive(false);
    }

    private void SetActive(bool active)
    {
        activated_ = active;

        CircleCollider collider = entity.GetComponent<CircleCollider>();
        if (collider != null)
        {
            collider.enable = active ? 1 : 0;
        }

        SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
        if (renderer != null)
        {
            renderer.color = active
                ? new Vector4(0.3f, 0.85f, 1.0f, 0.9f)
                : new Vector4(1.0f, 0.85f, 0.2f, 0.35f);
        }
    }
}
