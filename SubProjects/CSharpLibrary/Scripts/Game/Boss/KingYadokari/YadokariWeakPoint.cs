using System;

public class YadokariWeakPoint : MonoScript {
    [SerializeField] private string ownerEntityName_ = "KingYadokari";

    private KingYadokari owner_;

    public override void Initialize() {
        ResolveOwner();
    }

    public void Damage(float damage) {
        if (owner_ == null) {
            ResolveOwner();
        }

        if (owner_ != null) {
            owner_.TakeWeakPointDamage(damage);
        }
    }

    private void ResolveOwner() {
        owner_ = entity.GetScript<KingYadokari>();
        if (owner_ != null || String.IsNullOrEmpty(ownerEntityName_)) {
            return;
        }

        Entity ownerEntity = ecsGroup.FindEntity(ownerEntityName_);
        owner_ = ownerEntity != null ? ownerEntity.GetScript<KingYadokari>() : null;
    }
}
