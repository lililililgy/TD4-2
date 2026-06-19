using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class BoxCollider2D : Component {
    public Vector2 size {
        get { return InternalGetSizeBox2D(nativeHandle); }
        set { InternalSetSizeBox2D(nativeHandle, value); }
    }

    public bool isTrigger {
        get { return InternalIsTriggerBox2D(nativeHandle); }
        set { InternalSetTriggerBox2D(nativeHandle, value); }
    }

    public float mass {
        get { return InternalGetMassBox2D(nativeHandle); }
        set { InternalSetMassBox2D(nativeHandle, value); }
    }

    public bool useOwnerScale {
        get { return InternalIsUseOwnerScaleBox2D(nativeHandle); }
        set { InternalSetUseOwnerScaleBox2D(nativeHandle, value); }
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float InternalGetMassBox2D(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetMassBox2D(ulong nativeHandle, float mass);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector2 InternalGetSizeBox2D(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetSizeBox2D(ulong nativeHandle, Vector2 size);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool InternalIsTriggerBox2D(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetTriggerBox2D(ulong nativeHandle, bool trigger);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool InternalIsUseOwnerScaleBox2D(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetUseOwnerScaleBox2D(ulong nativeHandle, bool use);
}
