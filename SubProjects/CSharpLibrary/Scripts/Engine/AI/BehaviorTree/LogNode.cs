using System;

/// <summary>
/// コンソールにメッセージを出力するデバッグ用タスク。
/// </summary>
public class LogNode : BehaviorNode
{
    public string message = "LogNode executed.";

    public LogNode() { }

    public LogNode(string message)
    {
        this.message = message;
    }

    protected override NodeStatus Execute(Blackboard blackboard, Entity owner)
    {
        return NodeStatus.Success;
    }
}

