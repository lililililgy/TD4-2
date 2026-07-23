using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

/// <summary>
/// キングクラゲの状態(基底クラス)
/// </summary>
internal interface IKingJellyfishState
{
    void Enter(KingJellyfish owner);
    void Update(KingJellyfish owner);
    void Exit(KingJellyfish owner);
}

//==========================================-
// キングクラゲの待機状態クラス
//==========================================
internal sealed class KingJellyfishIdleState : IKingJellyfishState
{
    private float elapsed;
    public void Enter(KingJellyfish owner)
    {
        elapsed = 0.0f;
        owner.ResetActionLoop();
    }
    public void Update(KingJellyfish owner)
    {
        elapsed += Time.deltaTime;
        if (owner.ConsumeAttackRequest() || elapsed >= owner.IdleDuration)
        {
            owner.ChangeState(new KingJellyfishMoveState());
        }
    }
    public void Exit(KingJellyfish owner)
    {
    }
}

//==========================================
// キングクラゲの攻撃状態クラス
//==========================================
internal sealed class KingJellyfishAttackState : IKingJellyfishState
{
    private float elapsed;
    private KingJellyfishAttackTypeEnum attackType;
    private bool chargeStarted;
    private bool chargeRecovered;
    private float recoveryElapsed;
    private bool laserFired;

    public void Enter(KingJellyfish owner)
    {
        elapsed = 0.0f;
        attackType = owner.SelectAttackType();
        chargeStarted = false;
        chargeRecovered = false;
        recoveryElapsed = 0.0f;
        laserFired = false;

        if (attackType == KingJellyfishAttackTypeEnum.ChargeAttack)
        {
            owner.UpdateChargeTell();
        }
        else
        {
            owner.UpdateLaserTell(attackType);
        }

        owner.ShowAttackTelegraph(attackType);
    }

    public void Update(KingJellyfish owner)
    {
        elapsed += Time.deltaTime;
        if (attackType == KingJellyfishAttackTypeEnum.ChargeAttack)
        {
            UpdateChargeAttack(owner);
            return;
        }

        if (attackType == KingJellyfishAttackTypeEnum.Omnidirectional_Beam)
        {
            UpdateOmnidirectionalLaser(owner);
            return;
        }

        if (attackType == KingJellyfishAttackTypeEnum.ElectricField)
        {
            UpdateElectricField(owner);
            return;
        }

        if (attackType == KingJellyfishAttackTypeEnum.RotatingBeam)
        {
            UpdateRotatingLaser(owner);
        }
    }

    public void Exit(KingJellyfish owner)
    {
        owner.HideAttackTelegraph();
        owner.EnsureSpriteOpaque();
        if (attackType == KingJellyfishAttackTypeEnum.ChargeAttack)
        {
            owner.EndChargeAttackMovement();
            owner.SetWeakPointCollisionEnabled(true);
        }
    }

    private void UpdateChargeAttack(KingJellyfish owner)
    {
        float tellEndTime = owner.ChargeTellDuration;
        float moveEndTime = tellEndTime + owner.ChargeMoveDuration;

        if (elapsed < tellEndTime)
        {
            owner.UpdateChargeTell();
            return;
        }

        if (!chargeRecovered && elapsed < moveEndTime)
        {
            if (!chargeStarted)
            {
                // 体当たり攻撃の開始
                chargeStarted = owner.BeginChargeAttack();
                if (!chargeStarted)
                {
                    owner.ChangeState(new KingJellyfishIdleState());
                    return;
                }
            }

            chargeRecovered = owner.UpdateChargeAttack();
            if (chargeRecovered)
            {
                owner.SetWeakPointCollisionEnabled(true);
            }
            return;
        }

        chargeRecovered = true;
        owner.SetWeakPointCollisionEnabled(true);
        if (chargeStarted)
        {
            // ダメージフィールドの展開
            owner.DeployChargeDamageField();
        }

        recoveryElapsed += Time.deltaTime;
        if (recoveryElapsed < owner.ChargeRecoveryDuration)
        {
            owner.UpdateChargeRecovery(recoveryElapsed);
            return;
        }

        if (owner.ConsumeActionLoop())
        {
            owner.ChangeState(new KingJellyfishMoveState());
            return;
        }

        owner.ChangeState(new KingJellyfishIdleState());
    }

