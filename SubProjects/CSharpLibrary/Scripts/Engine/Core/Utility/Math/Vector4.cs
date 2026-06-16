using System;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public struct Vector4 {
	public float x, y, z, w;

	public Vector4(float x, float y, float z, float w) {
		this.x = x;
		this.y = y;
		this.z = z;
		this.w = w;
	}

	/// -----------------------------------------------
	/// public methods
	/// -----------------------------------------------
	public Vector4 Normalized() {
		float length = this.Length();
		if (length == 0.0f) {
			return zero;
		}
		return this / length;
	}

	public float Length() {
		return (float)Math.Sqrt(x * x + y * y + z * z + w * w);
	}

	/// -----------------------------------------------
	/// static public methods
	/// -----------------------------------------------

	static public Vector4 Normalized(Vector4 v) {
		return v.Normalized();
	}

	static public float Length(Vector4 v) {
		return v.Length();
	}

	static public Vector4 Lerp(Vector4 v1, Vector4 v2, float t) {
		Vector4 result = new Vector4();
		result.x = Mathf.Lerp(v1.x, v2.x, t);
		result.y = Mathf.Lerp(v1.y, v2.y, t);
		result.z = Mathf.Lerp(v1.z, v2.z, t);
		result.w = Mathf.Lerp(v1.w, v2.w, t);
		return result;
	}

	static public Vector4 ColorCodeToVector4(uint color) {
		float r = ((color >> 16) & 0xFF) / 255.0f;
		float g = ((color >> 8) & 0xFF) / 255.0f;
		float b = (color & 0xFF) / 255.0f;
		float a = ((color >> 24) & 0xFF) / 255.0f;
		return new Vector4(r, g, b, a);
	}

	/// -----------------------------------------------
	/// 定数
	/// -----------------------------------------------

	static public readonly Vector4 zero = new Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	static public readonly Vector4 one = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	static public readonly Vector4 red = new Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	static public readonly Vector4 green = new Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	static public readonly Vector4 blue = new Vector4(0.0f, 0.0f, 1.0f, 1.0f);
	static public readonly Vector4 negativeInfinity = new Vector4(float.NegativeInfinity,  float.NegativeInfinity, float.NegativeInfinity, float.NegativeInfinity);
	static public readonly Vector4 positiveInfinity = new Vector4(float.PositiveInfinity, float.PositiveInfinity, float.PositiveInfinity, float.PositiveInfinity);

	/// ------------------------------------------------
	/// operators
	/// ------------------------------------------------

	static public Vector4 operator +(Vector4 a, Vector4 b) {
		return new Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
	}

	static public Vector4 operator -(Vector4 a, Vector4 b) {
		return new Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
	}

	static public Vector4 operator *(Vector4 a, float b) {
		return new Vector4(a.x * b, a.y * b, a.z * b, a.w * b);
	}
	static public Vector4 operator /(Vector4 a, float b) {
		/// 例外処理、 inf予防
		if (b == 0.0f) {
			return zero; 
		}

		return new Vector4(a.x / b, a.y / b, a.z / b, a.w / b);
	}

	//static public bool operator ==(Vector4 a, Vector4 b) {
	//	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
	//}

	//static public bool operator !=(Vector4 a, Vector4 b) {
	//	return !(a == b);
	//}


}
