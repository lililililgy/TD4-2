using System;

/// <summary>
/// ノードの実行を制御する「デコレーター（条件モジュール）」の基底クラス。
/// ノード（タスクやコンポジット）にアタッチされ、実行の可否を判定したり、
/// 実行結果を後から書き換えたりする機能を提供する。
/// UEのビヘイビアツリーにおける「Decorator」に相当する。
/// </summary>
public abstract class BehaviorDecorator
{
    /// <summary>
    /// エディタ上で割り当てられたデコレーターの一意なIDハッシュ値。
    /// Blackboardでの状態保存に使用される。
    /// </summary>
    public uint NodeIdHash { get; set; }

    /// <summary>
    /// Blackboardの監視による割り込み（Abort）のポリシー。
    /// Self: 条件が満たされなくなった瞬間に、自身がアタッチされているノードを強制終了する。
    /// None: 監視による割り込みを行わない。
    /// </summary>
    public ObserverAbortPolicy AbortPolicy { get; set; } = ObserverAbortPolicy.None;

    /// <summary>
    /// ノードを実行する直前に呼ばれ、実行の可否（条件を満たしているか）を判定する。
    /// これが false を返した場合、アタッチ先のノードは実行されず Failure となる。
    /// </summary>
    /// <param name="blackboard">AIの共有記憶領域</param>
    /// <param name="owner">このAIを実行しているエンティティ</param>
    /// <returns>条件を満たしている場合は true</returns>
    public abstract bool CalculateCondition(Blackboard blackboard, Entity owner);

    /// <summary>
    /// このデコレーターが監視すべき Blackboard の変数（キー）のハッシュ値を取得する。
    /// ツリーの初期化時に呼ばれ、このキーの値が変更された際に再評価がトリガーされる。
    /// 監視が不要な場合は 0 を返す。
    /// </summary>
    /// <returns>監視対象のキーハッシュ値</returns>
    public virtual uint GetMonitoredKey() => 0;

    /// <summary>
    /// アタッチ先ノードの実行完了後に呼ばれ、その実行結果（NodeStatus）を加工する。
    /// 例：ForceSuccessDecoratorなら常にSuccessを返す、LoopDecoratorなら回数に応じてRunningを返すなど。
    /// </summary>
    /// <param name="currentStatus">ノード本体が返した実行結果</param>
    /// <param name="blackboard">AIの共有記憶領域</param>
    /// <returns>加工後の最終的な実行結果</returns>
    public virtual NodeStatus PostProcessStatus(NodeStatus currentStatus, Blackboard blackboard) => currentStatus;
}

/// <summary>
/// ノードが実行されている間、バックグラウンドで定期的な処理を行う「サービス」モジュールの基底クラス。
/// 索敵処理や、Blackboard変数の継続的な更新などに使用される。
/// UEのビヘイビアツリーにおける「Service」に相当する。
/// </summary>
public abstract class BehaviorService
{
    /// <summary>
    /// エディタ上で割り当てられたサービスの一意なIDハッシュ値。
    /// タイマー管理のキー生成などに使用される。
    /// </summary>
    public uint NodeIdHash { get; set; }

    /// <summary>
    /// サービスが実行される間隔（秒）。
    /// 例えば 0.5f に設定すると、ノード実行中であっても 0.5秒に1回だけ OnTick が呼ばれるため、処理負荷を軽減できる。
    /// </summary>
    public float Interval { get; set; } = 0.5f;

    /// <summary>
    /// 指定された Interval が経過するたびに呼び出される定期更新処理。
    /// 継承先のクラスで索敵ロジックなどを実装する。
    /// </summary>
    /// <param name="blackboard">AIの共有記憶領域</param>
    /// <param name="owner">このAIを実行しているエンティティ</param>
    public abstract void OnTick(Blackboard blackboard, Entity owner);
}
