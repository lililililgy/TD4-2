using System;

// 「当たると本体(頭)の共有ライフを削る」当たり判定の中継。
// 頭・胴体・触手が別 Entity で、ライフの実体は頭(Player)の HP が1つだけ持つ構成で使う。
// このスクリプトは HP の実体を持たず、所有者(Player)の HP を解決して公開するだけ。
// 攻撃側の AttackCollision が、相手が DamageRelay を持っていれば OwnerHp へダメージを転送する。
//
// 頭は攻撃判定(AttackCollision)を持つが DamageRelay は持たない＆HP は directlyDamageable_=false に
// するので、頭に攻撃が当たってもライフは削れない。胴体・触手にこれを付ける。
public class DamageRelay : MonoScript {

    [SerializeField] private string ownerName_ = "Player"; // ライフの持ち主(頭)の Entity 名

    private HP ownerHp_;

    public override void Initialize() {
        Entity owner = ecsGroup.FindEntity(ownerName_);
        ownerHp_ = owner != null ? owner.GetScript<HP>() : null;
    }

    // 転送先（所有者=頭）の共有 HP。未解決なら null。
    public HP OwnerHp {
        get { return ownerHp_; }
    }
}
