using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class SkinMeshRenderer : Component {
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct BatchData {
		public uint compId;
		public int enable;
		public int isPlaying;
		public float animationTime;
		public float animationScale;
	}

	private BatchData batchData;

	public BatchData GetBatchData() {
		return batchData;
	}

	public string meshPath {
		get {
			return InternalGetMeshName(nativeHandle);
		}
		set {
			InternalSetMeshName(nativeHandle, value);
		}
	}

	public string texturePath {
		get {
			return InternalGetTexturePath(nativeHandle);
		}
		set {
			InternalSetTexturePath(nativeHandle, value);
		}
	}

	public bool isPlaying {
		get {
			return batchData.isPlaying != 0;
		}
		set {
			batchData.isPlaying = value ? 1 : 0;
		}
	}

	public float animationTime {
		get {
			return batchData.animationTime;
		}
		set {
			batchData.animationTime = value;
		}
	}

	public float animationScale {
		get {
			return batchData.animationScale;
		}
		set {
			batchData.animationScale = value;
		}
	}

	public TransformData GetJointTransform(string jointName) {
		Vector3 scale;
		Quaternion rotation;
		Vector3 translation;
		InternalGetJointTransform(nativeHandle, jointName, out scale, out rotation, out translation);

		TransformData jointTransform = new TransformData();
		jointTransform.scale = scale;
		jointTransform.rotate = rotation;
		jointTransform.position = translation;

		return jointTransform;
	}

	/// MeshPathのAccessor
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern string InternalGetMeshName(ulong nativeHandle);
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetMeshName(ulong nativeHandle, string meshName);

	/// TexturePathのAccessor
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern string InternalGetTexturePath(ulong nativeHandle);
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetTexturePath(ulong nativeHandle, string texturePath);

	/// JointTransformを取得
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalGetJointTransform(ulong nativeHandle, string jointName, out Vector3 s, out Quaternion q, out Vector3 t);

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(SkinMeshRenderer), batch, 1, ecsGroupName);

		this.enable = batch[0].enable;
		batchData.enable = batch[0].enable;
		batchData.isPlaying = batch[0].isPlaying;
		batchData.animationTime = batch[0].animationTime;
		batchData.animationScale = batch[0].animationScale;
	}
}