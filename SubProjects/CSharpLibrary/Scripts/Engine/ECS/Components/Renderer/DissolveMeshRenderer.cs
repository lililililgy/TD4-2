using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public class DissolveMeshRenderer : Component {
	[StructLayout(LayoutKind.Sequential)]
	public struct BatchData {
		public uint compId;
		public float threshold;
		public UVTransform uvTransform;
	}

	public float threshold = 0.0f;
	public UVTransform uvTransform = UVTransform.identity;
}
