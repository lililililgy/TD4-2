public enum YadokariClawStateType {
    Idle,
    Attacking,
    Returning,
    Destroyed
}

internal interface IYadokariClawState {
    YadokariClawStateType StateType { get; }
    void Enter(YadokariGiantClaw owner);
    void Update(YadokariGiantClaw owner);
    void Exit(YadokariGiantClaw owner);
}

internal sealed class YadokariClawIdleState : IYadokariClawState {
    public YadokariClawStateType StateType { get { return YadokariClawStateType.Idle; } }

    public void Enter(YadokariGiantClaw owner) {
        owner.ApplyIdle();
    }

    public void Update(YadokariGiantClaw owner) {
    }

    public void Exit(YadokariGiantClaw owner) {
    }
}

internal sealed class YadokariClawAttackingState : IYadokariClawState {
    private readonly Entity target_;
    private readonly float damage_;
    private readonly float speed_;
    private readonly float distance_;

    public YadokariClawAttackingState(Entity target, float damage, float speed, float distance) {
        target_ = target;
        damage_ = damage;
        speed_ = speed;
        distance_ = distance;
    }

    public YadokariClawStateType StateType { get { return YadokariClawStateType.Attacking; } }

    public void Enter(YadokariGiantClaw owner) {
        if (!owner.ApplyAttack(target_, damage_, speed_, distance_)) {
            owner.ChangeState(new YadokariClawReturningState());
        }
    }

    public void Update(YadokariGiantClaw owner) {
        if (owner.UpdateAttack()) {
            owner.ChangeState(new YadokariClawReturningState());
        }
    }

    public void Exit(YadokariGiantClaw owner) {
    }
}

internal sealed class YadokariClawReturningState : IYadokariClawState {
    private float elapsed_;

    public YadokariClawStateType StateType { get { return YadokariClawStateType.Returning; } }

    public void Enter(YadokariGiantClaw owner) {
        elapsed_ = 0.0f;
        owner.BeginReturn();
    }

    public void Update(YadokariGiantClaw owner) {
        elapsed_ += Time.deltaTime;
        float duration = owner.ReturnDuration;
        float progress = duration > 0.0f ? elapsed_ / duration : 1.0f;
        owner.UpdateReturn(progress);
        if (progress >= 1.0f) {
            owner.ChangeState(new YadokariClawIdleState());
        }
    }

    public void Exit(YadokariGiantClaw owner) {
    }
}

internal sealed class YadokariClawDestroyedState : IYadokariClawState {
    private float elapsed_;

    public YadokariClawStateType StateType { get { return YadokariClawStateType.Destroyed; } }

    public void Enter(YadokariGiantClaw owner) {
        elapsed_ = 0.0f;
        owner.ApplyDestroyed();
    }

    public void Update(YadokariGiantClaw owner) {
        elapsed_ += Time.deltaTime;
        if (elapsed_ >= owner.DestroyDuration) {
            owner.DestroyEntity();
        }
    }

    public void Exit(YadokariGiantClaw owner) {
    }
}
