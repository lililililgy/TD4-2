using System;

/// <summary>
/// Blackboard上のVector3座標に向かって移動を試みるアクションノード。
/// SimpleEQSなどで算出した目標地点への移動に使用する。
/// </summary>
public class MoveToPosNode : BehaviorNode
{
    /// <summary>
    /// 目標座標が格納されているBlackboardのキー。
    /// </summary>
    [BlackboardKey]
    public string posKey = "MoveToPos";

    /// <summary>
    /// ターゲットにどれだけ近づいたら移動を完了（Success）とみなすかの距離。
    /// </summary>
    public float stopDistance = 1.0f;

    protected override NodeStatus Execute(Blackboard blackboard, Entity owner)
    {
        uint key = BehaviorTreeLoader.HashString(posKey);
        if (!blackboard.HasKey(key))
        {
            return NodeStatus.Failure;
        }

        Vector3 targetPos = blackboard.GetVector3(key);
        Vector3 ownerPos = owner.transform.position;

        // 距離を計算
        Vector3 diff = targetPos - ownerPos;
        float distance = diff.Length();

        if ((int)(Time.time * 2) % 10 == 0) {
        }

        // 1. 到着判定
        if (distance <= stopDistance)
        {
            var intent = owner.GetComponent<AgentIntentComponent>();
            if (intent != null)
            {
                intent.desiredMoveDirection = Vector3.zero;
            }
            return NodeStatus.Success;
        }

        // 2. 移動方向の設定
        var aiIntent = owner.GetComponent<AgentIntentComponent>();
        if (aiIntent != null)
        {
            Vector3 dir = diff.Normalized();
            aiIntent.desiredMoveDirection = dir;
        }

        return NodeStatus.Running;
    }

    public override void OnAbort(Blackboard blackboard, Entity owner)
    {
        // 中断時は足を止める
        var intent = owner.GetComponent<AgentIntentComponent>();
        if (intent != null)
        {
            intent.desiredMoveDirection = Vector3.zero;
        }
    }
}

