using System;

// MessageBus で流すゲームイベントの定義置き場。

// 敵(HPで死亡判定される Entity)が撃破された際に HP.Update() から発行される。
// KillCountObjective が撃破数のカウント（ボス撃破の判定も含む）に購読する。
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
public class PlayerMovingEvent
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

// ObjectiveSystem がフェーズに突入した際に発行される（各 Objective の BeginObjective() 後）。
// フェーズごとの敵湧き切り替えやボス出現など、「そのフェーズの開始で何かする」側が購読する。
//
// 購読は必ず Awake() で行うこと。ObjectiveSystem.Initialize() が最初の1発を発行するため、
// Initialize() で購読するとエンティティの並び順次第で初回を取りこぼす
// （ECSGroup は CallAwake() → CallInitialize() の順に全エンティティを回す）。
public class PhaseBeganEvent
{
    public PhaseBeganEvent(string phaseName)
    {
        this.phaseName = phaseName;
    }

    // 突入したフェーズエンティティ名（例: "Phase_Boss"）
    public readonly string phaseName;
}

// ObjectiveSystem がフェーズを離脱した際に発行される。PhaseBeganEvent と必ず対になる
// （Began を出していないフェーズでは Ended も出ない）。進行終了（末尾のフェーズ完了）時にも出るため、
// 「フェーズ外では止める」側はこれを見て止められる。
public class PhaseEndedEvent
{
    public PhaseEndedEvent(string phaseName)
    {
        this.phaseName = phaseName;
    }

    // 離脱したフェーズエンティティ名
    public readonly string phaseName;
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
