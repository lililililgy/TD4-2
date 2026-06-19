using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

/// <summary>
/// 1つのエンティティ（AIキャラクターなど）に関連付けられるビヘイビアツリーの実行単位。
/// ツリー全体のルートノード、共有記憶（Blackboard）、および監視ロジックの管理を行う。
/// </summary>
public class BehaviorTree
{
    private BehaviorNode _rootNode;
    /// <summary>
    /// ツリーの最上位（起点）となるノード。
    /// </summary>
    public BehaviorNode RootNode {
        get => _rootNode;
        set {
            _rootNode = value;
            if (_rootNode != null) {
                _rootNode.Tree = this;
            }
        }
    }

    /// <summary>
    /// このツリー内で共有される記憶領域。
    /// ターゲットの情報、フラグ、クールダウンのタイマーなどが保持される。
    /// </summary>
    public Blackboard Blackboard { get; } = new Blackboard();

    /// <summary>
    /// このビヘイビアツリーを実行している（アタッチされている）エンティティ。
    /// 移動処理やTransformの取得などに使用される。
    /// </summary>
    public Entity Owner { get; }

    private string _sourcePath;
    /// <summary>
    /// このツリーがロードされた元のJSONファイルのパス。
    /// エディタでのデバッグ表示のフィルタリングに使用される。
    /// </summary>
    public string SourcePath { 
        get => _sourcePath;
        set {
            if (value != null) {
                // パス形式を統一（スラッシュ、小文字化など）
                _sourcePath = value.Replace("\\", "/").Trim();
                if (_sourcePath.StartsWith("./")) _sourcePath = _sourcePath.Substring(2);
            } else {
                _sourcePath = null;
            }
        }
    }

    /// <summary>
    /// エディタ上の「Entry」ノードのID。
    /// デバッグ表示（フロー）のために使用される。
    /// </summary>
    public uint EntryNodeId { get; set; }

    /// <summary>
    /// Blackboardの変数が書き換わった際、ツリーの再評価（割り込み処理）が必要かどうかを示すフラグ
    /// </summary>
    private bool _reevaluateRequest = false;

    // Blackboardの特定のキー（変数）を監視しているデコレーターのリスト。
    // キーのハッシュ値を元に、どのデコレーターが反応すべきかを管理する。
    private readonly Dictionary<uint, List<BehaviorDecorator>> _monitoredDecorators = new Dictionary<uint, List<BehaviorDecorator>>();

    // 現在実行中のノード（Running状態のノード）へのポインタ。
    // イベント駆動エンジンとして、次フレームはここから再開する。
    public BehaviorNode ActiveNode { get; set; }

    /// <summary>
    /// サブツリーを含め、現在実際に実行されている最も深いノードを取得する。
    /// </summary>
    public BehaviorNode DeepestActiveNode
    {
        get
        {
            if (ActiveNode == null) return null;
            if (ActiveNode is RunBehaviorNode runBehavior && runBehavior.SubTree != null)
            {
                var deepest = runBehavior.SubTree.DeepestActiveNode;
                return deepest ?? ActiveNode;
            }
            return ActiveNode;
        }
    }

    /// <summary>
    /// 現在のTick回数。ノードが今フレーム実行されたかを判定するために使用される。
    /// </summary>
    public uint TickCount { get; set; } = 0;

    /// <summary>
    /// 新しいビヘイビアツリーのインスタンスを生成する。
    /// </summary>
    /// <param name="owner">ツリーを所有するエンティティ</param>
    public BehaviorTree(Entity owner)
    {
        Owner = owner;
        // Blackboardの値が変更されたイベントを購読し、変更時にツリーへ通知が行くように設定
        Blackboard.OnValueChanged += HandleBlackboardChanged;

        // BlackboardManagerに自身を登録（C++からの通知を受け取れるようにする）
        if (owner != null && owner.Id != 0)
        {
            BlackboardManager.Register(owner.Id, Blackboard);
        }
    }

    /// <summary>
    /// ツリー構造が決定（ロード）された後に、Decoratorの監視登録を初期化する。
    /// ツリー全体を走査し、ObserverAbortが設定されているデコレーターを抽出する。
    /// </summary>
    public void InitializeMonitoring()
    {
        _monitoredDecorators.Clear();
        if (RootNode == null) return;
        RegisterMonitoringRecursive(RootNode);
    }

    /// <summary>
    /// 再帰的にノードを辿り、監視が必要なデコレーターを _monitoredDecorators に登録する内部メソッド。
    /// </summary>
    /// <param name="node">現在走査中のノード</param>
    private void RegisterMonitoringRecursive(BehaviorNode node)
    {
        // ノードにアタッチされているすべてのデコレーターをチェック
        foreach (var decorator in node.Decorators)
        {
            // AbortPolicy (中断設定) が None 以外（SelfやLowerPriorityなど）の場合のみ監視対象とする
            if (decorator.AbortPolicy != ObserverAbortPolicy.None)
            {
                uint key = decorator.GetMonitoredKey();
                if (key != 0)
                {
                    // 監視リストにキーが存在しなければ新しくリストを作成
                    if (!_monitoredDecorators.ContainsKey(key)) _monitoredDecorators[key] = new List<BehaviorDecorator>();
                    
                    // そのキーの監視リストにデコレーターを追加
                    _monitoredDecorators[key].Add(decorator);
                }
            }
        }

        // CompositeNode (SequenceやSelector等、子を持つノード) の場合は、さらに子ノードを再帰的に走査
        if (node is CompositeNode composite)
        {
            foreach (var child in composite.GetChildren()) RegisterMonitoringRecursive(child);
        }
    }

