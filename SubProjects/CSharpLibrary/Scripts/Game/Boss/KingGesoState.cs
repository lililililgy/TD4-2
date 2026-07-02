
using System.Collections.Generic;

/// <summary>
/// キングゲソの状態(基底クラス)
/// </summary>
internal interface IKingGesoState
{
    void Enter(KingGeso owner);
    void Update(KingGeso owner);
    void Exit(KingGeso owner);
}

//==========================================-
// キングゲソの待機状態クラス
//==========================================
internal sealed class KingGesoIdleState : IKingGesoState
{
    // 経過時間
    private float _elapsed;

    /// <summary>
    /// 状態開始時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Enter(KingGeso owner)
    {
        _elapsed = 0.0f;
    }

    /// <summary>
    /// 状態更新時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Update(KingGeso owner)
    {
        _elapsed += Time.deltaTime;
        if (owner.ConsumeAttackRequest() || _elapsed >= owner.IdleDuration)
        {
            // 待機時間が経過したか、攻撃要求があった場合、攻撃状態に遷移
            owner.ChangeState(new KingGesoAttackState());
        }
    }

    /// <summary>
    /// 状態終了時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Exit(KingGeso owner)
    {
    }
}

//==========================================-
// キングゲソの攻撃状態
//==========================================
internal sealed class KingGesoAttackState : IKingGesoState
{
    private float _elapsed;
    private float _spawnElapsed;
    private int _spawnedCount;
    private readonly List<Entity> _pendingGesos = new List<Entity>();

    public void Enter(KingGeso owner)
    {
        _elapsed = 0.0f;
        _spawnElapsed = 0.0f;
        _spawnedCount = 0;
        _pendingGesos.Clear();

        SpawnNextGeso(owner);
    }

    public void Update(KingGeso owner)
    {
        StartPendingGesoAttacks(owner);

        _elapsed += Time.deltaTime;
        _spawnElapsed += Time.deltaTime;

        if (!SpawnDueGesos(owner))
        {
            return;
        }

        if (_elapsed >= owner.AttackDuration)
        {
            //attack終了後、クールダウン状態に遷移
            owner.ChangeState(new KingGesoCooldownState());
        }
    }

    public void Exit(KingGeso owner)
    {
        // 攻撃終了時にアクティブなゲソを破棄
        owner.DestroyActiveGeso();
    }

    private void StartPendingGesoAttacks(KingGeso owner)
    {
        for (int i = _pendingGesos.Count - 1; i >= 0; i--)
        {
            Entity pendingGeso = _pendingGesos[i];
            if (pendingGeso == null || owner.StartGesoAttack(pendingGeso))
            {
                _pendingGesos.RemoveAt(i);
            }
        }
    }

    private bool SpawnDueGesos(KingGeso owner)
    {
        float interval = owner.WaveGesoInterval;
        while (_spawnedCount < owner.WaveGesoCount && _spawnElapsed >= interval)
        {
            _spawnElapsed -= interval;
            if (!SpawnNextGeso(owner))
            {
                return false;
            }
        }

        return true;
    }

    private bool SpawnNextGeso(KingGeso owner)
    {
        Entity geso = owner.SpawnGeso();
        if (geso == null)
        {
            owner.ChangeState(new KingGesoCooldownState());
            return false;
        }

        _pendingGesos.Add(geso);
        _spawnedCount++;
        return true;
    }
}

//==========================================-
// キングゲソのクールダウン状態
//==========================================
internal sealed class KingGesoCooldownState : IKingGesoState
{
    private float _elapsed;

    public void Enter(KingGeso owner)
    {
        // クールダウン状態に入った時点で経過時間をリセット
        _elapsed = 0.0f;
    }

    public void Update(KingGeso owner)
    {
        _elapsed += Time.deltaTime;
        if (_elapsed >= owner.CooldownDuration)
        {
            // クールダウン終了後、待機状態に遷移
            owner.ChangeState(new KingGesoIdleState());
        }
    }

    public void Exit(KingGeso owner)
    {
    }
}
