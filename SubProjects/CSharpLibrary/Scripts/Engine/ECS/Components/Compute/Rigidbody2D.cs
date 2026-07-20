using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class Rigidbody2D : Component {
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct BatchData {
        public uint compId;
        public int enable;
        public Vector2 velocity;
        public float mass;
        public float restitution;
        public int useGravity;
        public float gravityScale;
        public int freezeX;
        public int freezeY;
    }

    private BatchData batchData;

    public BatchData GetBatchData() {
        return batchData;
    }

    public override void SyncFromNative(string ecsGroupName) {
        if (nativeHandle == 0) return;

        BatchData[] batch = new BatchData[1];
        batch[0].compId = compId;
        ComponentBatchManager.InternalGetBatch(typeof(Rigidbody2D), batch, 1, ecsGroupName);

        this.enable = batch[0].enable;
        batchData.enable = batch[0].enable;
        batchData.velocity = batch[0].velocity;
        batchData.mass = batch[0].mass;
        batchData.restitution = batch[0].restitution;
        batchData.useGravity = batch[0].useGravity;
        batchData.gravityScale = batch[0].gravityScale;
        batchData.freezeX = batch[0].freezeX;
        batchData.freezeY = batch[0].freezeY;
    }

    public void ApplyBatchData(BatchData data) {
        this.enable = data.enable;
        batchData.enable = data.enable;
        batchData.velocity = data.velocity;
        batchData.mass = data.mass;
        batchData.restitution = data.restitution;
        batchData.useGravity = data.useGravity;
        batchData.gravityScale = data.gravityScale;
        batchData.freezeX = data.freezeX;
        batchData.freezeY = data.freezeY;
    }

    private void PushBatch() {
        if (nativeHandle != 0) {
            ComponentBatchManager.InternalSetBatch(typeof(Rigidbody2D), new BatchData[] {
                new BatchData {
                    compId = compId,
                    enable = this.enable,
                    velocity = batchData.velocity,
                    mass = batchData.mass,
                    restitution = batchData.restitution,
                    useGravity = batchData.useGravity,
                    gravityScale = batchData.gravityScale,
                    freezeX = batchData.freezeX,
                    freezeY = batchData.freezeY
                }
            }, 1, entity.ecsGroupName);
        }
    }

    public Vector2 velocity {
        get {
            return batchData.velocity;
        }
        set {
            batchData.velocity = value;
            PushBatch();
        }
    }

    public float mass {
        get {
            return batchData.mass;
        }
        set {
            batchData.mass = value;
            PushBatch();
        }
    }

    public float restitution {
        get {
            return batchData.restitution;
        }
        set {
            batchData.restitution = value;
            PushBatch();
        }
    }

    public bool useGravity {
        get {
            return batchData.useGravity != 0;
        }
        set {
            batchData.useGravity = value ? 1 : 0;
            PushBatch();
        }
    }

    public float gravityScale {
        get {
            return batchData.gravityScale;
        }
        set {
            batchData.gravityScale = value;
            PushBatch();
        }
    }

    public bool freezeX {
        get {
            return batchData.freezeX != 0;
        }
        set {
            batchData.freezeX = value ? 1 : 0;
            PushBatch();
        }
    }

    public bool freezeY {
        get {
            return batchData.freezeY != 0;
        }
        set {
            batchData.freezeY = value ? 1 : 0;
            PushBatch();
        }
    }
}
