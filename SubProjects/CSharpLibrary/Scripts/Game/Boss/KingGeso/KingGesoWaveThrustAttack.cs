using System.Collections.Generic;

internal sealed class KingGesoWaveThrustAttack : IKingGesoAttack
{
    private sealed class TimedGeso
    {
        public Entity entity;
        public float elapsed;
    }

    private float elapsed_;
    private float spawnElapsed_;
    private int spawnedCount_;
    private readonly List<Entity> pendingGesos_ = new List<Entity>();
    private readonly List<TimedGeso> activeGesos_ = new List<TimedGeso>();

    public void Enter(KingGeso owner)
    {
        elapsed_ = 0.0f;
        spawnElapsed_ = 0.0f;
        spawnedCount_ = 0;
        pendingGesos_.Clear();
        activeGesos_.Clear();

        SpawnNextGeso(owner);
    }

    public bool Update(KingGeso owner)
    {
        StartPendingGesoAttacks(owner);
        UpdateActiveGesoLifeTimes(owner);

        elapsed_ += Time.deltaTime;
        spawnElapsed_ += Time.deltaTime;

        if (elapsed_ < owner.WaveAttackDuration)
        {
            float interval = owner.WaveGesoInterval;
            while (spawnedCount_ < owner.WaveGesoCount && spawnElapsed_ >= interval)
            {
                spawnElapsed_ -= interval;
                if (!SpawnNextGeso(owner))
                {
                    return true;
                }
            }
        }

        return elapsed_ >= owner.WaveAttackDuration
            && pendingGesos_.Count == 0
            && activeGesos_.Count == 0;
    }

    public void Exit(KingGeso owner)
    {
        pendingGesos_.Clear();
        activeGesos_.Clear();
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
            if (pendingGeso == null)
            {
                pendingGesos_.RemoveAt(i);
                continue;
            }

            if (owner.StartGesoAttack(pendingGeso, KingGesoAttackType.WaveThrust))
            {
                activeGesos_.Add(new TimedGeso {
                    entity = pendingGeso,
                    elapsed = 0.0f,
                });
                pendingGesos_.RemoveAt(i);
            }
        }
    }

    private void UpdateActiveGesoLifeTimes(KingGeso owner)
    {
        for (int i = activeGesos_.Count - 1; i >= 0; i--)
        {
            TimedGeso timedGeso = activeGesos_[i];
            if (timedGeso == null || timedGeso.entity == null)
            {
                activeGesos_.RemoveAt(i);
                continue;
            }

            timedGeso.elapsed += Time.deltaTime;
            if (timedGeso.elapsed >= owner.WaveGesoLifeTime)
            {
                owner.DestroyActiveGeso(timedGeso.entity);
                activeGesos_.RemoveAt(i);
            }
        }
    }
}
