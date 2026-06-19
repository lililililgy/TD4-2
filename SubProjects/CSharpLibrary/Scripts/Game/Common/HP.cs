class HP : MonoScript
{
    [SerializeField]
    public int MAX_HP = 100;
    [SerializeField]
    public int currentHp = 0;
    [SerializeField]
    public bool disableAutoDestruction = false;
    [SerializeField]
    public bool isInvincible = false;
    [SerializeField]
    public float regenPerSecond = 0.0f;

    private bool _isDead = false;
    private float accumulatedRegen = 0.0f;
    private int lastCurrentHp = 0;

    public override void Initialize()
    {
        currentHp = MAX_HP;
        lastCurrentHp = currentHp;
        _isDead = false;
        isInvincible = false;
        accumulatedRegen = 0.0f;
    }

    public override void Update()
    {
        if (_isDead)
        {
            if (!disableAutoDestruction)
            {
                entity.Destroy();
            }
            return;
        }

        // 自然回復
        if (regenPerSecond > 0 && currentHp < MAX_HP)
        {
            accumulatedRegen += regenPerSecond * Time.deltaTime;
            if (accumulatedRegen >= 1.0f)
            {
                int regenAmount = (int)accumulatedRegen;
                Heal(regenAmount);
                accumulatedRegen -= regenAmount;
            }
        }
    }

    public void TakeDamage(int damage)
    {
        if (_isDead || isInvincible) return;

        currentHp -= damage;
        if (currentHp <= 0)
        {
            currentHp = 0;
            OnDead();
        }
    }

    private void OnDead()
    {
        // 死亡フラグを立ててUpdateで破棄するようにする（衝突中のクラッシュ防止）
        _isDead = true;
    }

    public void Heal(int healAmount)
    {
        currentHp += healAmount;
        if (currentHp > MAX_HP)
        {
            currentHp = MAX_HP;
        }
    }

    public bool HasHPChanged()
    {
        bool changed = currentHp != lastCurrentHp;
        lastCurrentHp = currentHp;
        return changed;
    }

    public float CurrentHpRatio()
    {
        return Mathf.Clamp01((float)currentHp / MAX_HP);
    }
}
