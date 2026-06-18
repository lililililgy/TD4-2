using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class SpriteRenderer : Component {
	[StructLayout(LayoutKind.Sequential)]
	public struct BatchData {
		public uint compId;
		public Vector4 color;
		public Vector2 textureSize;
		public UVTransform uvTransform;
	}

	BatchData batchData;
	public BatchData GetBatchData() {
		return batchData;
	}

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(SpriteRenderer), batch, 1, ecsGroupName);

		batchData.color = batch[0].color;
		batchData.textureSize = batch[0].textureSize;
		batchData.uvTransform = batch[0].uvTransform;
	}

	//public string meshPath {
	//	get {
	//		return InternalGetMeshName(nativeHandle);
	//	}
	//	set {
	//		InternalSetMeshName(nativeHandle, value);
	//	}
	//}

	public Vector4 color {
		get {
			return InternalGetColor(nativeHandle);
		}
		set {
			InternalSetColor(nativeHandle, value);
		}
	}


	/// -------------------------------------------
	/// internal methods
	/// -------------------------------------------

	//[MethodImpl(MethodImplOptions.InternalCall)]
	//static extern string InternalGetMeshName(ulong nativeHandle);

	//[MethodImpl(MethodImplOptions.InternalCall)]
	//static extern void InternalSetMeshName(ulong nativeHandle, string meshName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern Vector4 InternalGetColor(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetColor(ulong nativeHandle, Vector4 color);

}
