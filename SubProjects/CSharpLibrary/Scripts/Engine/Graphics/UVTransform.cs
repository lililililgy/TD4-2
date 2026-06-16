using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public struct UVTransform {
	public Vector2 position;
	public Vector2 scale;
	public float rotate;
	private float pad1;
	private float pad2;
	private float pad3;

	public static UVTransform identity {
		get {
			UVTransform uv = new UVTransform();
			uv.position = Vector2.zero;
			uv.scale = Vector2.one;
			uv.rotate = 0.0f;
			return uv;
		}
	}
}
