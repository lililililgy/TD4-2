using System;

// MessageBus で流すゲームイベントの定義置き場。

// 敵(HPで死亡判定される Entity)が撃破された際に HP.Update() から発行される。
// KillCountObjective が撃破数のカウントに、BossDefeatObjective がボス撃破判定に購読する。
public class EnemyKilledEvent {
    public EnemyKilledEvent(string entityName) {
        this.entityName = entityName;
    }

    // 撃破された Entity の名前（ボス判定などの識別用）
    public readonly string entityName;
}
