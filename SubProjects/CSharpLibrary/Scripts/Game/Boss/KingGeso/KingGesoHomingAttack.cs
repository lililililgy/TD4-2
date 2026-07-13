using System.Collections.Generic;

//==========================================
// キングゲソの追尾弾攻撃状態
//==========================================

internal sealed class KingGesoHomingAttack : IKingGesoAttack {
    private float elapsed_;
    private int spawnedCount_;
    private bool failed_;
    private readonly List<Entity> pendingProjectiles_ = new List<Entity>();

    public void Enter(KingGeso owner) {
        elapsed_ = owner.HomingProjectileInterval;
        spawnedCount_ = 0;
        failed_ = false;
        pendingProjectiles_.Clear();
        SpawnDueProjectiles(owner);
    }

    public bool Update(KingGeso owner) {
        if (failed_) {
            return true;
        }

        StartPendingProjectiles(owner);
        elapsed_ += Time.deltaTime;
        SpawnDueProjectiles(owner);

        return failed_ ||
            (spawnedCount_ >= owner.HomingProjectileCount && pendingProjectiles_.Count == 0);
    }

    public void Exit(KingGeso owner) {
        for (int i = 0; i < pendingProjectiles_.Count; i++) {
            Entity projectile = pendingProjectiles_[i];
            if (projectile != null) {
                projectile.Destroy();
            }
        }
        pendingProjectiles_.Clear();
    }

    private void SpawnDueProjectiles(KingGeso owner) {
        float interval = owner.HomingProjectileInterval;
        while (spawnedCount_ < owner.HomingProjectileCount && elapsed_ >= interval) {
            elapsed_ -= interval;
            Entity projectile = owner.SpawnHomingProjectile();
            if (projectile == null) {
                failed_ = true;
                return;
            }

            pendingProjectiles_.Add(projectile);
            spawnedCount_++;
        }
    }

    private void StartPendingProjectiles(KingGeso owner) {
        for (int i = pendingProjectiles_.Count - 1; i >= 0; i--) {
            Entity projectile = pendingProjectiles_[i];
            if (projectile == null || owner.ShootHomingBullet(projectile)) {
                pendingProjectiles_.RemoveAt(i);
            }
        }
    }
}
