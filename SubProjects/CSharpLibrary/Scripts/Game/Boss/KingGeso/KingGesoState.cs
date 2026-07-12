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
    private float elapsed;

    /// <summary>
    /// 状態開始時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Enter(KingGeso owner)
    {
        elapsed = 0.0f;
    }

    /// <summary>
    /// 状態更新時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Update(KingGeso owner)
    {
        elapsed += Time.deltaTime;
        if (owner.ConsumeAttackRequest() || elapsed >= owner.IdleDuration)
        {
            // 待機時間が経過したか、攻撃要求があった場合、攻撃状態に遷移
            owner.ChangeState(owner.CreateNextAttackState());
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
// キングゲソの追尾弾攻撃状態
//==========================================
internal sealed class KingGesoHomingAttackState : IKingGesoState
{
    private float _elapsed;
    private int _spawnedCount;
    private readonly List<Entity> _pendingProjectiles = new List<Entity>();

    public void Enter(KingGeso owner)
    {
        _elapsed = owner.HomingProjectileInterval;
        _spawnedCount = 0;
        _pendingProjectiles.Clear();
        SpawnDueProjectiles(owner);
    }

    public void Update(KingGeso owner)
    {
        StartPendingProjectiles(owner);
        _elapsed += Time.deltaTime;
        SpawnDueProjectiles(owner);

        if (_spawnedCount >= owner.HomingProjectileCount && _pendingProjectiles.Count == 0)
        {
            owner.ChangeState(new KingGesoCooldownState());
        }
    }

    public void Exit(KingGeso owner)
    {
    }

    private void SpawnDueProjectiles(KingGeso owner)
    {
        float interval = owner.HomingProjectileInterval;
        while (_spawnedCount < owner.HomingProjectileCount && _elapsed >= interval)
        {
            _elapsed -= interval;
            Entity projectile = owner.SpawnHomingProjectile();
            if (projectile == null)
            {
                owner.ChangeState(new KingGesoCooldownState());
                return;
            }

            _pendingProjectiles.Add(projectile);
            _spawnedCount++;
        }
    }

    private void StartPendingProjectiles(KingGeso owner)
    {
        for (int i = _pendingProjectiles.Count - 1; i >= 0; i--)
        {
            Entity projectile = _pendingProjectiles[i];
            if (projectile == null || owner.StartHomingProjectile(projectile))
            {
                _pendingProjectiles.RemoveAt(i);
            }
        }
    }
}

//==========================================-
// キングゲソの攻撃状態
//==========================================
internal sealed class KingGesoAttackState : IKingGesoState
{
    private IKingGesoAttack currentAttack_;

    public void Enter(KingGeso owner)
    {
        currentAttack_ = CreateAttack(owner.SelectAttackType());
        currentAttack_.Enter(owner);
    }

    public void Update(KingGeso owner)
    {
        if (currentAttack_ == null || currentAttack_.Update(owner))
        {
            //attack終了後、クールダウン状態に遷移
            owner.ChangeState(new KingGesoCooldownState());
        }
    }

    public void Exit(KingGeso owner)
    {
        if (currentAttack_ != null)
        {
            currentAttack_.Exit(owner);
            currentAttack_ = null;
        }

        // 攻撃終了時にアクティブなゲソを破棄
        owner.DestroyActiveGeso();
    }

    private IKingGesoAttack CreateAttack(KingGesoAttackType attackType)
    {
        if (attackType == KingGesoAttackType.InkBarrage)
        {
            return new KingGesoInkBarrageAttack();
        }

        if (attackType == KingGesoAttackType.PincerThrust)
        {
            return new KingGesoPincerThrustAttack();
        }

        return new KingGesoWaveThrustAttack();
    }
}

//==========================================-
// キングゲソのクールダウン状態
//==========================================
internal sealed class KingGesoCooldownState : IKingGesoState
{
    private float elapsed;

    public void Enter(KingGeso owner)
    {
        // クールダウン状態に入った時点で経過時間をリセット
        elapsed = 0.0f;
    }

    public void Update(KingGeso owner)
    {
        elapsed += Time.deltaTime;
        if (elapsed >= owner.CooldownDuration)
        {
            // クールダウン終了後、待機状態に遷移
            owner.ChangeState(new KingGesoIdleState());
        }
    }

    public void Exit(KingGeso owner)
    {
    }
}

//==========================================
// キングゲソの死亡状態
//==========================================
internal sealed class KingGesoDeadState : IKingGesoState
{
    public void Enter(KingGeso owner)
    {
        // 死亡時の処理（必要に応じて追加）
        owner.DestroyActiveGeso();
    }
    public void Update(KingGeso owner)
    {
        // 死亡状態では特に更新処理は不要
    }
    public void Exit(KingGeso owner)
    {
        // 死亡状態からの遷移は通常ないため、特に処理は不要
    }
}
