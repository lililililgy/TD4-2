using System;
using System.Collections.Generic;

public class KingGeso : MonoScript
{
    private const int DefaultWaveGesoCount = 6;
    private const float DefaultWaveGesoInterval = 2.0f;

    [SerializeField] public int maxHp = 10;
    [SerializeField] public string gesoPrefabName = "GesoHand";
    [SerializeField] public string targetEntityName = "Player";
    [SerializeField] public string cameraEntityName = "MainCamera";
    [SerializeField] public Vector2 gesoSpawnOffset = Vector2.zero;
    [SerializeField] public float screenHalfWidth = 1280.0f;
    [SerializeField] public float screenHalfHeight = 720.0f;
    [SerializeField] public float screenEdgeMargin = 0.5f;
    /// 待機状態の持続時間（秒）
    [SerializeField] public float idleDuration = 2.0f;
    /// 攻撃状態の持続時間（秒）
    [SerializeField] public float attackDuration = 3.0f;
    /// クールダウン時間（秒）
    [SerializeField] public float cooldownDuration = 3.0f;
    /// ゲソの攻撃力
    [SerializeField] public float gesoAttackDamage = 1.0f;
    /// ゲソの回転速度
    [SerializeField] public float gesoRotationSpeed = 8.0f;
    /// ゲソの移動時間
    [SerializeField] public float gesoMoveDuration = 0.1f;
    /// ゲソの通過距離
    [SerializeField] public float gesoPassThroughDistance = 300.0f;
    /// 波状攻撃で出すゲソの本数
    [SerializeField] public int waveGesoCount = DefaultWaveGesoCount;
    /// 波状攻撃で次のゲソを出す間隔（秒）
    [SerializeField] public float waveGesoInterval = DefaultWaveGesoInterval;
    /// 追尾弾のプレハブ名
    [SerializeField] public string homingProjectilePrefabName = "KingGesoHomingProjectile";
    /// 追尾弾の発射数
    [SerializeField] public int homingProjectileCount = 3;
    /// 追尾弾の発射間隔（秒）
    [SerializeField] public float homingProjectileInterval = 0.35f;
    /// 追尾弾の移動速度
    [SerializeField] public float homingProjectileSpeed = 120.0f;
    /// 追尾弾の追尾の強さ
    [SerializeField] public float homingProjectileTurnSpeed = 2.0f;
    /// 追尾弾の生存時間（秒）
    [SerializeField] public float homingProjectileLifeTime = 8.0f;
    /// 追尾弾の攻撃力
    [SerializeField] public float homingProjectileDamage = 1.0f;
    /// 追尾弾の発射位置
    [SerializeField] public Vector2 homingProjectileSpawnOffset = Vector2.zero;

    private HP _hp;
    private IKingGesoState _state;
    private Entity _targetEntity;
    private Entity _cameraEntity;
    private List<Entity> _activeGesos = new List<Entity>();
    private bool _attackRequested;
    private bool _useHomingAttackNext;


    //=============================================================
    // 初期化
    //=============================================================
    public override void Initialize()
    {
        _hp = entity.GetScript<HP>();
        if (_hp == null)
        {
            _hp = entity.AddScript<HP>();
        }

        _hp.MaxHp = maxHp > 0 ? maxHp : 1;
        _hp.Initialize();

        if (!String.IsNullOrEmpty(targetEntityName))
        {
            _targetEntity = ecsGroup.FindEntity(targetEntityName);
        }
        if (!String.IsNullOrEmpty(cameraEntityName))
        {
            _cameraEntity = ecsGroup.FindEntity(cameraEntityName);
        }

        _activeGesos.Clear();
        _attackRequested = false;
        _useHomingAttackNext = false;
        ChangeState(new KingGesoIdleState());
    }

    //=============================================================
    // 更新
    //=============================================================
    public override void Update()
    {
        // --- デバッグ用：HキーでHPを10%減らす ---
        if (Input.TriggerKey(KeyCode.H))
        {
            if (_hp != null)
            {
                _hp.TakeDamage(1);
            }
        }

        if (Input.TriggerKey(KeyCode.J))
        {
            RequestAttack();
        }

        if (_state != null)
        {
            _state.Update(this);
        }

        // --- デバッグ用：視線の表示 ---
        GizmoBatch.DrawRay(transform.position + Vector3.up * 2.0f, transform.forward * 5.0f, new Vector4(0, 1, 0, 1));
    }

    //=============================================================
    // ターゲットの設定
    //=============================================================
    public void SetTarget(Entity target)
    {
        _targetEntity = target;
    }

    //=============================================================
    // 攻撃の要求
    //=============================================================
    public void RequestAttack()
    {
        _attackRequested = true;
    }

    //=============================================================
    // ダメージ処理
    //=============================================================
    public void TakeDamage(float damage)
    {
        if (_hp == null)
        {
            return;
        }

        _hp.TakeDamage(damage);
    }

