using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class CameraComponent : Component {
	public enum CameraType : int {
		Type3D,
		Type2D,
	}

	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct BatchData {
		public uint compId;
		public Matrix4x4 matVP;
		public Matrix4x4 matView;
		public Matrix4x4 matProjection;

		public float fovY;
		public float nearClip;
		public float farClip;
		public int cameraType;
	}

	public Matrix4x4 matVP = Matrix4x4.kIdentity;
	public Matrix4x4 matView = Matrix4x4.kIdentity;
	public Matrix4x4 matProjection = Matrix4x4.kIdentity;

	public float fovY = 0.7f;
	public float nearClip = 0.1f;
	public float farClip = 1000.0f;
	public CameraType cameraType = CameraType.Type3D;

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(CameraComponent), batch, 1, ecsGroupName);

		this.matVP = batch[0].matVP;
		this.matView = batch[0].matView;
		this.matProjection = batch[0].matProjection;
		this.fovY = batch[0].fovY;
		this.nearClip = batch[0].nearClip;
		this.farClip = batch[0].farClip;
		this.cameraType = (CameraType)batch[0].cameraType;
	}
}
