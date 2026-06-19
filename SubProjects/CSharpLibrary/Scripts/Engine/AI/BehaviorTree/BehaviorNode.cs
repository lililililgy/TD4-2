using System.Collections.Generic;

/// <summary>
/// ビヘイビアツリーを構成する全てのノードの基底クラス。
/// アクション（葉）、コンポジット（枝）など全てのノードはこれを継承する。
/// ノード自体は状態を持たない（ステートレス）設計とし、実行状態はBlackboardに依存させる。
/// </summary>
public abstract class BehaviorNode
{
    /// <summary>
    /// ノードの識別名（デバッグ・表示用）。
    /// </summary>
    public string name { get; set; }

    /// <summary>
    /// エディタ上で割り当てられたノードの一意なIDのハッシュ値。
    /// Blackboardでの状態保存や、C++側との同期キーとして使用される。
    /// </summary>
    public uint NodeIdHash { get; set; }

    /// <summary>
    /// エディタ上の生のノードID。
    /// </summary>
    public int NodeId { get; set; }

    /// <summary>
    /// このノードが最後に実行された際の結果ステータス（成功、失敗、実行中）。
    /// </summary>
    public NodeStatus LastStatus { get; protected set; } = NodeStatus.Failure;

    /// <summary>
    /// このノードが最後に実行（Tick）された際のツリー全体のTick回数。
    /// エディタでのデバッグ表示（現在実行中のパスかどうかの判定）に使用される。
    /// </summary>
    public uint LastTickCount { get; set; } = 0;

    /// <summary>
    /// デバッグ用のブレークポイントがこのノードに設定されているかどうか。
    /// trueの場合、Tick実行時にゲームが一時停止する。
    /// </summary>
    public bool HasBreakpoint { get; set; } = false;

    /// <summary>
    /// このノードにアタッチされているデコレーター（条件モジュール）のリスト。
    /// ノード本体の実行前後に評価される。
    /// </summary>
    public List<BehaviorDecorator> Decorators { get; } = new List<BehaviorDecorator>();

    /// <summary>
    /// このノードにアタッチされているサービス（定期実行モジュール）のリスト。
    /// ノードがアクティブな間、指定間隔で定期的に実行される。
    /// </summary>
    public List<BehaviorService> Services { get; } = new List<BehaviorService>();

    /// <summary>
    /// デコレーターをリストに追加する。
    /// </summary>
    public void AddDecorator(BehaviorDecorator decorator) => Decorators.Add(decorator);

    /// <summary>
    /// サービスをリストに追加する。
    /// </summary>
    public void AddService(BehaviorService service) => Services.Add(service);

    private BehaviorTree _tree;
    /// <summary>
    /// このノードが属しているビヘイビアツリーのインスタンス。
    /// </summary>
    public BehaviorTree Tree {
        get => _tree;
        set {
            _tree = value;
            // コンポジットノードの場合は子ノードにも再帰的に伝播させる
            if (this is CompositeNode composite) {
                foreach (var child in composite.GetChildren()) {
                    child.Tree = value;
                }
            }
        }
    }

    /// <summary>
    /// このノードの親ノード。ツリー構造を上に辿るために使用される。
    /// </summary>
    public BehaviorNode Parent { get; set; }

    /// <summary>
    /// 子ノードが完了（Success or Failure）した際に親ノードへ通知されるコールバック。
    /// コンポジットノードはこのメソッドをオーバーライドして、次に実行すべき子ノードを決定する。
    /// </summary>
    /// <param name="child">完了した子ノード</param>
    /// <param name="status">子ノードの実行結果</param>
    /// <param name="blackboard">共有記憶領域</param>
    /// <param name="owner">実行エンティティ</param>
    /// <returns>親ノードとしての新しい実行状態</returns>
    public virtual NodeStatus OnChildCompleted(BehaviorNode child, NodeStatus status, Blackboard blackboard, Entity owner)
    {
        return status;
    }

