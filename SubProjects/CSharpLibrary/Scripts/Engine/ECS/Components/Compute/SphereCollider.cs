using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class SphereCollider : Component {
    public float radius {
        get { return InternalGetRadius(nativeHandle); }
        set { InternalSetRadius(nativeHandle, value); }
    }

    public bool isTrigger {
        get { return InternalIsTrigger(nativeHandle); }
        set { InternalSetTrigger(nativeHandle, value); }
    }

    public float mass {
        get { return InternalGetMass(nativeHandle); }
        set { InternalSetMass(nativeHandle, value); }
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float InternalGetMass(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetMass(ulong nativeHandle, float mass);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float InternalGetRadius(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetRadius(ulong nativeHandle, float radius);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool InternalIsTrigger(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetTrigger(ulong nativeHandle, bool trigger);
}
