using System;
using System.Collections.Generic;

//============================================================
// 攻撃タイプの列挙型
//============================================================
public enum KingGesoAttackType
{
    WaveThrust, //波状突き攻撃
    PincerThrust, //挟み撃ち攻撃
    InkBarrage, //螺旋墨弾幕
    HomingBullet, //追尾弾発射
}

//============================================================
// キングゲソのクラス
//============================================================
public class KingGeso : MonoScript
{
    [SerializeField] public int maxHp = 1000;
    [SerializeField] public string gesoPrefabName = "GesoHand";
    [SerializeField] public string targetEntityName = "Player";
    [SerializeField] public string cameraEntityName = "MainCamera";
    [SerializeField] public Vector2 gesoSpawnOffset = Vector2.zero;
    [SerializeField] public float screenHalfWidth = 1280.0f;
    [SerializeField] public float screenHalfHeight = 720.0f;
    [SerializeField] public float screenEdgeMargin = 0.5f;
    /// 待機状態の持続時間（秒）
    [SerializeField] public float idleDuration = 2.0f;
    /// クールダウン時間（秒）
    [SerializeField] public float cooldownDuration = 3.0f;
    /// 攻撃タイプをランダムに選択するか
    [SerializeField] public bool randomizeAttackType = true;
    /// ランダム選択しない場合に使う攻撃タイプ
    [SerializeField] public KingGesoAttackType fixedAttackType = KingGesoAttackType.WaveThrust;
    [SerializeField] public string chargeEffectEntityName = "Geso_ChargeEffect";
    [SerializeField] public int chargeEffectEmitCount = 1;
    [SerializeField] public float chargeEffectEmitInterval = 0.2f;

    private HP hp_;
    private ExpDropper expDropper_;
    private IKingGesoState state_;
    private Entity targetEntity_;
    private Entity cameraEntity_;
    private List<Entity> activeGesos_ = new List<Entity>();
    private bool attackRequested_;
    private bool damageStateRequested_;
    private KingGesoWaveThrustSettings waveSettings_;
    private KingGesoPincerThrustSettings pincerSettings_;
    private KingGesoInkBarrageSettings inkSettings_;
    private KingGesoHomingAttackSettings homingSettings_;
    private readonly List<Entity> inkBulletPool_ = new List<Entity>();
    private readonly List<Entity> availableInkBullets_ = new List<Entity>();
    private readonly HashSet<Entity> activeInkBullets_ = new HashSet<Entity>();
    private Vector4 attackTellOriginalColor_;
    private bool attackTellActive_;
    private Entity chargeEffectEntity_;
    private float chargeEffectEmitElapsed_;


    //=============================================================
    // 初期化
    //=============================================================
    public override void Initialize()
    {
        // HPコンポーネントを取得または追加
        hp_ = entity.GetScript<HP>();
        if (hp_ == null)
        {
            hp_ = entity.AddScript<HP>();
        }

        hp_.MaxHp = maxHp;
        hp_.Initialize();
        expDropper_ = entity.GetScript<ExpDropper>();

        waveSettings_ = entity.GetScript<KingGesoWaveThrustSettings>();
        if (waveSettings_ == null) waveSettings_ = entity.AddScript<KingGesoWaveThrustSettings>();
        pincerSettings_ = entity.GetScript<KingGesoPincerThrustSettings>();
        if (pincerSettings_ == null) pincerSettings_ = entity.AddScript<KingGesoPincerThrustSettings>();
        inkSettings_ = entity.GetScript<KingGesoInkBarrageSettings>();
        if (inkSettings_ == null) inkSettings_ = entity.AddScript<KingGesoInkBarrageSettings>();
        homingSettings_ = entity.GetScript<KingGesoHomingAttackSettings>();
        if (homingSettings_ == null) homingSettings_ = entity.AddScript<KingGesoHomingAttackSettings>();

        InitializeInkBulletPool();

        // ターゲットとカメラのエンティティを取得
        targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        cameraEntity_ = ecsGroup.FindEntity(cameraEntityName);

        activeGesos_.Clear();
        attackRequested_ = false;
        damageStateRequested_ = false;
        chargeEffectEntity_ = null;
        chargeEffectEmitElapsed_ = 0.0f;
        ChangeState(new KingGesoIdleState());
    }

