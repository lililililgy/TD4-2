#include "Quaternion.h"

using namespace ONEngine;

#include <DirectXMath.h>

/// std
#include <numbers>
#include <cmath>


using namespace DirectX;


const Quaternion Quaternion::kIdentity = Quaternion(0.0f, 0.0f, 0.0f, 1.0f); ///< 単位クォータニオン


Quaternion::Quaternion() {
	*this = kIdentity;
}

Quaternion::Quaternion(float x, float y, float z, float w) {
	x = x;
	y = y;
	z = z;
	w = w;
}

float Quaternion::Length(const Quaternion& q) {
	return std::sqrt(
		q.x * q.x +
		q.y * q.y +
		q.z * q.z +
		q.w * q.w
	);
}

Quaternion Quaternion::Normalize(const Quaternion& q) {
	float len = Length(q);
	if (len != 0.0f) {
		return q / len;
	}
	return q;
}

Vector3 Quaternion::Transform(const Vector3& v, const Quaternion& q) {
	// ベクトルをクォータニオンに変換 (w = 0)
	Quaternion qVec = { v.x, v.y, v.z, 0.0f };

	// 回転: q * v * q^(-1)
	Quaternion qConjugate = q.Conjugate();
	Quaternion result = q * qVec * qConjugate;

	// 結果をベクトルとして返す
	return { result.x, result.y, result.z };
}

Quaternion Quaternion::Lerp(const Quaternion& start, const Quaternion& end, float t) {
	return Quaternion(
		std::lerp(start.x, end.x, t),
		std::lerp(start.y, end.y, t),
		std::lerp(start.z, end.z, t),
		std::lerp(start.w, end.w, t)
	);
}

Quaternion Quaternion::MakeFromAxis(const Vector3& axis, float theta) {
	float halfAngle = theta * 0.5f;
	float sinHalfAngle = std::sin(halfAngle);

	Vector3 normalizedAxis = axis.Normalize();

	float w = std::cos(halfAngle);
	float x = normalizedAxis.x * sinHalfAngle;
	float y = normalizedAxis.y * sinHalfAngle;
	float z = normalizedAxis.z * sinHalfAngle;

	return Quaternion(x, y, z, w);
}

Matrix4x4 Quaternion::MakeRotateAxisAngle(const Vector3& axis, float theta) {
	return Matrix4x4::MakeRotate(MakeFromAxis(axis, theta));
}

Quaternion Quaternion::LookAt(const Vector3& position, const Vector3& target, const Vector3& up) {
	/// ----- 視線の方向への回転を計算する ----- ///

	XMFLOAT3 xmPosition = { position.x, position.y, position.z };
	XMFLOAT3 xmTarget = { target.x, target.y, target.z };
	XMFLOAT3 xmUp = { up.x, up.y, up.z };

	XMVECTOR posVec = XMLoadFloat3(&xmPosition);
	XMVECTOR targetVec = XMLoadFloat3(&xmTarget);
	XMVECTOR upVec = XMLoadFloat3(&xmUp);

	// 視線方向（forward）を計算
	XMVECTOR lookAtVec = XMVectorSubtract(targetVec, posVec);
	lookAtVec = XMVector3Normalize(lookAtVec);

	// forward と up がほぼ平行な場合は up を差し替える（真上 or 真下対策）
	float dot = XMVectorGetX(XMVector3Dot(lookAtVec, upVec));
	if (fabsf(dot) > 0.999f) {
		// Y軸とほぼ同一方向なのでZ軸をUpに使用
		upVec = XMVectorSet(0, 0, 1, 0);
	}

	// View 行列を作成（左手系）
	XMMATRIX viewMatrix = XMMatrixLookToLH(posVec, lookAtVec, upVec);

	// View は World の逆行列なので逆行列を取る（これがカメラのワールド行列になる）
	XMMATRIX worldMatrix = XMMatrixInverse(nullptr, viewMatrix);

	// worldMatrix から回転成分を取り出す
	XMVECTOR scalePart;
	XMVECTOR rotQuat;
	XMVECTOR transPart;
	bool decomposed = XMMatrixDecompose(&scalePart, &rotQuat, &transPart, worldMatrix);

	XMVECTOR finalQuat;
	if (decomposed) {
		finalQuat = rotQuat;
	} else {
		finalQuat = XMQuaternionRotationMatrix(worldMatrix);
	}

	XMFLOAT4 result;
	XMStoreFloat4(&result, finalQuat);

	return { result.x, result.y, result.z, result.w };
}



Quaternion Quaternion::LookAt(const Vector3& position, const Vector3& target) {
	XMFLOAT3 xmPosition, xmTarget;
	xmPosition = { position.x, position.y, position.z };
	xmTarget = { target.x, target.y, target.z };

	// ベクトルに変換
	XMVECTOR eyeVec = XMLoadFloat3(&xmPosition);
	XMVECTOR targetVec = XMLoadFloat3(&xmTarget);

	// 前方ベクトルを計算
	XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(targetVec, eyeVec));

	// オイラー角の計算
	float yaw = std::atan2(XMVectorGetX(forward), XMVectorGetZ(forward));
	float pitch = std::asin(-XMVectorGetY(forward));

	// ロール角は不要なのでゼロとする
	float roll = 0.0f;

	// オイラー角をクォータニオンに変換
	XMVECTOR quaternion = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

	// XMFLOAT4に変換して返す
	XMFLOAT4 result;
	XMStoreFloat4(&result, quaternion);

	return { result.x, result.y, result.z, result.w };
}

