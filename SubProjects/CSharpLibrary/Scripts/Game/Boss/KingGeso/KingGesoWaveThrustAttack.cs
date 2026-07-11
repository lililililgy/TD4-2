using System.Collections.Generic;

internal sealed class KingGesoWaveThrustAttack : IKingGesoAttack
{
    private float elapsed_;
    private float spawnElapsed_;
    private int spawnedCount_;
    private readonly List<Entity> pendingGesos_ = new List<Entity>();

    public void Enter(KingGeso owner)
    {
        elapsed_ = 0.0f;
        spawnElapsed_ = 0.0f;
        spawnedCount_ = 0;
        pendingGesos_.Clear();

        SpawnNextGeso(owner);
    }

    public bool Update(KingGeso owner)
    {
        StartPendingGesoAttacks(owner);

        elapsed_ += Time.deltaTime;
        spawnElapsed_ += Time.deltaTime;

        float interval = owner.WaveGesoInterval;
        while (spawnedCount_ < owner.WaveGesoCount && spawnElapsed_ >= interval)
        {
            spawnElapsed_ -= interval;
            if (!SpawnNextGeso(owner))
            {
                return true;
            }
        }

        return elapsed_ >= owner.AttackDuration;
    }

    public void Exit(KingGeso owner)
    {
        pendingGesos_.Clear();
    }

    private bool SpawnNextGeso(KingGeso owner)
    {
        Entity geso = owner.SpawnGeso();
        if (geso == null)
        {
            return false;
        }

        pendingGesos_.Add(geso);
        spawnedCount_++;
        return true;
    }

    private void StartPendingGesoAttacks(KingGeso owner)
    {
        for (int i = pendingGesos_.Count - 1; i >= 0; i--)
        {
            Entity pendingGeso = pendingGesos_[i];
            if (pendingGeso == null || owner.StartGesoAttack(pendingGeso, KingGesoAttackType.WaveThrust))
            {
                pendingGesos_.RemoveAt(i);
            }
        }
    }
}