    //=============================================================
    // 更新
    //=============================================================
    public override void Update()
    {

        //死亡判定
        if(hp_ != null && hp_.CurrentHp <= 0)
        {
            // 死亡ステートに遷移
            if (!(state_ is KingGesoDeadState))
            {
                ChangeState(new KingGesoDeadState());
            }
            return;
        }

        if (damageStateRequested_)
        {
            damageStateRequested_ = false;
            if (!(state_ is KingGesoDamageState))
            {
                ChangeState(new KingGesoDamageState());
            }
        }

        if (state_ != null)
        {
            state_.Update(this);
        }
    }

    //=============================================================
    // ターゲットの設定
    //=============================================================
    public void SetTarget(Entity target)
    {
        targetEntity_ = target;
    }

    //=============================================================
    // 攻撃の要求
    //=============================================================
    public void RequestAttack()
    {
        attackRequested_ = true;
    }

    //=============================================================
    // ダメージ処理
    //=============================================================
    public void TakeDamage(float damage)
    {
        if (hp_ == null || damage <= 0.0f)
        {
            return;
        }

        float hpBeforeDamage = hp_.CurrentHp;
        hp_.TakeDamage(damage);
        if (hp_.CurrentHp >= hpBeforeDamage)
        {
            return;
        }

        if (expDropper_ == null)
        {
            expDropper_ = entity.GetScript<ExpDropper>();
        }
        if (expDropper_ != null)
        {
            expDropper_.DropOnDamage();
        }

        if(hp_.IsDead == false)
        {
            // 衝突コールバック中に触手をDestroyしないよう、状態遷移はUpdateまで遅延する。
            damageStateRequested_ = true;
        }
    }

    internal float IdleDuration
    {
        get { return idleDuration > 0.0f ? idleDuration : 0.01f; }
    }

    internal float CooldownDuration
    {
        get { return cooldownDuration > 0.0f ? cooldownDuration : 0.01f; }
    }

    internal float WaveAttackDuration => Positive(waveSettings_.attackDuration);
    internal int WaveGesoCount => waveSettings_.gesoCount > 0 ? waveSettings_.gesoCount : 1;
    internal float WaveGesoInterval => Positive(waveSettings_.spawnInterval);
    internal float WaveGesoLifeTime => Positive(waveSettings_.gesoLifeTime);

    internal float PincerAttackDuration => Positive(pincerSettings_.attackDuration);

    internal float GetAttackTellDuration(KingGesoAttackType attackType)
    {
        if (attackType == KingGesoAttackType.PincerThrust)
        {
            return NonNegative(pincerSettings_.tellDuration);
        }
        if (attackType == KingGesoAttackType.InkBarrage)
        {
            return NonNegative(inkSettings_.tellDuration);
        }
        if (attackType == KingGesoAttackType.HomingBullet)
        {
            return NonNegative(homingSettings_.tellDuration);
        }
        return NonNegative(waveSettings_.tellDuration);
    }

    internal void BeginAttackTell()
    {
        chargeEffectEmitElapsed_ = 0.0f;
        EmitChargeEffect();

        SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
        if (renderer == null)
        {
            return;
        }

        attackTellOriginalColor_ = renderer.color;
        attackTellActive_ = true;
    }

    internal void UpdateAttackTell(KingGesoAttackType attackType)
    {
        UpdateChargeEffect();

        if (!attackTellActive_)
        {
            return;
        }

        SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
        if (renderer == null)
        {
            return;
        }

        Vector4 tellColor = GetAttackTellColor(attackType);
        float pulse = (Mathf.Sin(Time.time * Mathf.PI * 6.0f) + 1.0f) * 0.5f;
        float blend = 0.25f + pulse * 0.55f;
        renderer.color = new Vector4(
            attackTellOriginalColor_.x + (tellColor.x - attackTellOriginalColor_.x) * blend,
            attackTellOriginalColor_.y + (tellColor.y - attackTellOriginalColor_.y) * blend,
            attackTellOriginalColor_.z + (tellColor.z - attackTellOriginalColor_.z) * blend,
            attackTellOriginalColor_.w);
    }

    internal void EndAttackTell()
    {
        chargeEffectEmitElapsed_ = 0.0f;
        if (!attackTellActive_)
        {
            return;
        }

        SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
        if (renderer != null)
        {
            renderer.color = attackTellOriginalColor_;
        }
        attackTellActive_ = false;
    }

