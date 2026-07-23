
//============================================================
// クラゲの突進攻撃後のダメージ判定フィールド
//============================================================
public class JellyfishChargeDamageField : MonoScript
{
    [SerializeField] public float damage = 20.0f;
    [SerializeField] public float duration = 0.6f;
    [SerializeField] public float width = 180.0f;

    private float elapsed_;
    private Entity sparkParticleEntity_;

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

    //============================================================
    // フィールドの範囲設定
    //============================================================
    public void Configure(
        Vector2 start,
        Vector2 end,
        float fieldWidth,
        float fieldDamage,
        float fieldDuration,
        float depth,
        string sparkParticlePrefabName,
        int sparkParticleEmitCount)
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

        DeploySparkParticle(
            sparkParticlePrefabName,
            sparkParticleEmitCount,
            new Vector3(width, length, 1.0f));
    }

    public override void OnDestroy()
    {
        DestroySparkParticle();
    }

    private void DeploySparkParticle(string prefabName, int emitCount, Vector3 fieldSize)
    {
        DestroySparkParticle();

        if (string.IsNullOrEmpty(prefabName))
        {
            return;
        }

        Entity particleEntity = ecsGroup.CreateEntity(prefabName);
        if (particleEntity == null)
        {
            return;
        }

        particleEntity.transform.position = transform.position;
        particleEntity.transform.rotation = transform.rotation;
        particleEntity.transform.scale = Vector3.one;

        ParticleSystem2D particleSystem = particleEntity.GetComponent<ParticleSystem2D>();
        if (particleSystem == null)
        {
            particleEntity.Destroy();
            return;
        }

        sparkParticleEntity_ = particleEntity;
        particleSystem.SetBoxShape(fieldSize);

        if (emitCount > 0)
        {
            particleSystem.Emit(emitCount);
        }
    }

    private void DestroySparkParticle()
    {
        if (sparkParticleEntity_ == null)
        {
            return;
        }

        ParticleSystem2D particleSystem = sparkParticleEntity_.GetComponent<ParticleSystem2D>();
        if (particleSystem != null)
        {
            particleSystem.Stop();
        }

        sparkParticleEntity_.Destroy();
        sparkParticleEntity_ = null;
    }
}
