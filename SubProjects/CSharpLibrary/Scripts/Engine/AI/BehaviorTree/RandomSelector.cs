using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// 子ノードをランダムな順序で実行し、「一つでも成功すればSuccessを返す」コンポジットノード。
/// 標準の Selector と異なり、実行開始時に子の順序をシャッフルするため、
/// 毎回異なる行動を選択させたい場合（ボスのランダムな攻撃など）に使用される。
/// </summary>
public class RandomSelector : CompositeNode
{
    protected override NodeStatus Execute(Blackboard blackboard, Entity owner)
    {
        uint startTimeKey = BehaviorTreeLoader.HashString("RandSelStart_" + NodeIdHash);
        uint indicesKey = BehaviorTreeLoader.HashString("RandIndices_" + NodeIdHash);
        uint currentIdxKey = BehaviorTreeLoader.HashString("RandCurrentIdx_" + NodeIdHash);

        // 1. 初期化：子のインデックスをシャッフルして保持
        if (!blackboard.HasKey(startTimeKey))
        {
            blackboard.SetFloat(startTimeKey, Time.time);
            
            List<int> indices = Enumerable.Range(0, children.Count).ToList();
            Random rnd = new Random(Guid.NewGuid().GetHashCode());
            indices = indices.OrderBy(x => rnd.Next()).ToList();
            
            // 簡易的にint配列を文字列にシリアライズして保存（BlackboardがList<int>をサポートしていない場合）
            string indicesStr = string.Join(",", indices);
            blackboard.SetString(indicesKey, indicesStr);
            blackboard.SetInt(currentIdxKey, 0);
        }

        string savedIndices = blackboard.GetString(indicesKey);
        int[] shuffledIndices = savedIndices.Split(',').Select(int.Parse).ToArray();
        int currentProgressIdx = blackboard.GetInt(currentIdxKey);

        for (int i = currentProgressIdx; i < shuffledIndices.Length; i++)
        {
            int childIdx = shuffledIndices[i];
            blackboard.SetInt(currentIdxKey, i);

            var status = children[childIdx].Tick(blackboard, owner);

            switch (status)
            {
                case NodeStatus.Success:
                    ClearState(blackboard);
                    return NodeStatus.Success;

                case NodeStatus.Running:
                    return NodeStatus.Running;

                case NodeStatus.Failure:
                    continue;
            }
        }

        ClearState(blackboard);
        return NodeStatus.Failure;
    }

    private void ClearState(Blackboard blackboard)
    {
        blackboard.Remove(BehaviorTreeLoader.HashString("RandSelStart_" + NodeIdHash));
        blackboard.Remove(BehaviorTreeLoader.HashString("RandIndices_" + NodeIdHash));
        blackboard.Remove(BehaviorTreeLoader.HashString("RandCurrentIdx_" + NodeIdHash));
    }

    public override void OnAbort(Blackboard blackboard, Entity owner)
    {
        ClearState(blackboard);
    }
}
