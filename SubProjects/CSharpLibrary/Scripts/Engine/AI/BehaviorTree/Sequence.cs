/// <summary>
/// 子ノードを左から右へ順番に実行し、「全てが成功したらSuccessを返す」コンポジットノード。
/// いわゆる「AND（論理積）」の挙動を示し、
/// 「敵に近づく」→「攻撃モーションを再生」→「待機」といった、一連の決まった手順（シーケンス）をこなす際によく使われる。
/// </summary>
public class Sequence : CompositeNode
{
    public Sequence() : base() { }
    public Sequence(params BehaviorNode[] nodes) : base(nodes) { }

    /// <summary>
    /// 子ノードの実行ループ。
    /// 実行ポインタ（ActiveNode）を維持するイベント駆動設計に対応し、
    /// Running状態から再開した場合は、中断されていたインデックスから処理を開始する。
    /// </summary>
    protected override NodeStatus Execute(Blackboard blackboard, Entity owner)
    {
        // 実行中の子ノードのインデックスをBlackboardから取得（なければ0から開始）
        uint indexKey = BehaviorTreeLoader.HashString("SeqIndex_" + NodeIdHash);
        int startIndex = blackboard.GetInt(indexKey, 0);

        for (int i = startIndex; i < children.Count; i++)
        {
            // 現在実行中のインデックスを保存（途中中断やRunning時に備える）
            blackboard.SetInt(indexKey, i);
            
            // 子ノードを実行
            var status = children[i].Tick(blackboard, owner);
            
            switch (status)
            {
                // 手順の途中で失敗したら、即座に終了しインデックスをリセット
                case NodeStatus.Failure:
                    blackboard.SetInt(indexKey, 0);
                    return NodeStatus.Failure;
                    
                // 実行中の場合は、次フレームでこのインデックスから再開するためにRunningを返す
                case NodeStatus.Running:
                    return NodeStatus.Running;
                    
                // 成功したら次の子ノードへ進む
                case NodeStatus.Success:
                    continue;
            }
        }

        // 全て成功したらインデックスをリセットしてSuccessを返す
        blackboard.SetInt(indexKey, 0);
        return NodeStatus.Success;
    }

    public override NodeStatus OnChildCompleted(BehaviorNode child, NodeStatus status, Blackboard blackboard, Entity owner)
    {
        uint indexKey = BehaviorTreeLoader.HashString("SeqIndex_" + NodeIdHash);
        int currentIndex = blackboard.GetInt(indexKey, 0);

        if (status == NodeStatus.Failure)
        {
            blackboard.SetInt(indexKey, 0);
            return NodeStatus.Failure;
        }

        // 次の手順へ
        int nextIndex = currentIndex + 1;
        if (nextIndex >= children.Count)
        {
            blackboard.SetInt(indexKey, 0);
            return NodeStatus.Success;
        }

        blackboard.SetInt(indexKey, nextIndex);
        return children[nextIndex].Tick(blackboard, owner);
    }
}
