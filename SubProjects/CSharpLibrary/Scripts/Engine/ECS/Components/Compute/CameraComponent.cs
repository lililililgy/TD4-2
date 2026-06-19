using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class CameraComponent : Component {
	public enum CameraType : int {
		Type3D,
		Type2D,
	}

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
}
