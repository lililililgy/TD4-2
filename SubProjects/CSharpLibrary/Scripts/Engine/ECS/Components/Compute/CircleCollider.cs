using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class CircleCollider : Component {
    public float radius {
        get { return InternalGetRadius(nativeHandle); }
        set { InternalSetRadius(nativeHandle, value); }
    }

    public bool isTrigger {
        get { return InternalIsTriggerCircle(nativeHandle); }
        set { InternalSetTriggerCircle(nativeHandle, value); }
    }

    public float mass {
        get { return InternalGetMassCircle(nativeHandle); }
        set { InternalSetMassCircle(nativeHandle, value); }
    }

    public bool useOwnerScale {
        get { return InternalIsUseOwnerScaleCircle(nativeHandle); }
        set { InternalSetUseOwnerScaleCircle(nativeHandle, value); }
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float InternalGetMassCircle(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetMassCircle(ulong nativeHandle, float mass);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float InternalGetRadius(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetRadius(ulong nativeHandle, float radius);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool InternalIsTriggerCircle(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetTriggerCircle(ulong nativeHandle, bool trigger);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool InternalIsUseOwnerScaleCircle(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetUseOwnerScaleCircle(ulong nativeHandle, bool use);
}
