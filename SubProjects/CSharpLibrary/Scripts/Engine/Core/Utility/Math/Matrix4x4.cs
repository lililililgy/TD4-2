
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

[StructLayout(LayoutKind.Sequential)]
public struct Matrix4x4 {
	public float m00, m01, m02, m03;
	public float m10, m11, m12, m13;
	public float m20, m21, m22, m23;
	public float m30, m31, m32, m33;

	/// <summary>
	/// 単位行列
	/// </summary>
	public static readonly Matrix4x4 kIdentity = new Matrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

	public Matrix4x4(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33) {

		this.m00 = m00;
		this.m01 = m01;
		this.m02 = m02;
		this.m03 = m03;
		this.m10 = m10;
		this.m11 = m11;
		this.m12 = m12;
		this.m13 = m13;
		this.m20 = m20;
		this.m21 = m21;
		this.m22 = m22;
		this.m23 = m23;
		this.m30 = m30;
		this.m31 = m31;
		this.m32 = m32;
		this.m33 = m33;
	}


	/// =================================
	/// methods
	/// =================================


	static public Matrix4x4 Scale(Vector3 v) {
		return new Matrix4x4(
			v.x, 0, 0, 0,
			0, v.y, 0, 0,
			0, 0, v.z, 0,
			0, 0, 0, 1
		);
	}

	static public Matrix4x4 RotateX(float x) {
		return new Matrix4x4(
			1, 0, 0, 0,
			0, Mathf.Cos(x), Mathf.Sin(x), 0,
			0, -Mathf.Sin(x), Mathf.Cos(x), 0,
			0, 0, 0, 1
		);
	}

	static public Matrix4x4 RotateY(float y) {
		return new Matrix4x4(
			Mathf.Cos(y), 0, -Mathf.Sin(y), 0,
			0, 1, 0, 0,
			Mathf.Sin(y), 0, Mathf.Cos(y), 0,
			0, 0, 0, 1
		);
	}

	static public Matrix4x4 RotateZ(float z) {
		return new Matrix4x4(
			Mathf.Cos(z), Mathf.Sin(z), 0, 0,
			-Mathf.Sin(z), Mathf.Cos(z), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		);
	}


	static public Matrix4x4 Rotate(Vector3 v) {
		Matrix4x4 rx = RotateX(v.x);
		Matrix4x4 ry = RotateY(v.y);
		Matrix4x4 rz = RotateZ(v.z);

		// Combine the rotations: R = Rz * Ry * Rx
		return rx * ry * rz;
	}

	static public Matrix4x4 Rotate(Quaternion q) {
		if (Quaternion.Length(q) == 0.0f) {
			return new Matrix4x4();
		}
		Matrix4x4 result = new Matrix4x4();

		float ww = q.w * q.w;
		float xx = q.x * q.x;
		float yy = q.y * q.y;
		float zz = q.z * q.z;
		float wx = q.w * q.x;
		float wy = q.w * q.y;
		float wz = q.w * q.z;
		float xy = q.x * q.y;
		float xz = q.x * q.z;
		float yz = q.y * q.z;

		result.m00 = ww + xx - yy - zz;
		result.m01 = 2 * (xy + wz);
		result.m02 = 2 * (xz - wy);

		result.m10 = 2 * (xy - wz);
		result.m11 = ww - xx + yy - zz;
		result.m12 = 2 * (yz + wx);

		result.m20 = 2 * (xz + wy);
		result.m21 = 2 * (yz - wx);
		result.m22 = ww - xx - yy + zz;

		result.m33 = 1.0f;

		return result;
	}

