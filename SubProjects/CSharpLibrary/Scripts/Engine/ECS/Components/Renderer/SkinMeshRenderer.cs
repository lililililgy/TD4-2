using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;

public class SkinMeshRenderer : Component {

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
			return InternalGetIsPlaying(nativeHandle);
		}
		set {
			InternalSetIsPlaying(nativeHandle, value);
		}
	}


	public float animationTime {
		get {
			return InternalGetAnimationTime(nativeHandle);
		}
		set {
			InternalSetAnimationTime(nativeHandle, value);
		}
	}

	public float animationScale {
		get {
			return InternalGetAnimationScale(nativeHandle);
		}
		set {
			InternalSetAnimationScale(nativeHandle, value);
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

	/// IsPlayingのAccessor
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern bool InternalGetIsPlaying(ulong nativeHandle);
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetIsPlaying(ulong nativeHandle, bool isPlaying);

	/// AnimationTimeのAccessor
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern float InternalGetAnimationTime(ulong nativeHandle);
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetAnimationTime(ulong nativeHandle, float animationTime);

	/// AnimationScaleのAccessor
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern float InternalGetAnimationScale(ulong nativeHandle);
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetAnimationScale(ulong nativeHandle, float animationScale);

	/// JointTransformを取得
	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalGetJointTransform(ulong nativeHandle, string jointName, out Vector3 s, out Quaternion q, out Vector3 t);


}