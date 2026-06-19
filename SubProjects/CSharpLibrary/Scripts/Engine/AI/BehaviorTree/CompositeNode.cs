using System.Collections.Generic;

/// <summary>
/// 複数の子ノードを持つ「コンポジット（枝）ノード」の基底クラス。
/// Sequence（シーケンス）やSelector（セレクター）などはこれを継承して実装される。
/// ノード自身は状態を持たず、子ノードへの実行（Tick）の委譲と結果の集約を担当する。
/// </summary>
public abstract class CompositeNode : BehaviorNode
{
    /// <summary>
    /// このコンポジットノードにぶら下がっている子ノードのリスト。
    /// リストの順序（通常は左から右）がそのまま実行の優先順位となる。
    /// </summary>
    protected readonly List<BehaviorNode> children = new List<BehaviorNode>();

    /// <summary>
    /// デフォルトコンストラクタ。エディタからの動的ロード時に使用される。
    /// </summary>
    public CompositeNode() { }

    /// <summary>
    /// コードベースで直接ツリーを構築する際のための可変長引数コンストラクタ。
    /// </summary>
    /// <param name="nodes">追加する子ノード群</param>
    public CompositeNode(params BehaviorNode[] nodes)
    {
        foreach (var node in nodes)
        {
            AddChild(node);
        }
    }

    /// <summary>
    /// 子ノードを末尾に追加する。
    /// ツリーの初期化・ロード時に呼び出される。
    /// </summary>
    /// <param name="node">追加する子ノード</param>
    public void AddChild(BehaviorNode node)
    {
        if (node != null)
        {
            node.Parent = this;
            node.Tree = this.Tree;
            children.Add(node);
        }
    }

    /// <summary>
    /// 登録されているすべての子ノードのリストを取得する。
    /// 実行順序の確認やデバッグ表示（状態の再帰的収集）などに使用される。
    /// </summary>
    /// <returns>子ノードのリスト</returns>
    public List<BehaviorNode> GetChildren() => children;
}