	static public Matrix4x4 Translate(Vector3 v) {
		return new Matrix4x4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			v.x, v.y, v.z, 1
		);
	}


	static public Vector3 Transform(Vector3 v, Matrix4x4 m) {
		return new Vector3(
			m.m00 * v.x + m.m10 * v.y + m.m20 * v.z + m.m30,
			m.m01 * v.x + m.m11 * v.y + m.m21 * v.z + m.m31,
			m.m02 * v.x + m.m12 * v.y + m.m22 * v.z + m.m32
		);
	}


	static public Matrix4x4 Inverse(Matrix4x4 m) {
		float a = m.m00, b = m.m01, c = m.m02, d = m.m03;
		float e = m.m10, f = m.m11, g = m.m12, h = m.m13;
		float i = m.m20, j = m.m21, k = m.m22, l = m.m23;
		float m4 = m.m30, n = m.m31, o = m.m32, p = m.m33;

		float kp_lo = k * p - l * o;
		float jp_ln = j * p - l * n;
		float jo_kn = j * o - k * n;
		float ip_lm4 = i * p - l * m4;
		float io_km4 = i * o - k * m4;
		float in_jm4 = i * n - j * m4;

		float det =
			a * (f * kp_lo - g * jp_ln + h * jo_kn)
		  - b * (e * kp_lo - g * ip_lm4 + h * io_km4)
		  + c * (e * jp_ln - f * ip_lm4 + h * in_jm4)
		  - d * (e * jo_kn - f * io_km4 + g * in_jm4);

		if (Mathf.Abs(det) < 1e-8f) {
			// 逆行列が存在しない
			return Matrix4x4.kIdentity;
		}

		float invDet = 1.0f / det;

		Matrix4x4 r = new Matrix4x4();

		// 1 行目
		r.m00 = (f * kp_lo - g * jp_ln + h * jo_kn) * invDet;
		r.m01 = -(b * kp_lo - c * jp_ln + d * jo_kn) * invDet;
		r.m02 = (b * (g * p - h * o) - c * (f * p - h * n) + d * (f * o - g * n)) * invDet;
		r.m03 = -(b * (g * m4 - h * k) - c * (f * m4 - h * i) + d * (f * k - g * i)) * invDet;

		// 2 行目
		r.m10 = -(e * kp_lo - g * ip_lm4 + h * io_km4) * invDet;
		r.m11 = (a * kp_lo - c * ip_lm4 + d * io_km4) * invDet;
		r.m12 = -(a * (g * p - h * o) - c * (e * p - h * m4) + d * (e * o - g * m4)) * invDet;
		r.m13 = (a * (g * m4 - h * k) - c * (e * m4 - h * i) + d * (e * k - g * i)) * invDet;

		// 3 行目
		r.m20 = (e * jp_ln - f * ip_lm4 + h * in_jm4) * invDet;
		r.m21 = -(a * jp_ln - b * ip_lm4 + d * in_jm4) * invDet;
		r.m22 = (a * (f * p - h * n) - b * (e * p - h * m4) + d * (e * n - f * m4)) * invDet;
		r.m23 = -(a * (f * m4 - h * j) - b * (e * m4 - h * i) + d * (e * j - f * i)) * invDet;

		// 4 行目
		r.m30 = -(e * jo_kn - f * io_km4 + g * in_jm4) * invDet;
		r.m31 = (a * jo_kn - b * io_km4 + c * in_jm4) * invDet;
		r.m32 = -(a * (f * o - g * n) - b * (e * o - g * m4) + c * (e * n - f * m4)) * invDet;
		r.m33 = (a * (f * k - g * j) - b * (e * k - g * i) + c * (e * j - f * i)) * invDet;

		return r;
	}

	public static Matrix4x4 CreateLookToLH(Vector3 eye, Vector3 forward, Vector3 up) {
		Vector3 zAxis = Vector3.Normalize(forward); // +Z
		Vector3 xAxis = Vector3.Normalize(Vector3.Cross(up, zAxis)); // +X
		Vector3 yAxis = Vector3.Cross(zAxis, xAxis); // +Y

		return new Matrix4x4(
			xAxis.x, yAxis.x, zAxis.x, 0,
			xAxis.y, yAxis.y, zAxis.y, 0,
			xAxis.z, yAxis.z, zAxis.z, 0,
			-Vector3.Dot(xAxis, eye),
			-Vector3.Dot(yAxis, eye),
			-Vector3.Dot(zAxis, eye),
			1
		);
	}


	public static Matrix4x4 operator *(Matrix4x4 a, Matrix4x4 b) {
		Matrix4x4 result = new Matrix4x4();

		result.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20 + a.m03 * b.m30;
		result.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21 + a.m03 * b.m31;
		result.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22 + a.m03 * b.m32;
		result.m03 = a.m00 * b.m03 + a.m01 * b.m13 + a.m02 * b.m23 + a.m03 * b.m33;

		result.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20 + a.m13 * b.m30;
		result.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31;
		result.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32;
		result.m13 = a.m10 * b.m03 + a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33;

		result.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20 + a.m23 * b.m30;
		result.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31;
		result.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32;
		result.m23 = a.m20 * b.m03 + a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33;

		result.m30 = a.m30 * b.m00 + a.m31 * b.m10 + a.m32 * b.m20 + a.m33 * b.m30;
		result.m31 = a.m30 * b.m01 + a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31;
		result.m32 = a.m30 * b.m02 + a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32;
		result.m33 = a.m30 * b.m03 + a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33;

		return result;
	}
}

