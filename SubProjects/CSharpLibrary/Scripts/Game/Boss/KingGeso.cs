using System;

public class KingGeso : MonoScript
{
    [SerializeField]
    public int maxHp = 10;
    [SerializeField]
    public string gesoPrefabName = "Geso";
    [SerializeField]
    public string targetEntityName = "Player";
    [SerializeField]
    public string cameraEntityName = "Camera1";
    [SerializeField]
    public Vector3 gesoSpawnOffset = Vector3.zero;
    [SerializeField]
    public float screenHalfWidth = 8.0f;
    [SerializeField]
    public float screenHalfHeight = 4.5f;
    [SerializeField]
    public float screenEdgeMargin = 0.5f;
    [SerializeField]
    public float idleDuration = 2.0f;
    [SerializeField]
    public float attackDuration = 1.0f;
    [SerializeField]
    public float cooldownDuration = 1.0f;
    [SerializeField]
    public float gesoAttackDamage = 10.0f;
    [SerializeField]
    public float gesoAttackRadius = 1.5f;
    [SerializeField]
    public float gesoRotationSpeed = 8.0f;
    [SerializeField]
    public float gesoMoveDuration = 0.25f;

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

        _hp.MAX_HP = maxHp > 0 ? maxHp : 1;
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
        _activeGeso.transform.position = GetScreenCenter() + gesoSpawnOffset;
        return true;
    }

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

        hand.attackDamage = gesoAttackDamage;
        hand.attackRadius = gesoAttackRadius;
        hand.attackDuration = AttackDuration;
        hand.rotationSpeed = gesoRotationSpeed;
        hand.moveDuration = gesoMoveDuration;
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

    private Vector3 CreateRandomScreenEdgeOffset()
    {
        float halfWidth = screenHalfWidth > 0.0f ? screenHalfWidth : 0.01f;
        float halfHeight = screenHalfHeight > 0.0f ? screenHalfHeight : 0.01f;
        float horizontal = RandomUtil.NextFloat11() * halfWidth;
        float vertical = RandomUtil.NextFloat11() * halfHeight;
        int edge = (int)(RandomUtil.NextFloat() * 4.0f);

        switch (edge)
        {
            case 0:
                return new Vector3(-halfWidth - screenEdgeMargin, 0.0f, vertical);
            case 1:
                return new Vector3(halfWidth + screenEdgeMargin, 0.0f, vertical);
            case 2:
                return new Vector3(horizontal, 0.0f, -halfHeight - screenEdgeMargin);
            default:
                return new Vector3(horizontal, 0.0f, halfHeight + screenEdgeMargin);
        }
    }

    private Vector3 GetScreenCenter()
    {
        Vector3 center = transform.worldPosition;
        if (_cameraEntity != null && _cameraEntity.transform != null)
        {
            center = _cameraEntity.transform.worldPosition;
        }

        if (_targetEntity != null && _targetEntity.transform != null)
        {
            center.y = _targetEntity.transform.worldPosition.y;
        }
        return center;
    }
}