    private void UpdateChargeEffect()
    {
        float interval = NonNegative(chargeEffectEmitInterval);
        if (interval <= 0.0f)
        {
            return;
        }

        chargeEffectEmitElapsed_ += Time.deltaTime;
        if (chargeEffectEmitElapsed_ < interval)
        {
            return;
        }

        chargeEffectEmitElapsed_ %= interval;
        EmitChargeEffect();
    }

    private void EmitChargeEffect()
    {
        if (chargeEffectEmitCount <= 0 || String.IsNullOrEmpty(chargeEffectEntityName))
        {
            return;
        }

        if (chargeEffectEntity_ == null || chargeEffectEntity_.Id == 0)
        {
            chargeEffectEntity_ = ecsGroup.FindEntity(chargeEffectEntityName);
        }
        if (chargeEffectEntity_ == null)
        {
            return;
        }

        chargeEffectEntity_.enable = true;
        if (chargeEffectEntity_.transform != null)
        {
            chargeEffectEntity_.transform.position = transform.position;
            chargeEffectEntity_.transform.rotation = transform.rotation;
        }

        ParticleSystem2D particleSystem = chargeEffectEntity_.GetComponent<ParticleSystem2D>();
        if (particleSystem != null)
        {
            particleSystem.Emit(chargeEffectEmitCount);
        }
    }

    private static Vector4 GetAttackTellColor(KingGesoAttackType attackType)
    {
        if (attackType == KingGesoAttackType.PincerThrust)
        {
            return new Vector4(1.0f, 0.25f, 0.25f, 1.0f);
        }
        if (attackType == KingGesoAttackType.InkBarrage)
        {
            return new Vector4(0.55f, 0.2f, 1.0f, 1.0f);
        }
        if (attackType == KingGesoAttackType.HomingBullet)
        {
            return new Vector4(0.2f, 0.85f, 1.0f, 1.0f);
        }
        return new Vector4(1.0f, 0.75f, 0.15f, 1.0f);
    }

    internal int HomingProjectileCount => homingSettings_.projectileCount > 0 ? homingSettings_.projectileCount : 1;
    internal float HomingProjectileInterval => Positive(homingSettings_.launchInterval);

    internal int InkBulletCountPerWave => inkSettings_.bulletCountPerWave > 0 ? inkSettings_.bulletCountPerWave : 1;
    internal int InkBulletWaveCount => inkSettings_.waveCount > 0 ? inkSettings_.waveCount : 1;
    internal float InkBulletWaveInterval => Positive(inkSettings_.waveInterval);
    internal float InkBarrageRecoveryDuration => inkSettings_.recoveryDuration > 0.0f ? inkSettings_.recoveryDuration : 0.0f;
    internal bool ReverseInkBarrageHalfway => inkSettings_.reverseHalfway;
    internal float InkBulletAngleOffset => inkSettings_.angleOffset;

    //=============================================================
    // 攻撃タイプの選択
    //=============================================================
    internal KingGesoAttackType SelectAttackType()
    {
        if (!randomizeAttackType)
        {
            return fixedAttackType;
        }

        // 攻撃タイプの選択重みを考慮してランダムに選択
        float waveWeight = NonNegative(waveSettings_.selectionWeight);
        float pincerWeight = NonNegative(pincerSettings_.selectionWeight);
        float inkWeight = NonNegative(inkSettings_.selectionWeight);
        float homingWeight = NonNegative(homingSettings_.selectionWeight);
        float totalWeight = waveWeight + pincerWeight + inkWeight + homingWeight;

        // 重みが0以下の場合は固定攻撃タイプを返す
        if (totalWeight <= 0.0f)
        {
            return fixedAttackType;
        }

        // ランダムに攻撃タイプを選択
        float lottery = RandomUtil.NextFloat() * totalWeight;
        if (lottery < waveWeight)
        {
            return KingGesoAttackType.WaveThrust;
        }

        lottery -= waveWeight;
        if (lottery < pincerWeight)
        {
            return KingGesoAttackType.PincerThrust;
        }

        lottery -= pincerWeight;
        if (lottery < inkWeight)
        {
            return KingGesoAttackType.InkBarrage;
        }

        lottery -= inkWeight;
        if (lottery < homingWeight)
        {
            return KingGesoAttackType.HomingBullet;
        }

        return KingGesoAttackType.WaveThrust;
    }

