using System.Collections.Generic;

/// <summary>
/// キングゲソの「はさみ突き」攻撃の状態。
/// StartPendingGesoAttacks() で pendingGesos_ に溜まったゲソを順次攻撃開始する。
/// </summary>
internal sealed class KingGesoPincerThrustAttack : IKingGesoAttack
{
    private float elapsed_;
    private readonly List<Entity> pendingGesos_ = new List<Entity>();

    public void Enter(KingGeso owner)
    {
        elapsed_ = 0.0f;
        pendingGesos_.Clear();

        List<Entity> spawnedGesos = new List<Entity>();
        //挟み撃ち攻撃用のゲソを生成する
        if (!owner.SpawnPincerGesos(spawnedGesos))
        {
            return;
        }

        for (int i = 0; i < spawnedGesos.Count; i++)
        {
            // 生成したゲソを pendingGesos_ に追加する
            pendingGesos_.Add(spawnedGesos[i]);
        }
    }

    public bool Update(KingGeso owner)
    {
        StartPendingGesoAttacks(owner);

        elapsed_ += Time.deltaTime;
        return elapsed_ >= owner.PincerAttackDuration;
    }

    public void Exit(KingGeso owner)
    {
        pendingGesos_.Clear();
    }

    private void StartPendingGesoAttacks(KingGeso owner)
    {
        for (int i = pendingGesos_.Count - 1; i >= 0; i--)
        {
            // 
            Entity pendingGeso = pendingGesos_[i];
            if (pendingGeso == null || owner.StartGesoAttack(pendingGeso, KingGesoAttackType.PincerThrust))
            {
                pendingGesos_.RemoveAt(i);
            }
        }
    }
}
