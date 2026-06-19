using System.Collections.Generic;

/// <summary>
/// 複数の子ノードを同時に（並行して）実行するコンポジットノード。
/// 「移動しながら射撃する」「敵を探しながら待機する」といった複合的な挙動を実現する。
/// 全ての子ノードが毎フレーム実行され、設定されたポリシーに基づいて成功・失敗を判定する。
/// </summary>
public class Parallel : CompositeNode
{
    /// <summary>
    /// 成功・失敗を判定するためのポリシー。
    /// One: 一つでも条件を満たせば完了
    /// All: 全てが条件を満たせば完了
    /// </summary>
    public enum Policy { One, All }

    /// <summary>
    /// 成功とみなす条件。
    /// One ならば「いずれかの子が成功」、All ならば「すべての子が成功」で自身も Success を返す。
    /// </summary>
    public Policy successPolicy = Policy.All;

    /// <summary>
    /// 失敗とみなす条件。
    /// One ならば「いずれかの子が失敗」、All ならば「すべての子が失敗」で自身も Failure を返す。
    /// </summary>
    public Policy failurePolicy = Policy.One;

    public Parallel() : base() { }
    public Parallel(params BehaviorNode[] nodes) : base(nodes) { }

    /// <summary>
    /// 並列実行のメインロジック。
    /// 全ての子ノードを順に評価し、それぞれの結果を集計する。
    /// </summary>
    protected override NodeStatus Execute(Blackboard blackboard, Entity owner)
    {
        int successCount = 0;
        int failureCount = 0;
        int runningCount = 0;

        for (int i = 0; i < children.Count; i++)
        {
            // この並列ノードの現在の実行サイクルにおいて、既に完了した子ノードの状態をチェック
            uint doneKey = BehaviorTreeLoader.HashString("ParChildDone_" + NodeIdHash + "_" + i);
            if (blackboard.HasKey(doneKey))
            {
                NodeStatus savedStatus = (NodeStatus)blackboard.GetInt(doneKey);
                if (savedStatus == NodeStatus.Success) successCount++;
                else failureCount++;
                continue;
            }

            // まだ完了していない子ノードのみ Tick を実行
            var status = children[i].Tick(blackboard, owner);
            
            if (status == NodeStatus.Success)
            {
                successCount++;
                blackboard.SetInt(doneKey, (int)NodeStatus.Success);
            }
            else if (status == NodeStatus.Failure)
            {
                failureCount++;
                blackboard.SetInt(doneKey, (int)NodeStatus.Failure);
            }
            else
            {
                runningCount++;
            }
        }

        NodeStatus finalStatus = NodeStatus.Running;

        // 1. 失敗判定のチェック
        if (failurePolicy == Policy.One && failureCount > 0) finalStatus = NodeStatus.Failure;
        else if (failurePolicy == Policy.All && failureCount == children.Count && children.Count > 0) finalStatus = NodeStatus.Failure;

        // 2. 成功判定のチェック
        if (finalStatus == NodeStatus.Running)
        {
            if (successPolicy == Policy.One && successCount > 0) finalStatus = NodeStatus.Success;
            else if (successPolicy == Policy.All && successCount == children.Count && children.Count > 0) finalStatus = NodeStatus.Success;
        }

        // 完了した場合、まだ Running 状態の子ノードをすべて中断させ、完了フラグを掃除する
        if (finalStatus != NodeStatus.Running)
        {
            AbortRunningChildren(blackboard, owner);
            ClearFinishedFlags(blackboard);
            return finalStatus;
        }

        return NodeStatus.Running;
    }

    public override void OnAbort(Blackboard blackboard, Entity owner)
    {
        AbortRunningChildren(blackboard, owner);
        ClearFinishedFlags(blackboard);
    }

    private void AbortRunningChildren(Blackboard blackboard, Entity owner)
    {
        foreach (var child in children)
        {
            if (child.LastStatus == NodeStatus.Running)
            {
                child.OnAbort(blackboard, owner);
            }
        }
    }

    private void ClearFinishedFlags(Blackboard blackboard)
    {
        for (int i = 0; i < children.Count; i++)
        {
            blackboard.Remove(BehaviorTreeLoader.HashString("ParChildDone_" + NodeIdHash + "_" + i));
        }
    }

    /// <summary>
    /// イベント駆動エンジン対応：並列ノードは子からの完了通知を個別に待つのではなく、
    /// 自身が ActiveNode となっている間、毎フレーム Execute を通じて全子を回す。
    /// </summary>
    public override NodeStatus OnChildCompleted(BehaviorNode child, NodeStatus status, Blackboard blackboard, Entity owner)
    {
        // 既に Execute 内で全子の Tick を回しているため、ここでのバブリングは
        // 再度 Execute を呼び出して全体の判定を最新に更新する。
        return Execute(blackboard, owner);
    }
}