    //=============================================================
    // 攻撃リクエスト処理
    //=============================================================
    internal bool ConsumeAttackRequest()
    {
        bool requested = attackRequested_;
        attackRequested_ = false;
        return requested;
    }

    //=============================================================
    // 状態の変更
    //=============================================================
    internal void ChangeState(IKingGesoState nextState)
    {
        if (state_ != null)
        {
            state_.Exit(this);
        }

        state_ = nextState;
        if (state_  != null)
        {
            state_.Enter(this);
        }
    }

    //=============================================================
    // ゲソのスポーン処理
    //=============================================================
    internal Entity SpawnGeso()
    {
        gesoSpawnOffset = CreateRandomScreenEdgeOffset();
        return SpawnGesoAtPosition(GetScreenCenter() + gesoSpawnOffset);
    }

    //=============================================================
    // ゲソのスポーン処理（挟み撃ち用）
    //=============================================================
    internal bool SpawnPincerGesos(List<Entity> spawnedGesos)
    {
        if (spawnedGesos == null)
        {
            return false;
        }

        Vector2 center = GetScreenCenter();
        Vector2 target = GetTargetPosition();
        Vector2 targetOffset = target - center;
        float halfWidth = (screenHalfWidth > 0.0f ? screenHalfWidth : 0.01f) + NonNegative(screenEdgeMargin);
        float halfHeight = (screenHalfHeight > 0.0f ? screenHalfHeight : 0.01f) + NonNegative(screenEdgeMargin);

        // プレイヤーを通る2種類の対角線から選び、その両端と画面境界の交点へ配置する。
        float diagonalY = RandomUtil.NextFloat() < 0.5f ? halfHeight : -halfHeight;
        Vector2 direction = new Vector2(halfWidth, diagonalY).Normalized();
        Vector2 oppositeDirection = new Vector2(-direction.x, -direction.y);
        Vector2 firstOffset = FindScreenEdgeIntersection(targetOffset, direction, halfWidth, halfHeight);
        Vector2 secondOffset = FindScreenEdgeIntersection(targetOffset, oppositeDirection, halfWidth, halfHeight);

        Entity first = SpawnGesoAtPosition(center + firstOffset);
        Entity second = SpawnGesoAtPosition(center + secondOffset);

        if (first == null || second == null)
        {
            if (first != null) DestroyActiveGeso(first);
            if (second != null) DestroyActiveGeso(second);
            return false;
        }

        spawnedGesos.Add(first);
        spawnedGesos.Add(second);
        return true;
    }

    //=============================================================
    // 指定方向の直線と画面境界の交点を取得
    //=============================================================
    private static Vector2 FindScreenEdgeIntersection(
        Vector2 origin,
        Vector2 direction,
        float halfWidth,
        float halfHeight)
    {
        float distanceToVerticalEdge = float.MaxValue;
        if (direction.x > 0.0001f)
        {
            distanceToVerticalEdge = (halfWidth - origin.x) / direction.x;
        }
        else if (direction.x < -0.0001f)
        {
            distanceToVerticalEdge = (-halfWidth - origin.x) / direction.x;
        }

        float distanceToHorizontalEdge = float.MaxValue;
        if (direction.y > 0.0001f)
        {
            distanceToHorizontalEdge = (halfHeight - origin.y) / direction.y;
        }
        else if (direction.y < -0.0001f)
        {
            distanceToHorizontalEdge = (-halfHeight - origin.y) / direction.y;
        }

        float distance = distanceToVerticalEdge < distanceToHorizontalEdge
            ? distanceToVerticalEdge
            : distanceToHorizontalEdge;
        if (distance < 0.0f || distance == float.MaxValue)
        {
            return origin;
        }

        return origin + direction * distance;
    }

    //=============================================================
    // ゲソのスポーン処理（指定位置）
    //=============================================================
    private Entity SpawnGesoAtPosition(Vector2 spawnPosition)
    {
        if (String.IsNullOrEmpty(gesoPrefabName))
        {
            return null;
        }

        Entity geso = ecsGroup.CreateEntity(gesoPrefabName);
        if (geso == null)
        {
            return null;
        }

        geso.transform.position = new Vector3(spawnPosition.x, spawnPosition.y, 0.0f);

        activeGesos_.Add(geso);
        return geso;
    }

