using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public class CameraComponent : Component {
	[StructLayout(LayoutKind.Sequential)]
	public struct BatchData {
		public uint compId;
		public Matrix4x4 matVP;
		public Matrix4x4 matView;
		public Matrix4x4 matProjection;
		public float fovY;
		public float aspectRatio;
		public float nearClip;
		public float farClip;
		public int cameraType;
	}

	public float fovY = 45.0f;
	public float aspectRatio = 16.0f / 9.0f;
	public float nearClip = 0.1f;
	public float farClip = 1000.0f;

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(CameraComponent), batch, 1, ecsGroupName);

		fovY = batch[0].fovY;
		nearClip = batch[0].nearClip;
		farClip = batch[0].farClip;
	}
}
