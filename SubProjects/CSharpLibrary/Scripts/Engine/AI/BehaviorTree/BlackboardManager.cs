using System;
using System.Collections.Generic;

/// <summary>
/// エンティティIDとそれに対応するBlackboardを紐付けるマネージャー。
/// C++側から特定のエンティティのAI状態（Blackboard）を操作するために使用される。
/// </summary>
public static class BlackboardManager
{
    private static readonly Dictionary<int, Blackboard> _blackboards = new Dictionary<int, Blackboard>();

    /// <summary>
    /// エンティティのBlackboardを登録する。
    /// </summary>
    public static void Register(int entityId, Blackboard blackboard)
    {
        _blackboards[entityId] = blackboard;
    }

    /// <summary>
    /// エンティティの登録を解除する。
    /// </summary>
    public static void Unregister(int entityId)
    {
        _blackboards.Remove(entityId);
    }

    /// <summary>
    /// エンティティIDからBlackboardを取得する。
    /// </summary>
    public static Blackboard Get(int entityId)
    {
        if (_blackboards.TryGetValue(entityId, out var bb)) return bb;
        return null;
    }

    /// <summary>
    /// C++側から呼び出し可能な、BlackboardのBool値を設定するメソッド。
    /// 主に演出完了通知（NotifyComplete）などで使用される。
    /// </summary>
    public static void SetBool(int entityId, string key, bool value)
    {
        var bb = Get(entityId);
        if (bb != null)
        {
            uint keyHash = BehaviorTreeLoader.HashString(key);
            bb.SetBool(keyHash, value);
        }
    }
}