    //=============================================================
    // ゲソの攻撃開始処理
    //=============================================================
    internal bool StartGesoAttack(Entity geso, KingGesoAttackType attackType)
    {
        if (geso == null)
        {
            return false;
        }

        GesoHand hand = geso.GetScript<GesoHand>();
        if (hand == null)
        {
            return false;
        }

        float damage;
        float attackDuration;
        float moveDuration;
        float passThroughDistance;
        float rotationSpeed;
        if (attackType == KingGesoAttackType.PincerThrust)
        {
            damage = pincerSettings_.damage;
            attackDuration = PincerAttackDuration;
            moveDuration = Positive(pincerSettings_.moveDuration);
            passThroughDistance = pincerSettings_.passThroughDistance;
            rotationSpeed = pincerSettings_.rotationSpeed;
        }
        else
        {
            damage = waveSettings_.damage;
            attackDuration = WaveAttackDuration;
            moveDuration = Positive(waveSettings_.moveDuration);
            passThroughDistance = waveSettings_.passThroughDistance;
            rotationSpeed = waveSettings_.rotationSpeed;
        }

        GesoHandAttackCommand command = new GesoHandAttackCommand {
            target = targetEntity_,
            damage = damage,
            attackDuration = attackDuration,
            moveDuration = moveDuration,
            passThroughDistance = passThroughDistance,
            rotationMode = attackType == KingGesoAttackType.PincerThrust
                ? GesoHandRotationMode.MatchTargetRotation
                : GesoHandRotationMode.FaceAttackDirection,
        };

        hand.rotationSpeed = rotationSpeed;
        return hand.CommandAttack(command);
    }

    //=============================================================
    // 追尾弾の生成と発射
    //=============================================================
    internal Entity SpawnHomingProjectile()
    {
        if (String.IsNullOrEmpty(homingSettings_.projectilePrefabName))
        {
            return null;
        }

        Entity projectile = ecsGroup.CreateEntity(homingSettings_.projectilePrefabName);
        if (projectile == null)
        {
            return null;
        }

        Vector3 offset = new Vector3(
            homingSettings_.spawnOffset.x,
            homingSettings_.spawnOffset.y,
            0.0f);
        projectile.transform.position = transform.position + offset;
        return projectile;
    }

    internal bool ShootHomingBullet(Entity projectile)
    {
        if (projectile == null)
        {
            return false;
        }

        KingGesoHomingBullet homing = projectile.GetScript<KingGesoHomingBullet>();
        if (homing == null)
        {
            return false;
        }

        homing.speed = homingSettings_.projectileSpeed;
        homing.turnSpeed = homingSettings_.turnSpeed;
        homing.lifeTime = homingSettings_.projectileLifeTime;
        homing.damage = homingSettings_.projectileDamage;
        homing.swimParticlePrefabName = homingSettings_.swimParticlePrefabName;
        homing.swimParticleEmitCount = homingSettings_.swimParticleEmitCount;
        homing.swimParticleEmitInterval = homingSettings_.swimParticleEmitInterval;
        return homing.CommandLaunch(targetEntity_);
    }

    //=============================================================
    // 墨弾の生成と発射
    //=============================================================
    internal bool FireInkBullet(Vector2 direction)
    {
        if (availableInkBullets_.Count == 0)
        {
            return false;
        }

        int lastIndex = availableInkBullets_.Count - 1;
        Entity bullet = availableInkBullets_[lastIndex];
        availableInkBullets_.RemoveAt(lastIndex);
        activeInkBullets_.Add(bullet);
        bullet.enable = true;

        Vector3 offset = new Vector3(inkSettings_.spawnOffset.x, inkSettings_.spawnOffset.y, 0.0f);
        bullet.transform.position = transform.position + offset;

        KingGesoInkBullet bulletScript = bullet.GetScript<KingGesoInkBullet>();
        if (bulletScript == null)
        {
            activeInkBullets_.Remove(bullet);
            bullet.enable = false;
            availableInkBullets_.Add(bullet);
            return false;
        }

        bulletScript.Configure(
            new Vector3(direction.x, direction.y, 0.0f),
            inkSettings_.bulletSpeed,
            inkSettings_.bulletLifeTime,
            inkSettings_.bulletDamage);
        return true;
    }

