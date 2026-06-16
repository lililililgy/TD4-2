using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;


[StructLayout(LayoutKind.Sequential)]
public struct Quaternion {

	public float x, y, z, w;

	public Quaternion(float x, float y, float z, float w) {
		this.x = x;
		this.y = y;
		this.z = z;
		this.w = w;
	}

	/// -----------------------------------------
	/// static public 定数
	/// -----------------------------------------

	static public Quaternion identity {
		get {
			return new Quaternion(0, 0, 0, 1);
		}
	}


	/// -----------------------------------------
	/// static public methods
	/// -----------------------------------------

	static public float Length(Quaternion q) {
		return q.Length();
	}

	static public Quaternion Normalized(Quaternion q) {
		return q.Normalized();
	}

	static public Quaternion Conjugate(Quaternion q) {
		return q.Conjugate();
	}

	static public Quaternion Inverse(Quaternion q) {
		return q.Inverse();
	}

	static public Quaternion Lerp(Quaternion q1, Quaternion q2, float t) {
		// 線形補間
		return new Quaternion(
			Mathf.Lerp(q1.x, q2.x, t),
			Mathf.Lerp(q1.y, q2.y, t),
			Mathf.Lerp(q1.z, q2.z, t),
			Mathf.Lerp(q1.w, q2.w, t)
		);
	}

	static public Quaternion MakeFromAxis(Vector3 axis, float angle) {
		/// angle はラジアンで指定
		float halfAngle = angle * 0.5f;
		float sinHalfAngle = Mathf.Sin(halfAngle);

		Vector3 normalizedAxis = axis.Normalized();

		return new Quaternion(
			normalizedAxis.x * sinHalfAngle,
			normalizedAxis.y * sinHalfAngle,
			normalizedAxis.z * sinHalfAngle,
			Mathf.Cos(halfAngle)
		);
	}

	static public Quaternion LookAt(Vector3 position, Vector3 target, Vector3 up) {

		// forward
		Vector3 forward = Vector3.Normalize(target - position);

		// forward と up の平行対策
		float dot = Vector3.Dot(forward, up);
		if (Mathf.Abs(dot) > 0.999f) {
			// 上すぎ問題の応急処置
			up = new Vector3(0, 0, 1);
		}

		// 左手系の LookTo 行列を作る
		Matrix4x4 view = Matrix4x4.CreateLookToLH(position, forward, up);

		// カメラのワールド行列 = view の逆行列
		Matrix4x4 world = Matrix4x4.Inverse(view);

		// 回転部分をクォータニオンへ
		Quaternion rot = Quaternion.CreateFromRotationMatrix(world);
		return Quaternion.Normalized(rot);
	}


	static public Quaternion CreateFromRotationMatrix(Matrix4x4 m) {
		float trace = m.m00 + m.m11 + m.m22;

		Quaternion q = new Quaternion();

		if (trace > 0.0f) {
			float s = Mathf.Sqrt(trace + 1.0f) * 2.0f; // 4 * w
			q.w = 0.25f * s;
			q.x = (m.m21 - m.m12) / s;
			q.y = (m.m02 - m.m20) / s;
			q.z = (m.m10 - m.m01) / s;
		} else if (m.m00 > m.m11 && m.m00 > m.m22) {
			float s = Mathf.Sqrt(1.0f + m.m00 - m.m11 - m.m22) * 2.0f; // 4 * x
			q.w = (m.m21 - m.m12) / s;
			q.x = 0.25f * s;
			q.y = (m.m01 + m.m10) / s;
			q.z = (m.m02 + m.m20) / s;
		} else if (m.m11 > m.m22) {
			float s = Mathf.Sqrt(1.0f + m.m11 - m.m00 - m.m22) * 2.0f; // 4 * y
			q.w = (m.m02 - m.m20) / s;
			q.x = (m.m01 + m.m10) / s;
			q.y = 0.25f * s;
			q.z = (m.m12 + m.m21) / s;
		} else {
			float s = Mathf.Sqrt(1.0f + m.m22 - m.m00 - m.m11) * 2.0f; // 4 * z
			q.w = (m.m10 - m.m01) / s;
			q.x = (m.m02 + m.m20) / s;
			q.y = (m.m12 + m.m21) / s;
			q.z = 0.25f * s;
		}

		// 任意のQuaternion型にNormalizeが無い場合は自前で処理
		float len = Mathf.Sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
		if (len > 1e-8f) {
			float inv = 1.0f / len;
			q.x *= inv;
			q.y *= inv;
			q.z *= inv;
			q.w *= inv;
		}

		return q;
	}


	/// -----------------------------------------
	/// public methods
	/// -----------------------------------------

	public float Length() {
		return Mathf.Sqrt(x * x + y * y + z * z + w * w);
	}

	public Quaternion Normalized() {
		float length = Length();
		if (length == 0.0f) {
			return identity;
		}

		return new Quaternion(x / length, y / length, z / length, w / length);
	}

	public Quaternion Conjugate() {
		return new Quaternion(-x, -y, -z, w);
	}

	public Quaternion Inverse() {
		Quaternion conjugate = Conjugate(); // 共役を計算
		float norm = Length();                // ノルムを計算
		if (norm == 0.0f) {
			return identity; // ノルムが0の場合は単位クォータニオンを返す
		}

		float normSquared = norm * norm;    // ノルムの二乗
		return conjugate / normSquared;
	}

	public Vector3 ToEuler() {
		Vector3 euler = new Vector3();

		// Pitch (X軸)
		float sinp = 2.0f * (w * x + y * z);
		float cosp = 1.0f - 2.0f * (x * x + y * y);
		euler.x = Mathf.Atan2(sinp, cosp);

		// Yaw (Y軸)
		float siny = 2.0f * (w * y - z * x);
		if (Mathf.Abs(siny) >= 1.0f) {
			euler.y = Mathf.CopySign(Mathf.PI / 2.0f, siny); // クランプ
		} else {
			euler.y = Mathf.Asin(siny);
		}

		// Roll (Z軸)
		float sinr = 2.0f * (w * z + x * y);
		float cosr = 1.0f - 2.0f * (y * y + z * z);
		euler.z = Mathf.Atan2(sinr, cosr);

		return euler;
	}

	static public Quaternion FromEuler(Vector3 euler) {
		float pitch = euler.x * 0.5f; // X回転
		float yaw = euler.y * 0.5f; // Y回転
		float roll = euler.z * 0.5f; // Z回転

		float sinPitch = Mathf.Sin(pitch);
		float cosPitch = Mathf.Cos(pitch);
		float sinYaw = Mathf.Sin(yaw);
		float cosYaw = Mathf.Cos(yaw);
		float sinRoll = Mathf.Sin(roll);
		float cosRoll = Mathf.Cos(roll);

		Quaternion q;
		q.x = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
		q.y = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;
		q.z = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
		q.w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;

		return q;
	}


	/// -------------------------------------------
	/// public operators
	/// - -----------------------------------------

	static public Quaternion operator *(Quaternion q1, Quaternion q2) {
		return new Quaternion(
			q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
			q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
			q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x,
			q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
		);
	}

	static public Quaternion operator /(Quaternion q, float scalar) {
		return new Quaternion(
			q.w / scalar,
			q.x / scalar,
			q.y / scalar,
			q.z / scalar
		);
	}

}