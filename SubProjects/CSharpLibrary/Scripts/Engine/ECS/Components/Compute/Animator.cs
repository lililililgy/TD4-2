using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class Animator : Component {
    // コンパイル定数 (C++側と合わせる)
    public const int MaxLayers = 4;
    public const int MaxStatesPerLayer = 2;

    public struct AnimationState {
        public uint clipId;
        public float time;
        public float weight;
        public bool isLoop;
        public float playbackSpeed;
        public float prevTime;
    }

    public struct AnimationLayer {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = MaxStatesPerLayer)]
        public AnimationState[] states;
        public float weight;
        public uint boneMaskHash;
    }

    // C++側が固定長配列を持つのに対し、C#側は参照を持つ形になるが、
    // バッチ更新時にはポインタ経由で直接アクセスすることを想定。
    public AnimationLayer[] layers = new AnimationLayer[MaxLayers];

    public void Play(string clipName, uint layerIndex = 0) {
        Play(StringHash.Get(clipName), layerIndex);
    }

    public void Play(uint clipId, uint layerIndex = 0) {
        Internal_Play(nativeHandle, clipId, layerIndex);
    }

    public void CrossFade(string clipName, float duration, uint layerIndex = 0) {
        CrossFade(StringHash.Get(clipName), duration, layerIndex);
    }

    public void CrossFade(uint clipId, float duration, uint layerIndex = 0) {
        Internal_CrossFade(nativeHandle, clipId, duration, layerIndex);
    }

    public float GetAnimationDuration(string clipName) {
        return GetAnimationDuration(StringHash.Get(clipName));
    }

    public float GetAnimationDuration(uint clipId) {
        return Internal_GetAnimationDuration(nativeHandle, clipId);
    }

    public void SetPlaybackSpeed(float speed, uint layerIndex = 0) {
        Internal_SetPlaybackSpeed(nativeHandle, speed, layerIndex);
    }

    public void SetLoop(bool isLoop, uint layerIndex = 0) {
        Internal_SetLoop(nativeHandle, isLoop, layerIndex);
    }

    /// <summary>
    /// 指定した時間(秒)でアニメーションが完了するように再生速度を調整して再生します。
    /// </summary>
    public void PlayWithDuration(string clipName, float duration, uint layerIndex = 0) {
        uint clipId = StringHash.Get(clipName);
        float animDuration = GetAnimationDuration(clipId);
        if (animDuration > 0 && duration > 0) {
            float speed = animDuration / duration;
            Play(clipId, layerIndex);
            SetPlaybackSpeed(speed, layerIndex);
        } else {
            Play(clipId, layerIndex);
        }
    }

    /// <summary>
    /// 指定した時間(秒)でアニメーションが完了するように再生速度を調整してクロスフェード再生します。
    /// </summary>
    public void CrossFadeWithDuration(string clipName, float duration, float crossFadeDuration = 0.15f, uint layerIndex = 0) {
        uint clipId = StringHash.Get(clipName);
        float animDuration = GetAnimationDuration(clipId);
        if (animDuration > 0 && duration > 0) {
            float speed = animDuration / duration;
            CrossFade(clipId, crossFadeDuration, layerIndex);
            SetPlaybackSpeed(speed, layerIndex);
        } else {
            CrossFade(clipId, crossFadeDuration, layerIndex);
        }
    }

    // ----- Internal Calls -----
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_Play(ulong nativeHandle, uint clipId, uint layerIndex);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_CrossFade(ulong nativeHandle, uint clipId, float duration, uint layerIndex);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_SetPlaybackSpeed(ulong nativeHandle, float speed, uint layerIndex);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_SetLoop(ulong nativeHandle, bool isLoop, uint layerIndex);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float Internal_GetAnimationDuration(ulong nativeHandle, uint clipId);
}

// 文字列ハッシュユーティリティ (C#側)
public static class StringHash {
    public static uint Get(string str) {
        if (string.IsNullOrEmpty(str)) return 0;
        // FNV-1a hash
        uint hash = 2166136261u;
        foreach (char c in str) {
            hash ^= (uint)c;
            hash *= 16777619u;
        }
        return hash;
    }
}
