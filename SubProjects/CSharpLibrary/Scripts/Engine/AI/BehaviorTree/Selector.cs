/// <summary>
/// 子ノードを左から右へ順番に実行し、「一つでも成功すれば即座にSuccessを返す」コンポジットノード。
/// いわゆる「OR（論理和）」の挙動を示し、優先度の高い行動（攻撃など）から順に試し、
/// 失敗したら次の行動（移動など）を試す、というAIのメインブレイン（意思決定）によく使われる。
/// </summary>
public class Selector : CompositeNode
{
    public Selector() : base() { }
    public Selector(params BehaviorNode[] nodes) : base(nodes) { }

    /// <summary>
    /// 子ノードの実行ループ。
    /// 実行ポインタ（ActiveNode）を維持する設計に対応。
    /// セレクターは「優先度の高い方から試す」性質上、常に0番目の子から評価を試みる。
    /// </summary>
    protected override NodeStatus Execute(Blackboard blackboard, Entity owner)
    {
        uint indexKey = BehaviorTreeLoader.HashString("SelIndex_" + NodeIdHash);

        for (int i = 0; i < children.Count; i++)
        {
            // 現在評価中のインデックスを保存
            blackboard.SetInt(indexKey, i);
            
            // 子ノードを実行
            var status = children[i].Tick(blackboard, owner);
            
            switch (status)
            {
                // 一つでも成功すれば、インデックスをリセットしてSuccessを返す
                case NodeStatus.Success:
                    blackboard.SetInt(indexKey, 0);
                    return NodeStatus.Success;
                    
                // 実行中の場合は、次フレームでこのインデックスから再開するためにRunningを返す
                case NodeStatus.Running:
                    return NodeStatus.Running;
                    
                // 失敗した場合は、次の優先度の子ノードを試す
                case NodeStatus.Failure:
                    continue;
            }
        }

        // 全て失敗したらインデックスをリセットしてFailureを返す
        blackboard.SetInt(indexKey, 0);
        return NodeStatus.Failure;
    }

    public override NodeStatus OnChildCompleted(BehaviorNode child, NodeStatus status, Blackboard blackboard, Entity owner)
    {
        uint indexKey = BehaviorTreeLoader.HashString("SelIndex_" + NodeIdHash);
        int currentIndex = blackboard.GetInt(indexKey, 0);

        // 子が成功したなら、セレクター自身も成功として終了
        if (status == NodeStatus.Success)
        {
            blackboard.SetInt(indexKey, 0);
            return NodeStatus.Success;
        }

        // 失敗した場合は、現在のインデックスの次から再開する
        int nextIndex = currentIndex + 1;
        if (nextIndex >= children.Count)
        {
            blackboard.SetInt(indexKey, 0);
            return NodeStatus.Failure;
        }

        blackboard.SetInt(indexKey, nextIndex);
        return children[nextIndex].Tick(blackboard, owner);
    }
}
