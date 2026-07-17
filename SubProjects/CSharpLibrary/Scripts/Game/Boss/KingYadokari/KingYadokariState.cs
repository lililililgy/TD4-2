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
            KingYadokariAttackTypeEnum attackType = owner.SelectAttackType();
            if (attackType == KingYadokariAttackTypeEnum.ShellBullet) {
                owner.ChangeState(new KingYadokariShellBulletAttackState());
                return;
            }

            owner.ChangeState(new KingYadokariGiantClawAttackState());
        }
    }

    public void Exit(KingYadokari owner) {
    }
}

internal sealed class KingYadokariGiantClawAttackState : IKingYadokariState {
    private float elapsed_;
    private bool attackStarted_;

    public void Enter(KingYadokari owner) {
        elapsed_ = 0.0f;
        attackStarted_ = false;
        owner.BeginAttackTell();
    }

    public void Update(KingYadokari owner) {
        elapsed_ += Time.deltaTime;
        if (!attackStarted_) {
            if (elapsed_ < owner.GiantClawTellDuration) {
                return;
            }

            attackStarted_ = owner.BeginGiantClawAttack();
            if (!attackStarted_) {
                owner.ChangeState(new KingYadokariRecoveryState(owner.GiantClawRecoveryDuration));
                return;
            }
        }

        if (elapsed_ < owner.GiantClawTellDuration + owner.GiantClawActiveDuration) {
            return;
        }

        owner.EndGiantClawAttack();
        owner.ChangeState(new KingYadokariRecoveryState(owner.GiantClawRecoveryDuration));
    }

    public void Exit(KingYadokari owner) {
        owner.EndGiantClawAttack();
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
            if (elapsed_ < owner.ShellBulletTellDuration) {
                return;
            }

            fired_ = true;
            owner.RestoreNormalVisual();
            if (!owner.FireShellBullet()) {
                owner.ChangeState(new KingYadokariRecoveryState(owner.ShellBulletRecoveryDuration));
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

        owner.ChangeState(new KingYadokariRecoveryState(owner.ShellBulletRecoveryDuration));
    }

    public void Exit(KingYadokari owner) {
    }
}

internal sealed class KingYadokariRecoveryState : IKingYadokariState {
    private float elapsed_;
    private readonly float duration_;

    public KingYadokariRecoveryState(float duration) {
        duration_ = duration;
    }

    public void Enter(KingYadokari owner) {
        elapsed_ = 0.0f;
        owner.RestoreNormalVisual();
    }

    public void Update(KingYadokari owner) {
        elapsed_ += Time.deltaTime;
        if (elapsed_ >= duration_) {
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
