using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class BoxCollider2D : Component {
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct BatchData {
        public uint compId;
        public int enable;
        public Vector2 size;
        public int isTrigger;
        public float mass;
        public int useOwnerScale;
    }

    private BatchData batchData;

    public BatchData GetBatchData() {
        return batchData;
    }

    public override void SyncFromNative(string ecsGroupName) {
        if (nativeHandle == 0) return;

        BatchData[] batch = new BatchData[1];
        batch[0].compId = compId;
        ComponentBatchManager.InternalGetBatch(typeof(BoxCollider2D), batch, 1, ecsGroupName);

        this.enable = batch[0].enable;
        batchData.enable = batch[0].enable;
        batchData.size = batch[0].size;
        batchData.isTrigger = batch[0].isTrigger;
        batchData.mass = batch[0].mass;
        batchData.useOwnerScale = batch[0].useOwnerScale;
    }

    public void ApplyBatchData(BatchData data) {
        this.enable = data.enable;
        batchData.enable = data.enable;
        batchData.size = data.size;
        batchData.isTrigger = data.isTrigger;
        batchData.mass = data.mass;
        batchData.useOwnerScale = data.useOwnerScale;
    }

    public Vector2 size {
        get {
            return batchData.size;
        }
        set {
            batchData.size = value;
            if (nativeHandle != 0) {
                ComponentBatchManager.InternalSetBatch(typeof(BoxCollider2D), new BatchData[] {
                    new BatchData {
                        compId = compId,
                        size = value,
                        isTrigger = batchData.isTrigger,
                        mass = batchData.mass,
                        useOwnerScale = batchData.useOwnerScale
                    }
                }, 1, entity.ecsGroupName);
            }
        }
    }

    public bool isTrigger {
        get {
            return batchData.isTrigger != 0;
        }
        set {
            batchData.isTrigger = value ? 1 : 0;
            if (nativeHandle != 0) {
                ComponentBatchManager.InternalSetBatch(typeof(BoxCollider2D), new BatchData[] {
                    new BatchData {
                        compId = compId,
                        size = batchData.size,
                        isTrigger = batchData.isTrigger,
                        mass = batchData.mass,
                        useOwnerScale = batchData.useOwnerScale
                    }
                }, 1, entity.ecsGroupName);
            }
        }
    }

    public float mass {
        get {
            return batchData.mass;
        }
        set {
            batchData.mass = value;
            if (nativeHandle != 0) {
                ComponentBatchManager.InternalSetBatch(typeof(BoxCollider2D), new BatchData[] {
                    new BatchData {
                        compId = compId,
                        size = batchData.size,
                        isTrigger = batchData.isTrigger,
                        mass = value,
                        useOwnerScale = batchData.useOwnerScale
                    }
                }, 1, entity.ecsGroupName);
            }
        }
    }

    public bool useOwnerScale {
        get {
            return batchData.useOwnerScale != 0;
        }
        set {
            batchData.useOwnerScale = value ? 1 : 0;
            if (nativeHandle != 0) {
                ComponentBatchManager.InternalSetBatch(typeof(BoxCollider2D), new BatchData[] {
                    new BatchData {
                        compId = compId,
                        size = batchData.size,
                        isTrigger = batchData.isTrigger,
                        mass = batchData.mass,
                        useOwnerScale = batchData.useOwnerScale
                    }
                }, 1, entity.ecsGroupName);
            }
        }
    }
}
