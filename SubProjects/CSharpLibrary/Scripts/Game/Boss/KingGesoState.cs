internal interface IKingGesoState
{
    void Enter(KingGeso owner);
    void Update(KingGeso owner);
    void Exit(KingGeso owner);
}

internal sealed class KingGesoIdleState : IKingGesoState
{
    private float _elapsed;

    public void Enter(KingGeso owner)
    {
        _elapsed = 0.0f;
    }

    public void Update(KingGeso owner)
    {
        _elapsed += Time.deltaTime;
        if (owner.ConsumeAttackRequest() || _elapsed >= owner.IdleDuration)
        {
            owner.ChangeState(new KingGesoAttackState());
        }
    }

    public void Exit(KingGeso owner)
    {
    }
}

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
        // The spawned entity is initialized at the beginning of the next frame.
        if (!_attackStarted)
        {
            _attackStarted = owner.StartActiveGesoAttack();
        }

        _elapsed += Time.deltaTime;
        if (_elapsed >= owner.AttackDuration)
        {
            owner.ChangeState(new KingGesoCooldownState());
        }
    }

    public void Exit(KingGeso owner)
    {
        owner.DestroyActiveGeso();
    }
}

internal sealed class KingGesoCooldownState : IKingGesoState
{
    private float _elapsed;

    public void Enter(KingGeso owner)
    {
        _elapsed = 0.0f;
    }

    public void Update(KingGeso owner)
    {
        _elapsed += Time.deltaTime;
        if (_elapsed >= owner.CooldownDuration)
        {
            owner.ChangeState(new KingGesoIdleState());
        }
    }

    public void Exit(KingGeso owner)
    {
    }
}
