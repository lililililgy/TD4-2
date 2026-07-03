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
    }
    public void Update(KingJellyfish owner)
    {
        elapsed += Time.deltaTime;
        if (owner.ConsumeAttackRequest() || elapsed >= owner.IdleDuration)
        {
            owner.ChangeState(new KingJellyfishAttackState());
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

    public void Enter(KingJellyfish owner)
    {
        elapsed = 0.0f;
        attackType = owner.SelectAttackType();
        chargeStarted = false;

        if (attackType == KingJellyfishAttackTypeEnum.ChargeAttack)
        {
            // 体当たり攻撃の準備
            owner.UpdateChargeTell();
        }
        else
        {
            // 全方位ビーム攻撃の準備
            owner.ChangeState(new KingJellyfishIdleState());
        }
    }

    public void Update(KingJellyfish owner)
    {
        elapsed += Time.deltaTime;
        if (attackType == KingJellyfishAttackTypeEnum.ChargeAttack)
        {
            UpdateChargeAttack(owner);
        }
    }

    public void Exit(KingJellyfish owner)
    {

    }

    private void UpdateChargeAttack(KingJellyfish owner)
    {
        float tellEndTime = owner.ChargeTellDuration;
        float moveEndTime = tellEndTime + owner.ChargeMoveDuration;
        float recoveryEndTime = moveEndTime + owner.ChargeRecoveryDuration;

        if (elapsed < tellEndTime)
        {
            owner.UpdateChargeTell();
            return;
        }

        if (elapsed < moveEndTime)
        {
            if (!chargeStarted)
            {
                chargeStarted = owner.BeginChargeAttack();
                if (!chargeStarted)
                {
                    owner.ChangeState(new KingJellyfishIdleState());
                    return;
                }
            }

            owner.UpdateChargeAttack(elapsed - tellEndTime);
            return;
        }

        if (elapsed < recoveryEndTime)
        {
            owner.UpdateChargeRecovery();
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
    public void Enter(KingJellyfish owner)
    {
        // 移動時の処理
    }
    public void Update(KingJellyfish owner)
    {

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
        // 死亡時の処理
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
