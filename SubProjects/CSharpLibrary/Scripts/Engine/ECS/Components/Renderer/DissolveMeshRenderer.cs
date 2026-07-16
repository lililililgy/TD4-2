using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public class DissolveMeshRenderer : Component {
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct BatchData {
		public uint compId;
		public int enable;
		public float threshold;
		public UVTransform uvTransform;
	}

	public float threshold = 0.0f;
	public UVTransform uvTransform = UVTransform.identity;

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(DissolveMeshRenderer), batch, 1, ecsGroupName);

		this.enable = batch[0].enable;
		threshold = batch[0].threshold;
		uvTransform = batch[0].uvTransform;
	}
}
