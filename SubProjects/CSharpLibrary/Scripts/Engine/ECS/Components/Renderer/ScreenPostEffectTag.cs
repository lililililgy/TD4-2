using System.Runtime.CompilerServices;

public enum ScreenPostEffectType : int {
    Grayscale = 0,
    RadialBlur = 1,
    Fisheye = 2,
    WaterDistortion = 3,
    WaterDepthFogVignette = 4,
    WaterColorGrading = 5,
    WaterCausticsLightShafts = 6,
    Pixelate = 7,
}

public class ScreenPostEffectTag : Component {
    public bool IsEffectEnabled(ScreenPostEffectType type) {
        return nativeHandle != 0 && InternalGetPostEffectEnabled(nativeHandle, (int)type);
    }

    public void SetEffectEnabled(ScreenPostEffectType type, bool enabled) {
        if (nativeHandle == 0) {
            return;
        }

        InternalSetPostEffectEnabled(nativeHandle, (int)type, enabled);
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool InternalGetPostEffectEnabled(ulong nativeHandle, int type);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void InternalSetPostEffectEnabled(ulong nativeHandle, int type, bool enabled);
}
