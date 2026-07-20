using System;

// 「敵を n 体倒せ」の目標。MessageBus の EnemyKilledEvent を購読して数える。
// targetEntityName_ を入れると、その名前の敵の撃破だけを数える。
// ボス撃破もこれで表現する（targetEntityName_ = "KingGeso", targetCount_ = 1 → 「0 / 1」）。
// 撃破判定はイベントで成立するため、フェーズ開始時点で対象が未スポーンでも検知できる。
public class KillCountObjective : CountObjective<EnemyKilledEvent> {

    // 撃破を数える対象の Entity 名。空なら任意の敵を数える
    [SerializeField] private string targetEntityName_ = "";

    protected override bool Accept(EnemyKilledEvent e) {
        return string.IsNullOrEmpty(targetEntityName_) || e.entityName == targetEntityName_;
    }
}
