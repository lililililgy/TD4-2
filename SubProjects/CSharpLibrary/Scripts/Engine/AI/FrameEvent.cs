using System;
using System.Runtime.CompilerServices;

/// <summary>
/// C++側のイベントシステムへのインターフェース。
/// </summary>
public static class FrameEvent
{
    /// <summary>
    /// C++側のEventTypeと一致させる必要があります。
    /// </summary>
    public enum Type : byte
    {
        TestEvent = 0,
        NamedEvent = 1,
        Attack = 2,
        Effect = 3,
    }

    /// <summary>
    /// エンティティに関連するイベントをキューに追加します。
    /// </summary>
    public static void EnqueueEntityEvent(Type eventType, int entityId)
    {
        Internal_EnqueueEntityEvent(eventType, entityId);
    }

    /// <summary>
    /// 名前付きイベントをキューに追加します。
    /// </summary>
    public static void EnqueueNamedEvent(string eventName, int entityId)
    {
        Internal_EnqueueNamedEvent(eventName, entityId);
    }

    /// <summary>
    /// 攻撃（当たり判定生成）イベントをキューに追加します。
    /// </summary>
    public static void EnqueueAttackEvent(string attackName, int ownerId, float damage, float radius, float duration, float offsetForward, float offsetUp)
    {
        Internal_EnqueueAttackEvent(attackName, ownerId, damage, radius, duration, offsetForward, offsetUp);
    }

    /// <summary>
    /// エフェクト（パーティクル・視覚演出）イベントをキューに追加します。
    /// </summary>
    public static void EnqueueEffectEvent(string effectName, int entityId, float scale = 1.0f, float duration = 2.0f)
    {
        Internal_EnqueueEffectEvent(effectName, entityId, scale, duration);
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_EnqueueEntityEvent(Type eventType, int entityId);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_EnqueueNamedEvent(string eventName, int entityId);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_EnqueueAttackEvent(string attackName, int ownerId, float damage, float radius, float duration, float offsetForward, float offsetUp);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_EnqueueEffectEvent(string effectName, int entityId, float scale, float duration);
}
