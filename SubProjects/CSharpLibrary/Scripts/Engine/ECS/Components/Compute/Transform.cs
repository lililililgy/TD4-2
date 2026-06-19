using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class TransformData {
	public Vector3 position;
	public Quaternion rotate;
	public Vector3 scale;
}

public class Transform : Component {
	public struct BatchData {
		public uint compId;
		public Vector3 position;
		public Quaternion rotate;
		public Vector3 scale;
		public Matrix4x4 matrix;
	}

	public Vector3 position = new Vector3(0f, 0f, 0f);
	public Vector3 worldPosition = Vector3.zero;
	public Quaternion rotate = Quaternion.identity;
	public Quaternion rotation { get => rotate; set => rotate = value; }
	public Vector3 scale = new Vector3(1f, 1f, 1f);
	public Matrix4x4 matrix = Matrix4x4.kIdentity;

    public Vector3 forward => rotate * Vector3.forward;
    public Vector3 up => rotate * Vector3.up;
    public Vector3 right => rotate * Vector3.right;
}