Quaternion Quaternion::Slerp(const Quaternion& start, const Quaternion& end, float t) {
	// startと_endの内積を計算
	float dot = start.w * end.w + start.x * end.x + start.y * end.y + start.z * end.z;

	// 内積が負の場合、endを反転してショートパスを取る
	Quaternion q2Copy = end;
	if (dot < 0.0f) {
		dot = -dot;
		q2Copy.w = -q2Copy.w;
		q2Copy.x = -q2Copy.x;
		q2Copy.y = -q2Copy.y;
		q2Copy.z = -q2Copy.z;
	}

	// もし内積がほぼ1なら、線形補間を使う
	const float THRESHOLD = 0.9995f;
	if (dot > THRESHOLD) {
		Quaternion result = {
			start.x + t * (q2Copy.x - start.x),
			start.y + t * (q2Copy.y - start.y),
			start.z + t * (q2Copy.z - start.z),
			start.w + t * (q2Copy.w - start.w)
		};
		return Normalize(result);
	}

	// θを計算
	float theta_0 = std::acos(dot); // θ_0 = cos^(-1)(dot)
	float theta = theta_0 * t;      // θ = θ_0 * t

	// sinを計算
	float sin_theta = std::sin(theta);
	float sin_theta_0 = std::sin(theta_0);

	float s1 = std::cos(theta) - dot * sin_theta / sin_theta_0;
	float s2 = sin_theta / sin_theta_0;

	// 補間したクォータニオンを計算
	Quaternion result = {
		(s1 * start.x) + (s2 * q2Copy.x),
		(s1 * start.y) + (s2 * q2Copy.y),
		(s1 * start.z) + (s2 * q2Copy.z),
		(s1 * start.w) + (s2 * q2Copy.w)
	};
	return result;
}

Quaternion Quaternion::FromEuler(const Vector3& euler) {
	float pitch = euler.x * 0.5f; // X回転
	float yaw = euler.y * 0.5f; // Y回転
	float roll = euler.z * 0.5f; // Z回転

	float sinPitch = std::sin(pitch);
	float cosPitch = std::cos(pitch);
	float sinYaw = std::sin(yaw);
	float cosYaw = std::cos(yaw);
	float sinRoll = std::sin(roll);
	float cosRoll = std::cos(roll);

	Quaternion q;
	q.x = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
	q.y = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;
	q.z = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
	q.w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;

	return q;
}

Vector3 Quaternion::ToEuler(const Quaternion& q) {
	Vector3 euler;

	// Pitch (X軸)
	float sinp = 2.0f * (q.w * q.x + q.y * q.z);
	float cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	euler.x = std::atan2(sinp, cosp);

	// Yaw (Y軸)
	float siny = 2.0f * (q.w * q.y - q.z * q.x);
	if (std::abs(siny) >= 1.0f) {
		euler.y = std::copysign(std::numbers::pi_v<float> / 2.0f, siny); // クランプ
	} else {
		euler.y = std::asin(siny);
	}

	// Roll (Z軸)
	float sinr = 2.0f * (q.w * q.z + q.x * q.y);
	float cosr = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	euler.z = std::atan2(sinr, cosr);

	return euler;
}

Quaternion Quaternion::FromRotationMatrix(const Matrix4x4& m) {
	Quaternion q;

	float trace = m.m[0][0] + m.m[1][1] + m.m[2][2];
	if (trace > 0.0f) {
		float s = std::sqrt(trace + 1.0f) * 2.0f; // S=4*qw
		q.w = 0.25f * s;
		q.x = (m.m[2][1] - m.m[1][2]) / s;
		q.y = (m.m[0][2] - m.m[2][0]) / s;
		q.z = (m.m[1][0] - m.m[0][1]) / s;
	} else {
		if (m.m[0][0] > m.m[1][1] && m.m[0][0] > m.m[2][2]) {
			float s = std::sqrt(1.0f + m.m[0][0] - m.m[1][1] - m.m[2][2]) * 2.0f; // S=4*qx
			q.w = (m.m[2][1] - m.m[1][2]) / s;
			q.x = 0.25f * s;
			q.y = (m.m[0][1] + m.m[1][0]) / s;
			q.z = (m.m[0][2] + m.m[2][0]) / s;
		} else if (m.m[1][1] > m.m[2][2]) {
			float s = std::sqrt(1.0f + m.m[1][1] - m.m[0][0] - m.m[2][2]) * 2.0f; // S=4*qy
			q.w = (m.m[0][2] - m.m[2][0]) / s;
			q.x = (m.m[0][1] + m.m[1][0]) / s;
			q.y = 0.25f * s;
			q.z = (m.m[1][2] + m.m[2][1]) / s;
		} else {
			float s = std::sqrt(1.0f + m.m[2][2] - m.m[0][0] - m.m[1][1]) * 2.0f; // S=4*qz
			q.w = (m.m[1][0] - m.m[0][1]) / s;
			q.x = (m.m[0][2] + m.m[2][0]) / s;
			q.y = (m.m[1][2] + m.m[2][1]) / s;
			q.z = 0.25f * s;
		}
	}

	q = Normalize(q); // 正規化

	return q;
}


Quaternion Quaternion::Conjugate() const {
	return { -x, -y, -z, w };
}

float Quaternion::Length() const {
	return std::sqrt(w * w + x * x + y * y + z * z);
}

Quaternion Quaternion::Inverse() const {
	Quaternion conjugate = this->Conjugate(); // 共役を計算
	float norm = this->Length();                // ノルムを計算
	if (norm == 0.0f) {
		// ノルムがゼロの場合、逆クォータニオンは定義されないため、適切なエラー処理を追加
	}

	float normSquared = norm * norm;    // ノルムの二乗
	return conjugate / normSquared;
}

float Quaternion::Dot(const Quaternion& other) const {
	return x * other.x + y * other.y + z * other.z + w * other.w;
}
