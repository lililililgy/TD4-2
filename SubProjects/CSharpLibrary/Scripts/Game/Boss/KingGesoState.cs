
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
    private bool _attackStarted;

    public void Enter(KingGeso owner)
    {
        _elapsed = 0.0f;
        _attackStarted = false;

        if (!owner.SpawnGeso())
        {
            owner.ChangeState(new KingGesoCooldownState());
        }
    }

    public void Update(KingGeso owner)
    {
        if (!_attackStarted)
        {
            // 攻撃開始
            _attackStarted = owner.StartActiveGesoAttack();
        }

        _elapsed += Time.deltaTime;
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
