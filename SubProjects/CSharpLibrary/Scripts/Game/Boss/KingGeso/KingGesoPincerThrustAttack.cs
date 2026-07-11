using System.Collections.Generic;

internal sealed class KingGesoPincerThrustAttack : IKingGesoAttack
{
    private float elapsed_;
    private readonly List<Entity> pendingGesos_ = new List<Entity>();

    public void Enter(KingGeso owner)
    {
        elapsed_ = 0.0f;
        pendingGesos_.Clear();

        List<Entity> spawnedGesos = new List<Entity>();
        if (!owner.SpawnPincerGesos(spawnedGesos))
        {
            return;
        }

        for (int i = 0; i < spawnedGesos.Count; i++)
        {
            pendingGesos_.Add(spawnedGesos[i]);
        }
    }

    public bool Update(KingGeso owner)
    {
        StartPendingGesoAttacks(owner);

        elapsed_ += Time.deltaTime;
        return elapsed_ >= owner.AttackDuration;
    }

    public void Exit(KingGeso owner)
    {
        pendingGesos_.Clear();
    }

    private void StartPendingGesoAttacks(KingGeso owner)
    {
        for (int i = pendingGesos_.Count - 1; i >= 0; i--)
        {
            Entity pendingGeso = pendingGesos_[i];
            if (pendingGeso == null || owner.StartGesoAttack(pendingGeso, KingGesoAttackType.PincerThrust))
            {
                pendingGesos_.RemoveAt(i);
            }
        }
    }
}
