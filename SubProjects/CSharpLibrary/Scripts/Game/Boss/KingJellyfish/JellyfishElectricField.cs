public class JellyfishElectricField : MonoScript
{
    [SerializeField] public float damage = 12.0f;
    [SerializeField] public float activationDelay = 1.0f;
    [SerializeField] public float activeDuration = 1.2f;
    [SerializeField] public float radius = 140.0f;
    [SerializeField] public string activationSePath = "./Assets/Sounds/se/boss/KingJellyfish_electricField.mp3";
    [SerializeField] public float activationSeVolume = 1.0f;
    [SerializeField] public float activationSePitch = 1.0f;

    private float elapsed_;
    private bool configured_;
    private bool activated_;
    private string thunderboltParticlePrefabName_;
    private int thunderboltParticleEmitCount_;
    private float thunderboltParticleDuration_;
    private float thunderboltParticleDestroyTime_;
    private Entity thunderboltParticleEntity_;

    public override void Initialize()
    {
        if (!configured_)
        {
            elapsed_ = 0.0f;
            activated_ = false;
        }

        thunderboltParticleEntity_ = null;
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

        if (thunderboltParticleEntity_ != null
            && elapsed_ >= thunderboltParticleDestroyTime_)
        {
            DestroyThunderboltParticle();
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
        float depth,
        string thunderboltParticlePrefabName,
        int thunderboltParticleEmitCount,
        float thunderboltParticleDuration)
    {
        radius = fieldRadius > 0.0f ? fieldRadius : 1.0f;
        damage = fieldDamage;
        activationDelay = delay > 0.0f ? delay : 0.01f;
        activeDuration = duration > 0.0f ? duration : 0.01f;
        elapsed_ = 0.0f;
        configured_ = true;
        activated_ = false;
        thunderboltParticlePrefabName_ = thunderboltParticlePrefabName;
        thunderboltParticleEmitCount_ = thunderboltParticleEmitCount > 0
            ? thunderboltParticleEmitCount
            : 1;
        thunderboltParticleDuration_ = thunderboltParticleDuration > 0.0f
            ? thunderboltParticleDuration
            : 1.0f;

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

        if (active)
        {
            SEOneShot.Play(entity, activationSePath, activationSeVolume, activationSePitch);
            EmitThunderboltParticle();
        }

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

    public override void OnDestroy()
    {
        DestroyThunderboltParticle();
    }

    private void EmitThunderboltParticle()
    {
        if (string.IsNullOrEmpty(thunderboltParticlePrefabName_))
        {
            return;
        }

        DestroyThunderboltParticle();

        Entity particleEntity = ecsGroup.CreateEntity(thunderboltParticlePrefabName_);
        if (particleEntity == null)
        {
            return;
        }

        particleEntity.transform.position = transform.position;
        particleEntity.transform.rotation = transform.rotation;

        ParticleSystem2D particleSystem = particleEntity.GetComponent<ParticleSystem2D>();
        if (particleSystem == null)
        {
            particleEntity.Destroy();
            return;
        }

        thunderboltParticleEntity_ = particleEntity;
        thunderboltParticleDestroyTime_ = elapsed_ + thunderboltParticleDuration_;
        particleSystem.Emit(thunderboltParticleEmitCount_);
    }

    private void DestroyThunderboltParticle()
    {
        if (thunderboltParticleEntity_ == null)
        {
            return;
        }

        thunderboltParticleEntity_.Destroy();
        thunderboltParticleEntity_ = null;
    }
}
