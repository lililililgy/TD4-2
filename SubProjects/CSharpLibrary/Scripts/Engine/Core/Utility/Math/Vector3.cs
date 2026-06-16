using System;
using System.Runtime.InteropServices;


[StructLayout(LayoutKind.Sequential)]
public struct Vector3 {
	public float x, y, z;
	
	
	
	/// =================================
	/// constructor
	/// =================================
	public Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) {
		this.x = x;
		this.y = y;
		this.z = z;
	}

	public Vector3(Vector3 other) {
		this.x = other.x;
		this.y = other.y;
		this.z = other.z;
	}

	/// =================================
	/// methods
	/// =================================
	public float Length() {
		return (float)System.Math.Sqrt(x * x + y * y + z * z);
	}

	public Vector3 Normalized() {
		float length = this.Length();
		if (length == 0.0f) return zero;
		return new Vector3(x / length, y / length, z / length);
	}

	/// =================================
	/// static methods
	/// =================================
	static public float Length(Vector3 v) {
		return v.Length();
	}

	static public Vector3 Normalize(Vector3 v) {
		return v.Normalized();
	}

	static public string ToSimpleString(Vector3 v) {
		return "(" + v.x + ", " + v.y + ", " + v.z + ")";
	}

	static public Vector3 LookAt(Vector3 from, Vector3 to) {
		Vector3 dir = to - from;
		float yaw = Mathf.Atan2(dir.x, dir.z);
		float pitch = Mathf.Atan2(-dir.y, Mathf.Sqrt(dir.x * dir.x + dir.z * dir.z));

		// roll（Z軸回転）は方向ベクトルからは求まらない（必要なら補助情報がいる）
		return new Vector3(pitch * Mathf.Rad2Deg, yaw * Mathf.Rad2Deg, 0f);
	}

	static public float Distance(Vector3 start, Vector3 end) {
		Vector3 dist = end - start;
		return dist.Length();
	}

	static public float Dot(Vector3 a, Vector3 b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static public Vector3 Cross(Vector3 a, Vector3 b) {
		return new Vector3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}

	static public Vector3 Lerp(Vector3 a, Vector3 b, float t) {
		return new Vector3(
			a.x + (b.x - a.x) * t,
			a.y + (b.y - a.y) * t,
			a.z + (b.z - a.z) * t
		);
	}

	/// =================================
	/// 定数
	/// =================================

	public static readonly Vector3 zero = new Vector3(0, 0, 0);
	public static readonly Vector3 one = new Vector3(1, 1, 1);
	public static readonly Vector3 up = new Vector3(0, 1, 0);
	public static readonly Vector3 down = new Vector3(0, -1, 0);
	public static readonly Vector3 left = new Vector3(-1, 0, 0);
	public static readonly Vector3 right = new Vector3(1, 0, 0);
	public static readonly Vector3 forward = new Vector3(0, 0, 1);
	public static readonly Vector3 back = new Vector3(0, 0, -1);
	

	/// =================================
	/// operators
	/// =================================
	public static Vector3 operator +(Vector3 a, Vector3 b) {
		return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
	}

	public static Vector3 operator -(Vector3 a, Vector3 b) {
		return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
	}

	public static Vector3 operator *(Vector3 a, float scalar) {
		return new Vector3(a.x * scalar, a.y * scalar, a.z * scalar);
	}

	public static Vector3 operator /(Vector3 a, float scalar) {
		return new Vector3(a.x / scalar, a.y / scalar, a.z / scalar);
	}

	static public Vector3 operator -(Vector3 v) {
		return new Vector3(-v.x, -v.y, -v.z);
	}
}