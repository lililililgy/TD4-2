using System.Runtime.CompilerServices;

public class ParticleSystem2D : Component {
    public void Emit(int count) {
        if (nativeHandle == 0 || count <= 0) {
            return;
        }

        InternalEmit(nativeHandle, count);
    }

    public void Stop() {
        if (nativeHandle == 0) {
            return;
        }

        InternalStop(nativeHandle);
    }

    public void SetBoxShape(Vector3 boxScale) {
        if (nativeHandle == 0) {
            return;
        }

        InternalSetBoxShape(nativeHandle, boxScale.x, boxScale.y, boxScale.z);
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalEmit(ulong nativeHandle, int count);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalStop(ulong nativeHandle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetBoxShape(ulong nativeHandle, float x, float y, float z);
}