    internal float IdleDuration
    {
        get { return idleDuration > 0.0f ? idleDuration : 0.01f; }
    }

    internal float AttackDuration
    {
        get { return attackDuration > 0.0f ? attackDuration : 0.01f; }
    }

    internal float CooldownDuration
    {
        get { return cooldownDuration > 0.0f ? cooldownDuration : 0.01f; }
    }

    internal int WaveGesoCount
    {
        get { return waveGesoCount > 0 ? waveGesoCount : DefaultWaveGesoCount; }
    }

    internal float WaveGesoInterval
    {
        get { return waveGesoInterval > 0.0f ? waveGesoInterval : DefaultWaveGesoInterval; }
    }

    internal int HomingProjectileCount
    {
        get { return homingProjectileCount > 0 ? homingProjectileCount : 1; }
    }

    internal float HomingProjectileInterval
    {
        get { return homingProjectileInterval > 0.0f ? homingProjectileInterval : 0.01f; }
    }

    //=============================================================
    // 内部処理
    //=============================================================
    internal bool ConsumeAttackRequest()
    {
        bool requested = _attackRequested;
        _attackRequested = false;
        return requested;
    }

    internal IKingGesoState CreateNextAttackState()
    {
        IKingGesoState next = _useHomingAttackNext
            ? (IKingGesoState)new KingGesoHomingAttackState()
            : new KingGesoAttackState();
        _useHomingAttackNext = !_useHomingAttackNext;
        return next;
    }

    //=============================================================
    // 状態の変更
    //=============================================================
    internal void ChangeState(IKingGesoState nextState)
    {
        if (_state != null)
        {
            _state.Exit(this);
        }

        _state = nextState;
        if (_state != null)
        {
            _state.Enter(this);
        }
    }

    //=============================================================
    // ゲソのスポーン処理
    //=============================================================
    internal Entity SpawnGeso()
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

        gesoSpawnOffset = CreateRandomScreenEdgeOffset();
        Vector2 spawnPosition = GetScreenCenter() + gesoSpawnOffset;
        // Vector2 spawnPosition = new Vector2(10.0f, 10.0f);
        geso.transform.position = new Vector3(spawnPosition.x, spawnPosition.y, GetMovementDepth());
        _activeGesos.Add(geso);
        return geso;
    }

    //=============================================================
    // ゲソの攻撃開始処理
    //=============================================================
    internal bool StartGesoAttack(Entity geso)
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

        // ゲソの攻撃パラメーターを設定して攻撃を開始
        hand.attackDamage = gesoAttackDamage;
        hand.attackDuration = AttackDuration;
        hand.rotationSpeed = gesoRotationSpeed;
        hand.moveDuration = gesoMoveDuration;
        hand.passThroughDistance = gesoPassThroughDistance;
        return hand.CommandAttack(_targetEntity);
    }

    internal Entity SpawnHomingProjectile()
    {
        if (String.IsNullOrEmpty(homingProjectilePrefabName))
        {
            return null;
        }

        Entity projectile = ecsGroup.CreateEntity(homingProjectilePrefabName);
        if (projectile == null)
        {
            return null;
        }

        Vector3 offset = new Vector3(homingProjectileSpawnOffset.x, homingProjectileSpawnOffset.y, 0.0f);
        projectile.transform.position = transform.worldPosition + offset;
        return projectile;
    }

    internal bool StartHomingProjectile(Entity projectile)
    {
        if (projectile == null)
        {
            return false;
        }

        KingGesoHomingProjectile homing = projectile.GetScript<KingGesoHomingProjectile>();
        if (homing == null)
        {
            return false;
        }

        homing.speed = homingProjectileSpeed;
        homing.turnSpeed = homingProjectileTurnSpeed;
        homing.lifeTime = homingProjectileLifeTime;
        homing.damage = homingProjectileDamage;
        return homing.CommandLaunch(_targetEntity);
    }

    //=============================================================
    // ゲソの破壊処理
    //=============================================================
    internal void DestroyActiveGeso()
    {
        for (int i = 0; i < _activeGesos.Count; i++)
        {
            if (_activeGesos[i] != null)
            {
                _activeGesos[i].Destroy();
            }
        }
        _activeGesos.Clear();
    }

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

    private Vector2 GetScreenCenter()
    {
        Vector3 center = transform.worldPosition;
        if (_cameraEntity != null && _cameraEntity.transform != null)
        {
            center = _cameraEntity.transform.worldPosition;
        }
        return new Vector2(center.x, center.y);
    }

    private float GetMovementDepth()
    {
        if (_targetEntity != null && _targetEntity.transform != null)
        {
            return _targetEntity.transform.worldPosition.z;
        }
        return transform.worldPosition.z;
    }
}
