using System;
using System.Collections.Generic;

//============================================================
// 攻撃タイプの列挙型
//============================================================
public enum KingGesoAttackType
{
    WaveThrust, //波状突き攻撃
    PincerThrust, //挟み撃ち攻撃
}

//============================================================
// キングゲソのクラス
//============================================================
public class KingGeso : MonoScript
{
    private const int DefaultWaveGesoCount = 6;
    private const float DefaultWaveGesoInterval = 2.0f;

    [SerializeField] public int maxHp = 1000;
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
    [SerializeField] public float gesoMoveDuration = 0.1f;
    /// ゲソの通過距離
    [SerializeField] public float gesoPassThroughDistance = 300.0f;
    /// 波状攻撃で出すゲソの本数
    [SerializeField] public int waveGesoCount = DefaultWaveGesoCount;
    /// 波状攻撃で次のゲソを出す間隔（秒）
    [SerializeField] public float waveGesoInterval = DefaultWaveGesoInterval;
    /// 攻撃タイプをランダムに選択するか
    [SerializeField] public bool randomizeAttackType = true;
    /// ランダム選択しない場合に使う攻撃タイプ
    [SerializeField] public KingGesoAttackType fixedAttackType = KingGesoAttackType.WaveThrust;
    /// 波状突きの選択重み
    [SerializeField] public float waveThrustWeight = 0.5f;
    /// 挟み撃ち突きの選択重み
    [SerializeField] public float pincerThrustWeight = 0.5f;

    private HP hp_;
    private IKingGesoState state_;
    private Entity targetEntity_;
    private Entity cameraEntity_;
    private List<Entity> activeGesos_ = new List<Entity>();
    private bool attackRequested_;
    private bool damageStateRequested_;


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

        // ターゲットとカメラのエンティティを取得
        targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        cameraEntity_ = ecsGroup.FindEntity(cameraEntityName);

        activeGesos_.Clear();
        attackRequested_ = false;
        damageStateRequested_ = false;
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
        if (hp_ == null)
        {
            return;
        }

        hp_.TakeDamage(damage);

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
        float waveWeight = waveThrustWeight > 0.0f ? waveThrustWeight : 0.0f;
        float pincerWeight = pincerThrustWeight > 0.0f ? pincerThrustWeight : 0.0f;
        float totalWeight = waveWeight + pincerWeight;

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

        return KingGesoAttackType.PincerThrust;
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
        float halfWidth = screenHalfWidth > 0.0f ? screenHalfWidth : 0.01f;
        float halfHeight = screenHalfHeight > 0.0f ? screenHalfHeight : 0.01f;

        Entity first;
        Entity second;
        if (RandomUtil.NextFloat() < 0.5f)
        {
            float y = Mathf.Clamp(targetOffset.y, -halfHeight, halfHeight);
            first = SpawnGesoAtPosition(center + new Vector2(-halfWidth - screenEdgeMargin, y));
            second = SpawnGesoAtPosition(center + new Vector2(halfWidth + screenEdgeMargin, y));
        }
        else
        {
            float x = Mathf.Clamp(targetOffset.x, -halfWidth, halfWidth);
            first = SpawnGesoAtPosition(center + new Vector2(x, -halfHeight - screenEdgeMargin));
            second = SpawnGesoAtPosition(center + new Vector2(x, halfHeight + screenEdgeMargin));
        }

        if (first == null || second == null)
        {
            return false;
        }

        spawnedGesos.Add(first);
        spawnedGesos.Add(second);
        return true;
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

        GesoHandAttackCommand command = new GesoHandAttackCommand {
            target = targetEntity_,
            damage = gesoAttackDamage,
            attackDuration = AttackDuration,
            moveDuration = gesoMoveDuration,
            passThroughDistance = gesoPassThroughDistance,
            rotationMode = attackType == KingGesoAttackType.PincerThrust
                ? GesoHandRotationMode.MatchTargetRotation
                : GesoHandRotationMode.FaceAttackDirection,
        };

        hand.rotationSpeed = gesoRotationSpeed;
        return hand.CommandAttack(command);
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
        Vector3 center = transform.worldPosition;
        if (cameraEntity_ != null && cameraEntity_.transform != null)
        {
            center = cameraEntity_.transform.worldPosition;
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
            Vector3 targetPosition = targetEntity_.transform.worldPosition;
            return new Vector2(targetPosition.x, targetPosition.y);
        }

        return GetScreenCenter();
    }
}
