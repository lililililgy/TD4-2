using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class CircleCollider : Component {
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct BatchData {
		public uint compId;
		public int enable;
		public float radius;
		public int isTrigger;
		public float mass;
		public int useOwnerScale;
	}

	private BatchData batchData;

	public BatchData GetBatchData() {
		return batchData;
	}

	public float radius {
		get { return batchData.radius; }
		set { batchData.radius = value; }
	}

	public bool isTrigger {
		get { return batchData.isTrigger != 0; }
		set { batchData.isTrigger = value ? 1 : 0; }
	}

	public float mass {
		get { return batchData.mass; }
		set { batchData.mass = value; }
	}

	public bool useOwnerScale {
		get { return batchData.useOwnerScale != 0; }
		set { batchData.useOwnerScale = value ? 1 : 0; }
	}

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(CircleCollider), batch, 1, ecsGroupName);

		this.enable = batch[0].enable;
		batchData.enable = batch[0].enable;
		batchData.radius = batch[0].radius;
		batchData.isTrigger = batch[0].isTrigger;
		batchData.mass = batch[0].mass;
		batchData.useOwnerScale = batch[0].useOwnerScale;
	}
}
