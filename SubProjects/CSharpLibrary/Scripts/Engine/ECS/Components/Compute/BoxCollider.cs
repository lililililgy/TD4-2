using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class BoxCollider : Component {
    public Vector3 size {
        get { return InternalGetSize(nativeHandle); }
        set { InternalSetSize(nativeHandle, value); }
    }

    public bool isTrigger {
        get { return InternalIsTrigger(nativeHandle); }
        set { InternalSetTrigger(nativeHandle, value); }
    }

    public float mass {
        get { return InternalGetMassBox(nativeHandle); }
        set { InternalSetMassBox(nativeHandle, value); }
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float InternalGetMassBox(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetMassBox(ulong nativeHandle, float mass);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 InternalGetSize(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetSize(ulong nativeHandle, Vector3 size);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool InternalIsTrigger(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetTrigger(ulong nativeHandle, bool trigger);
}
