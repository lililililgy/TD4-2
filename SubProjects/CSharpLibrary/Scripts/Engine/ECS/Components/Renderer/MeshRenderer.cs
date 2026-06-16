using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class MeshRenderer : Component {
	[StructLayout(LayoutKind.Sequential)]
	public struct BatchData {
		public uint compId;
		public Vector4 color;
		public uint postEffectFlags;
		public UVTransform uvTransform;
	}

	BatchData batchData;
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

	public Vector4 color {
		get {
			return batchData.color;
		}
		set {
			batchData.color = value;
		}
	}

	public uint postEffectFlags {
		get {
			return batchData.postEffectFlags;
		}
		set {
			batchData.postEffectFlags = value;
		}
	}

	/// -------------------------------------------
	/// internal methods
	/// -------------------------------------------

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern string InternalGetMeshName(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetMeshName(ulong nativeHandle, string meshName);

	//[MethodImpl(MethodImplOptions.InternalCall)]
	//static extern Vector4 InternalGetColor(ulong nativeHandle);

	//[MethodImpl(MethodImplOptions.InternalCall)]
	//static extern void InternalSetColor(ulong nativeHandle, Vector4 color);

	//[MethodImpl(MethodImplOptions.InternalCall)]
	//static extern uint InternalGetPostEffectFlags(ulong nativeHandle);

	//[MethodImpl(MethodImplOptions.InternalCall)]
	//static extern void InternalSetPostEffectFlags(ulong nativeHandle, uint flags);

}