    /// <summary>
    /// ノードが実行中（Running）に他の優先度の高いノードや条件変更によって中断された際に呼び出される。
    /// 移動の停止やアニメーションのキャンセルなど、後片付け処理をここで実装する。
    /// </summary>
    /// <param name="blackboard">共有記憶領域</param>
    /// <param name="owner">実行エンティティ</param>
    public virtual void OnAbort(Blackboard blackboard, Entity owner)
    {
        // 既定では何もしない
    }

    /// <summary>
    /// ノードの実行エントリーポイント。
    /// 内部でサービス、デコレーター、そして本体ロジックを順に処理する。
    /// </summary>
    /// <param name="blackboard">AIの共有記憶領域</param>
    /// <param name="owner">このAIを実行しているエンティティ</param>
    /// <returns>実行結果（Success, Failure, Running）</returns>
    public NodeStatus Tick(Blackboard blackboard, Entity owner)
    {
        // Tick回数を更新（デバッグ表示用）
        if (Tree != null)
        {
            LastTickCount = Tree.TickCount;
        }

        // 0. ブレークポイントチェック
        // エディタでブレークポイントが設定されている場合、C++エンジン側に通知してゲームを一時停止させる
        if (HasBreakpoint)
        {
            Internal_OnBreakpointHit(NodeIdHash);
        }

        // 1. Services の実行 (Interval管理)
        // アタッチされているすべてのサービスをループし、指定された間隔（Interval）が経過していれば実行する
        float currentTime = Time.time;
        foreach (var service in Services)
        {
            // 各サービスごとに前回の実行時間をBlackboardに記録しておくためのキーを生成
            uint timeKey = BehaviorTreeLoader.HashString("LastSrvTick_" + service.NodeIdHash);
            float lastTick = blackboard.GetFloat(timeKey, -1.0f);
            
            // 前回実行時から Interval 以上経過していれば処理を走らせる
            if (currentTime - lastTick >= service.Interval)
            {
                service.OnTick(blackboard, owner);
                // 実行後、現在の時間を記録して次回実行までのクールダウンを開始する
                blackboard.SetFloat(timeKey, currentTime);
            }
        }

        // 2. Decorators の条件チェック (Pre-Condition)
        // アタッチされているデコレーターの条件をすべて評価する。
        // 一つでも条件を満たさない（CalculateCondition が false）ものがあれば、即座にノードの実行を失敗として終了する。
        foreach (var decorator in Decorators)
        {
            if (!decorator.CalculateCondition(blackboard, owner))
            {
                return LastStatus = NodeStatus.Failure;
            }
        }

        // 3. 本体ロジックの実行
        // 継承先のクラスで実装された具体的な処理（移動、攻撃など）を実行する。
        NodeStatus result = Execute(blackboard, owner);

        // 4. Decorators による結果の加工 (Post-Process)
        // ノード本体の実行結果をデコレーターに渡し、結果を書き換える（例：強制的にSuccessにする、条件を満たすまでRunningにする等）。
        foreach (var decorator in Decorators)
        {
            result = decorator.PostProcessStatus(result, blackboard);
        }

        // 最終的な結果を保存し、親ノードへ返す
        return LastStatus = result;
    }

    /// <summary>
    /// ノード固有の具体的なロジック。継承先のクラス（各種アクションノードなど）で必ず実装する。
    /// </summary>
    /// <param name="blackboard">AIの共有記憶領域</param>
    /// <param name="owner">このAIを実行しているエンティティ</param>
    /// <returns>ノードの処理結果</returns>
    protected abstract NodeStatus Execute(Blackboard blackboard, Entity owner);

    /// <summary>
    /// C++エンジン側へブレークポイントのヒットを通知する内部呼び出しメソッド。
    /// </summary>
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.InternalCall)]
    private static extern void Internal_OnBreakpointHit(uint nodeIdHash);
}
