using System;

/// <summary>
/// Blackboardの値を直接書き換えるユーティリティタスク。
/// フェーズ遷移やフラグ操作をBT内で行うために使用する。
/// </summary>
public class SetBBValueNode : BehaviorNode
{
    [BlackboardKey]
    public string keyName = "";

    public enum ValueType { Int, Float, Bool, String }
    public ValueType type = ValueType.Int;

    public string value = "0";

    protected override NodeStatus Execute(Blackboard blackboard, Entity owner)
    {
        uint key = BehaviorTreeLoader.HashString(keyName);
        if (key == 0) return NodeStatus.Failure;

        try
        {
            switch (type)
            {
                case ValueType.Int:
                    blackboard.SetInt(key, int.Parse(value));
                    break;
                case ValueType.Float:
                    blackboard.SetFloat(key, float.Parse(value));
                    break;
                case ValueType.Bool:
                    blackboard.SetBool(key, bool.Parse(value));
                    break;
                case ValueType.String:
                    blackboard.SetString(key, value);
                    break;
            }
            return NodeStatus.Success;
        }
        catch
        {
            return NodeStatus.Failure;
        }
    }
}

