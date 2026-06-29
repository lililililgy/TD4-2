using System;

public class KingGeso : MonoScript
{
    [SerializeField] public int maxHp = 10;
    [SerializeField] public string gesoPrefabName = "GesoHand";
    [SerializeField] public string targetEntityName = "Player";
    [SerializeField] public string cameraEntityName = "Camera";
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
    [SerializeField] public float gesoMoveDuration = 0.5f;
    /// ゲソの通過距離
    [SerializeField] public float gesoPassThroughDistance = 300.0f;

    private HP _hp;
    private IKingGesoState _state;
    private Entity _targetEntity;
    private Entity _cameraEntity;
    private Entity _activeGeso;
    private bool _attackRequested;


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

        _activeGeso = null;
        _attackRequested = false;
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

    //=============================================================
    // 内部処理
    //=============================================================
    internal bool ConsumeAttackRequest()
    {
        bool requested = _attackRequested;
        _attackRequested = false;
        return requested;
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
    internal bool SpawnGeso()
    {
        DestroyActiveGeso();
        if (String.IsNullOrEmpty(gesoPrefabName))
        {
            return false;
        }

        _activeGeso = ecsGroup.CreateEntity(gesoPrefabName);
        if (_activeGeso == null)
        {
            return false;
        }

        gesoSpawnOffset = CreateRandomScreenEdgeOffset();
       Vector2 spawnPosition = GetScreenCenter() + gesoSpawnOffset;
       // Vector2 spawnPosition = new Vector2(10.0f, 10.0f);
        _activeGeso.transform.position = new Vector3(spawnPosition.x, spawnPosition.y, GetMovementDepth());
        return true;
    }

    //=============================================================
    // ゲソの攻撃開始処理
    //=============================================================
    internal bool StartActiveGesoAttack()
    {
        if (_activeGeso == null)
        {
            return false;
        }

        GesoHand hand = _activeGeso.GetScript<GesoHand>();
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

    //=============================================================
    // ゲソの破壊処理
    //=============================================================
    internal void DestroyActiveGeso()
    {
        if (_activeGeso != null)
        {
            _activeGeso.Destroy();
            _activeGeso = null;
        }
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
