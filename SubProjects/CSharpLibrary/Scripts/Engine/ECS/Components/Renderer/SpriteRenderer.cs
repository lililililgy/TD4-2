using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;

class SpriteRenderer : Component {

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
