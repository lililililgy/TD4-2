using System.Runtime.CompilerServices;

public class ParticleSystem2D : Component {
    public void Emit(int count) {
        if (nativeHandle == 0 || count <= 0) {
            return;
        }

        InternalEmit(nativeHandle, count);
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalEmit(ulong nativeHandle, int count);
}
