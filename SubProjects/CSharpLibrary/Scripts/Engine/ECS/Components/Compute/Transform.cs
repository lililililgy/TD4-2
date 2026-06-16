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

	public Vector3 position = new Vector3(0f, 0f, 0f);
	public Vector3 worldPosition = Vector3.zero;
	public Quaternion rotate = Quaternion.identity;
	public Vector3 scale = new Vector3(1f, 1f, 1f);

}
