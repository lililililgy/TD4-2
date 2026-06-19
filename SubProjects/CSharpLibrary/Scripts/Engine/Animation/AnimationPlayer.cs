using System;
using System.Runtime.CompilerServices;

namespace ONEngine {
    public class AnimationPlayer : Component {
        public void Play() => Internal_Play(nativeHandle);
        public void Pause() => Internal_Pause(nativeHandle);
        public void Stop() => Internal_Stop(nativeHandle);
        public void SetClip(string path) => Internal_SetClip(nativeHandle, path);

        public bool IsPlaying {
            get => Internal_GetIsPlaying(nativeHandle);
            set { if (value) Play(); else Pause(); }
        }

        public float CurrentTime {
            get => Internal_GetCurrentTime(nativeHandle);
            set => Internal_SetCurrentTime(nativeHandle, value);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Play(ulong nativeHandle);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Pause(ulong nativeHandle);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Stop(ulong nativeHandle);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetClip(ulong nativeHandle, string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetIsPlaying(ulong nativeHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetCurrentTime(ulong nativeHandle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCurrentTime(ulong nativeHandle, float time);
    }
}