    /// <summary>
    /// Blackboard内の変数値が変更された際に呼び出されるコールバック。
    /// </summary>
    /// <param name="keyHash">変更された変数のキーハッシュ</param>
    private void HandleBlackboardChanged(uint keyHash)
    {
        // 変更されたキーを監視しているデコレーターが存在するかチェック
        if (_monitoredDecorators.ContainsKey(keyHash))
        {
            // 監視対象の変数が変わったため、現在の実行パスが不正になった可能性がある。
            // 次回のTickでツリー全体の再評価（Abort処理）を要求する。
            _reevaluateRequest = true;
        }
    }

    /// <summary>
    /// 指定したノードとそのすべての子孫ノードに対して Abort 処理を再帰的に実行する。
    /// </summary>
    public void AbortRecursive(BehaviorNode node)
    {
        if (node == null) return;

        // 子ノード（コンポジットの場合）を先に中断
        if (node is CompositeNode composite)
        {
            foreach (var child in composite.GetChildren()) AbortRecursive(child);
        }

        // ノード本体の中断処理を呼び出し
        node.OnAbort(Blackboard, Owner);
    }

    /// <summary>
    /// 毎フレーム呼び出される、ビヘイビアツリーの実行エントリーポイント。
    /// </summary>
    public void Tick()
    {
        if (RootNode == null || Owner == null) return;

        // Tick回数をインクリメント
        TickCount++;

        // 1. 再評価リクエスト（割り込み）のチェック
        if (_reevaluateRequest)
        {
            // 割り込みが発生した場合、現在の実行ポイントを破棄して最初（Root）から評価し直す。
            if (ActiveNode != null)
            {
                AbortRecursive(RootNode);
            }
            ActiveNode = null;
            _reevaluateRequest = false;
        }

        // 2. 実行の開始
        NodeStatus status = RootNode.Tick(Blackboard, Owner);

        // 3. 実行中ノードの追跡
        ActiveNode = null;
        UpdateActiveNodeRecursive(RootNode);

        // ツリー全体が完了した場合はリセット
        if (status != NodeStatus.Running)
        {
            ActiveNode = null;
        }
    }

    /// <summary>
    /// ツリーを再帰的に探索し、現在Running状態にある最も深いノードを実行ポインタとして記録する。
    /// </summary>
    public void UpdateActiveNodeRecursive(BehaviorNode node)
    {
        if (node == null || node.LastStatus != NodeStatus.Running) return;

        ActiveNode = node; // より深いノードが見つかるたびに更新

        if (node is CompositeNode composite)
        {
            foreach (var child in composite.GetChildren())
            {
                if (child.LastStatus == NodeStatus.Running)
                {
                    UpdateActiveNodeRecursive(child);
                }
            }
        }
    }

    /// <summary>
    /// 全てのノードの直近の実行状態（Success/Failure/Running）を取得する。
    /// 主にエディタでのデバッグ可視化（ハイライト表示）に使用される。
    /// </summary>
    /// <param name="outStatuses">結果を格納するディクショナリ</param>
    public void GetAllNodeStatuses(Dictionary<uint, NodeStatus> outStatuses)
    {
        if (RootNode == null) return;

        // Entryノードの状態を報告
        if (EntryNodeId != 0)
        {
            // ルートがTickされたならEntryもアクティブとみなす
            bool wasRootTicked = (RootNode.LastTickCount == TickCount);
            Internal_UpdateNodeStatus(EntryNodeId, wasRootTicked ? (int)NodeStatus.Success : 4, SourcePath ?? "");
        }

        CollectStatusRecursive(RootNode, outStatuses);
    }

    /// <summary>
    /// ツリーを再帰的に辿り、実行状態を収集・C++エンジンへ通知する内部メソッド。
    /// </summary>
    /// <param name="node">現在走査中のノード</param>
    /// <param name="outStatuses">結果を格納するディクショナリ</param>
    private void CollectStatusRecursive(BehaviorNode node, Dictionary<uint, NodeStatus> outStatuses)
    {
        if (node == null) return;
        
        // 今フレーム実行されたかチェック
        bool wasTickedThisFrame = (node.LastTickCount == TickCount);
        int statusValue = wasTickedThisFrame ? (int)node.LastStatus : 4; // 4 は Inactive（非アクティブ）として扱う

        // C#側のディクショナリに状態を記録
        outStatuses[node.NodeIdHash] = wasTickedThisFrame ? node.LastStatus : NodeStatus.Failure;
        
        // C++エディタへ状態を通知（グラフ上のノード枠色をリアルタイムに変更するため）
        Internal_UpdateNodeStatus(node.NodeIdHash, statusValue, SourcePath ?? "");

        // 子ノードも再帰的に収集
        if (node is CompositeNode composite)
        {
            foreach (var child in composite.GetChildren())
            {
                CollectStatusRecursive(child, outStatuses);
            }
        }
        else if (node is RunBehaviorNode runBehavior)
        {
            // サブツリー内部の状態も同期対象にする
            if (runBehavior.SubTree != null)
            {
                runBehavior.SubTree.GetAllNodeStatuses(outStatuses);
            }
        }
    }

    /// <summary>
    /// C++エディタへノードの状態を送信するための内部呼び出し。
    /// </summary>
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_UpdateNodeStatus(uint nodeIdHash, int status, string treePath);
}
