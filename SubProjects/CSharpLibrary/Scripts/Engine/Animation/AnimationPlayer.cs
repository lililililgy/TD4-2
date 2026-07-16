using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace ONEngine {
    public class AnimationPlayer : Component {
        [StructLayout(LayoutKind.Sequential, Pack = 4)]
        public struct BatchData {
            public uint compId;
            public int enable;
            public int isPlaying;
            public float currentTime;
        }

        private BatchData batchData;

        public BatchData GetBatchData() {
            return batchData;
        }

        public void Play() => Internal_Play(nativeHandle);
        public void Pause() => Internal_Pause(nativeHandle);
        public void Stop() => Internal_Stop(nativeHandle);
        public void SetClip(string path) => Internal_SetClip(nativeHandle, path);

        public bool IsPlaying {
            get => batchData.isPlaying != 0;
            set {
                batchData.isPlaying = value ? 1 : 0;
                if (value) Play(); else Pause();
            }
        }

        public float CurrentTime {
            get => batchData.currentTime;
            set => batchData.currentTime = value;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Play(ulong nativeHandle);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Pause(ulong nativeHandle);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Stop(ulong nativeHandle);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetClip(ulong nativeHandle, string path);

        public override void SyncFromNative(string ecsGroupName) {
            if (nativeHandle == 0) return;

            BatchData[] batch = new BatchData[1];
            batch[0].compId = compId;
            ComponentBatchManager.InternalGetBatch(typeof(AnimationPlayer), batch, 1, ecsGroupName);

            this.enable = batch[0].enable;
            batchData.enable = batch[0].enable;
            batchData.isPlaying = batch[0].isPlaying;
            batchData.currentTime = batch[0].currentTime;
        }
    }
}
