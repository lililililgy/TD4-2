using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class TransformData {
	public Vector3 position;
	public Quaternion rotate;
	public Vector3 scale;
}

[StructLayout(LayoutKind.Sequential)]
public class Transform : Component {
	[StructLayout(LayoutKind.Sequential)]
	public struct BatchData {
		public uint compId;
		public Vector3 position;
		public Quaternion rotate;
		public Vector3 scale;
		public Matrix4x4 matWorld;
	}

	private Vector3 position_ = new Vector3(0f, 0f, 0f);
	public Vector3 position {
		get => position_;
		set {
			position_ = value;
			if (nativeHandle != 0) InternalSetPosition(nativeHandle, value.x, value.y, value.z);
		}
	}

	public Vector3 worldPosition = Vector3.zero;

	private Quaternion rotate_ = Quaternion.identity;
	public Quaternion rotate {
		get => rotate_;
		set {
			rotate_ = value;
			if (nativeHandle != 0) InternalSetRotate(nativeHandle, value.w, value.x, value.y, value.z);
		}
	}

	private Vector3 scale_ = new Vector3(1f, 1f, 1f);
	public Vector3 scale {
		get => scale_;
		set {
			scale_ = value;
			if (nativeHandle != 0) InternalSetScale(nativeHandle, value.x, value.y, value.z);
		}
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetPosition(ulong nativeHandle, float x, float y, float z);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetRotate(ulong nativeHandle, float w, float x, float y, float z);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetScale(ulong nativeHandle, float x, float y, float z);
}