    //==========================================
    // キングクラゲの全方位ビーム攻撃状態の更新処理
    //==========================================
    private void UpdateOmnidirectionalLaser(KingJellyfish owner)
    {
        float tellEndTime = owner.LaserTellDuration;
        float fireEndTime = tellEndTime + owner.LaserFireDuration;
        float recoveryEndTime = fireEndTime + owner.LaserRecoveryDuration;

        // ビーム攻撃の準備中
        if (elapsed < tellEndTime)
        {
            owner.UpdateLaserTell(KingJellyfishAttackTypeEnum.Omnidirectional_Beam);
            return;
        }

        // ビーム攻撃の発射中
        if (!laserFired)
        {
            owner.FireOmnidirectionalLaser();
            laserFired = true;
        }

        if (elapsed < recoveryEndTime)
        {
            owner.UpdateLaserRecovery();
            return;
        }

        if (owner.ConsumeActionLoop())
        {
            owner.ChangeState(new KingJellyfishMoveState());
            return;
        }

        owner.ChangeState(new KingJellyfishIdleState());
    }

    //==========================================
    // キングクラゲの電撃フィールド攻撃状態の更新処理
    //==========================================
    private void UpdateElectricField(KingJellyfish owner)
    {
        if (elapsed < owner.ElectricFieldTellDuration)
        {
            // 電撃フィールド攻撃の準備中
            owner.UpdateLaserTell(KingJellyfishAttackTypeEnum.ElectricField);
        }
        else
        {
            owner.HideAttackTelegraph();
        }

        if (!laserFired)
        {
            // 電撃フィールドの展開
            owner.DeployElectricFields();
            laserFired = true;
        }

        // 電撃フィールド攻撃の最終発動時間を計算
        float finalActivationTime = owner.ElectricFieldTellDuration
            + owner.ElectricFieldSpawnInterval * (owner.ElectricFieldCount - 1);

        // 電撃フィールド攻撃の終了時間を計算
        float attackEndTime = finalActivationTime
            + owner.ElectricFieldActiveDuration
            + owner.ElectricFieldRecoveryDuration;

        if (elapsed < attackEndTime)
        {
            return;
        }

        // 攻撃終了後の処理
        FinishAttack(owner);
    }

    //==========================================
    // キングクラゲの回転ビーム攻撃状態の更新処理
    //==========================================
    private void UpdateRotatingLaser(KingJellyfish owner)
    {
        float tellEndTime = owner.RotatingLaserTellDuration;
        float recoveryEndTime = tellEndTime
            + owner.RotatingLaserDuration
            + owner.RotatingLaserRecoveryDuration;

        if (elapsed < tellEndTime)
        {
            // 回転ビーム攻撃の準備中
            owner.UpdateLaserTell(KingJellyfishAttackTypeEnum.RotatingBeam);
            return;
        }

        if (!laserFired)
        {
            // 回転ビームの発射
            owner.FireRotatingLasers();
            laserFired = true;
        }

        if (elapsed < recoveryEndTime)
        {
            return;
        }

        // 攻撃終了後の処理
        FinishAttack(owner);
    }

    private void FinishAttack(KingJellyfish owner)
    {
        if (owner.ConsumeActionLoop())
        {
            owner.ChangeState(new KingJellyfishMoveState());
            return;
        }

        owner.ChangeState(new KingJellyfishIdleState());
    }

}

//==========================================
// キングクラゲの移動状態クラス
//==========================================
internal sealed class KingJellyfishMoveState : IKingJellyfishState
{
    private float elapsed;
    private bool moveFinished;
    private float inertiaElapsed;

    public void Enter(KingJellyfish owner)
    {
        elapsed = 0.0f;
        moveFinished = false;
        inertiaElapsed = 0.0f;
        owner.BeginArcMove();
    }

    public void Update(KingJellyfish owner)
    {
        if (!moveFinished)
        {
            elapsed += Time.deltaTime;
            moveFinished = owner.UpdateArcMove(elapsed);
            return;
        }

        inertiaElapsed += Time.deltaTime;
        if (inertiaElapsed < owner.MoveInertiaDuration)
        {
            owner.UpdateArcMoveInertia(inertiaElapsed);
            return;
        }

        owner.ChangeState(new KingJellyfishAttackState());
    }

    public void Exit(KingJellyfish owner)
    {

    }
}

//==========================================
// キングクラゲの死亡状態クラス
//==========================================
internal sealed class KingJellyfishDeadState : IKingJellyfishState
{
    public void Enter(KingJellyfish owner)
    {
        owner.BeginDeath();
    }
    public void Update(KingJellyfish owner)
    {
        // 死亡状態では特に更新処理は不要
    }
    public void Exit(KingJellyfish owner)
    {
        // 死亡状態からの遷移はないため、特に処理は不要
    }
}
