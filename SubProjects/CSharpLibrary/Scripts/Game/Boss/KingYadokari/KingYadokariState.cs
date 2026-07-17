internal interface IKingYadokariState {
    void Enter(KingYadokari owner);
    void Update(KingYadokari owner);
    void Exit(KingYadokari owner);
}

internal sealed class KingYadokariIdleState : IKingYadokariState {
    private float elapsed_;

    public void Enter(KingYadokari owner) {
        elapsed_ = 0.0f;
        owner.RestoreNormalVisual();
    }

    public void Update(KingYadokari owner) {
        elapsed_ += Time.deltaTime;
        if (elapsed_ >= owner.IdleDuration) {
            owner.ChangeState(new KingYadokariShellBulletAttackState());
        }
    }

    public void Exit(KingYadokari owner) {
    }
}

internal sealed class KingYadokariShellBulletAttackState : IKingYadokariState {
    private float elapsed_;
    private bool fired_;

    public void Enter(KingYadokari owner) {
        elapsed_ = 0.0f;
        fired_ = false;
        owner.BeginAttackTell();
    }

    public void Update(KingYadokari owner) {
        elapsed_ += Time.deltaTime;
        if (!fired_) {
            if (elapsed_ < owner.AttackTellDuration) {
                return;
            }

            fired_ = true;
            owner.RestoreNormalVisual();
            if (!owner.FireShellBullet()) {
                owner.ChangeState(new KingYadokariRecoveryState());
                return;
            }
        }

        bool reflectedHit;
        if (!owner.ConsumeBulletResult(out reflectedHit)) {
            return;
        }

        if (reflectedHit) {
            owner.ChangeState(new KingYadokariKnockDownState());
            return;
        }

        owner.ChangeState(new KingYadokariRecoveryState());
    }

    public void Exit(KingYadokari owner) {
    }
}

internal sealed class KingYadokariRecoveryState : IKingYadokariState {
    private float elapsed_;

    public void Enter(KingYadokari owner) {
        elapsed_ = 0.0f;
        owner.RestoreNormalVisual();
    }

    public void Update(KingYadokari owner) {
        elapsed_ += Time.deltaTime;
        if (elapsed_ >= owner.AttackRecoveryDuration) {
            owner.ChangeState(new KingYadokariIdleState());
        }
    }

    public void Exit(KingYadokari owner) {
    }
}

internal sealed class KingYadokariKnockDownState : IKingYadokariState {
    private float elapsed_;

    public void Enter(KingYadokari owner) {
        elapsed_ = 0.0f;
        owner.BeginKnockDown();
    }

    public void Update(KingYadokari owner) {
        elapsed_ += Time.deltaTime;
        if (elapsed_ >= owner.KnockDownDuration) {
            owner.ChangeState(new KingYadokariGetUpState());
        }
    }

    public void Exit(KingYadokari owner) {
    }
}

internal sealed class KingYadokariGetUpState : IKingYadokariState {
    private float elapsed_;

    public void Enter(KingYadokari owner) {
        elapsed_ = 0.0f;
        owner.BeginGetUp();
    }

    public void Update(KingYadokari owner) {
        elapsed_ += Time.deltaTime;
        float duration = owner.GetUpDuration;
        float progress = duration > 0.0f ? elapsed_ / duration : 1.0f;
        owner.UpdateGetUp(progress);

        if (progress >= 1.0f) {
            owner.FinishGetUp();
            owner.ChangeState(new KingYadokariIdleState());
        }
    }

    public void Exit(KingYadokari owner) {
    }
}