    internal void ReturnInkBullet(Entity bullet)
    {
        if (bullet == null || !activeInkBullets_.Remove(bullet))
        {
            return;
        }

        KingGesoInkBullet bulletScript = bullet.GetScript<KingGesoInkBullet>();
        if (bulletScript != null)
        {
            bulletScript.Deactivate();
        }
        else
        {
            bullet.enable = false;
        }

        availableInkBullets_.Add(bullet);
    }

    internal void DeactivateAllInkBullets()
    {
        List<Entity> activeBullets = new List<Entity>(activeInkBullets_);
        for (int i = 0; i < activeBullets.Count; i++)
        {
            ReturnInkBullet(activeBullets[i]);
        }
    }

    private void InitializeInkBulletPool()
    {
        DeactivateAllInkBullets();
        if (inkBulletPool_.Count > 0 || String.IsNullOrEmpty(inkSettings_.bulletPrefabName))
        {
            return;
        }

        int automaticSize = InkBulletCountPerWave * InkBulletWaveCount;
        int poolSize = inkSettings_.poolSize > 0 ? inkSettings_.poolSize : automaticSize;
        for (int i = 0; i < poolSize; i++)
        {
            Entity bullet = ecsGroup.CreateEntity(inkSettings_.bulletPrefabName);
            if (bullet == null)
            {
                break;
            }

            KingGesoInkBullet bulletScript = bullet.GetScript<KingGesoInkBullet>();
            if (bulletScript == null)
            {
                bullet.Destroy();
                continue;
            }

            bulletScript.BindPool(this);
            inkBulletPool_.Add(bullet);
            availableInkBullets_.Add(bullet);
        }
    }

    //=============================================================
    // ゲソの破壊処理
    //=============================================================
    internal void DestroyActiveGeso()
    {

        List<Entity> activeGesos = new List<Entity>(activeGesos_);
        activeGesos_.Clear();

        for (int i = 0; i < activeGesos.Count; i++)
        {
            if (activeGesos[i] != null)
            {
                activeGesos[i].Destroy();
            }
        }
    }

    internal void DestroyActiveGeso(Entity geso)
    {
        if (geso == null)
        {
            return;
        }

        activeGesos_.Remove(geso);
        geso.Destroy();
    }


    //=============================================================
    // 画面端のランダムなオフセットを作成
    //=============================================================
    private Vector2 CreateRandomScreenEdgeOffset()
    {
        float halfWidth = screenHalfWidth > 0.0f ? screenHalfWidth : 0.01f;
        float halfHeight = screenHalfHeight > 0.0f ? screenHalfHeight : 0.01f;
        float horizontal = RandomUtil.NextFloat11() * halfWidth;
        float vertical = RandomUtil.NextFloat11() * halfHeight;
        int edge = (int)(RandomUtil.NextFloat() * 4.0f);

        switch (edge)
        {
            case 0:
                return new Vector2(-halfWidth - screenEdgeMargin, vertical);
            case 1:
                return new Vector2(halfWidth + screenEdgeMargin, vertical);
            case 2:
                return new Vector2(horizontal, -halfHeight - screenEdgeMargin);
            default:
                return new Vector2(horizontal, halfHeight + screenEdgeMargin);
        }
    }

    //=============================================================
    // 画面中心の取得
    //=============================================================
    private Vector2 GetScreenCenter()
    {
        Vector3 center = transform.position;
        if (cameraEntity_ != null && cameraEntity_.transform != null)
        {
            center = cameraEntity_.transform.position;
        }
        return new Vector2(center.x, center.y);
    }

    //=============================================================
    // ターゲットの位置の取得
    //=============================================================
    private Vector2 GetTargetPosition()
    {
        if (targetEntity_ != null && targetEntity_.transform != null)
        {
            Vector3 targetPosition = targetEntity_.transform.position;
            return new Vector2(targetPosition.x, targetPosition.y);
        }

        return GetScreenCenter();
    }

    private static float Positive(float value)
    {
        return value > 0.0f ? value : 0.01f;
    }

    private static float NonNegative(float value)
    {
        return value > 0.0f ? value : 0.0f;
    }
}
