using System;

// MessageBus で流すゲームイベントの定義置き場。

// 敵(HPで死亡判定される Entity)が撃破された際に HP.Update() から発行される。
// KillCountObjective が撃破数のカウントに、BossDefeatObjective がボス撃破判定に購読する。
public class EnemyKilledEvent
{
    public EnemyKilledEvent(string entityName)
    {
        this.entityName = entityName;
    }

    // 撃破された Entity の名前（ボス判定などの識別用）
    public readonly string entityName;
}

// プレイヤーが移動入力を入れ始めた（入力なし→ありのエッジ）際に PlayerMoveComponent が発行される。
// チュートリアルの「移動した」検知に購読する想定。押しっぱなし中は再発行されない。
public class PlayerMovedEvent
{
}

// プレイヤーが弾を実際に発射した（クールダウン・弾切れ等を抜けて弾が生成された）際に
// PlayerShotComponent が発行される。チュートリアルの「射撃した」検知に購読する想定。
public class PlayerShotEvent
{
}

// プレイヤーがダッシュ状態に遷移した際に PlayerDashState.OnEnter() から発行される。
// チュートリアルの「ダッシュした」検知に購読する想定。
public class PlayerDashedEvent
{
}

// LevelingComponent を持つエンティティ(Player/Roe 等)のレベルが上がった際に
// LevelingComponent.LevelUp() から発行される。発行元は entityName で識別する。
// 「レベルnに到達」目標用に、上がったあとの新レベル値を運ぶ。1レベル上がるごとに1回発行される。
public class LevelUpEvent
{
    public LevelUpEvent(string entityName, int newLevel)
    {
        this.entityName = entityName;
        this.newLevel = newLevel;
    }

    // 上がったあとのレベル値
    public readonly int newLevel;
    public readonly string entityName;
}
