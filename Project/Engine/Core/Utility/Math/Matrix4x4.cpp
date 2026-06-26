#include "Matrix4x4.h"

using namespace ONEngine;

/// std
#include <cmath>

/// engine
#include "Quaternion.h"


using namespace DirectX;


/// @brief 単位行列の定義
const Matrix4x4 Matrix4x4::kIdentity = Matrix4x4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);


/// ===================================================
/// public : constructer
/// ===================================================

Matrix4x4::Matrix4x4() {
	*this = kIdentity;
}

Matrix4x4::Matrix4x4(const Matrix4x4& matrix) {
	for (size_t r = 0; r < 4; r++) {
		for (size_t c = 0; c < 4; c++) {
			m[r][c] = matrix.m[r][c];
		}
	}
}

Matrix4x4::Matrix4x4(const float matrix[4][4]) {
	for (size_t r = 0; r < 4; r++) {
		for (size_t c = 0; c < 4; c++) {
			m[r][c] = matrix[r][c];
		}
	}
}

Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33) {
	m[0][0] = m00;
	m[0][1] = m01;
	m[0][2] = m02;
	m[0][3] = m03;

	m[1][0] = m10;
	m[1][1] = m11;
	m[1][2] = m12;
	m[1][3] = m13;

	m[2][0] = m20;
	m[2][1] = m21;
	m[2][2] = m22;
	m[2][3] = m23;

	m[3][0] = m30;
	m[3][1] = m31;
	m[3][2] = m32;
	m[3][3] = m33;
}



/// ===================================================
/// public : static methods
/// ===================================================

Matrix4x4 Matrix4x4::MakeScale(const Vector3& v) {
	return Matrix4x4(
		v.x, 0.0f, 0.0f, 0.0f,
		0.0f, v.y, 0.0f, 0.0f,
		0.0f, 0.0f, v.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

Matrix4x4 Matrix4x4::MakeRotateX(float angle) {
	return Matrix4x4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, std::cos(angle), std::sin(angle), 0.0f,
		0.0f, -std::sin(angle), std::cos(angle), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

Matrix4x4 Matrix4x4::MakeRotateY(float angle) {
	return Matrix4x4(
		std::cos(angle), 0.0f, -std::sin(angle), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		std::sin(angle), 0.0f, std::cos(angle), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

Matrix4x4 Matrix4x4::MakeRotateZ(float angle) {
	return Matrix4x4(
		std::cos(angle), std::sin(angle), 0.0f, 0.0f,
		-std::sin(angle), std::cos(angle), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

Matrix4x4 Matrix4x4::MakeRotate(const Vector3& v) {
	Matrix4x4&& x = MakeRotateX(v.x);
	Matrix4x4&& y = MakeRotateY(v.y);
	Matrix4x4&& z = MakeRotateZ(v.z);

	Matrix4x4&& result = x * y * z;

	return result;
}

Matrix4x4 Matrix4x4::MakeRotate(const Quaternion& q) {
	/// ----- Quaternionでの回転行列の作成 ----- ///

	if (Quaternion::Length(q) == 0.0f) {
		return kIdentity;
	}
	Matrix4x4 result{};

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

	result.m[0][0] = ww + xx - yy - zz;
	result.m[0][1] = 2 * (xy + wz);
	result.m[0][2] = 2 * (xz - wy);

	result.m[1][0] = 2 * (xy - wz);
	result.m[1][1] = ww - xx + yy - zz;
	result.m[1][2] = 2 * (yz + wx);

	result.m[2][0] = 2 * (xz + wy);
	result.m[2][1] = 2 * (yz - wx);
	result.m[2][2] = ww - xx - yy + zz;

	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 Matrix4x4::MakeTranslate(const Vector3& v) {
	return Matrix4x4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		v.x, v.y, v.z, 1.0f
	);
}

Matrix4x4 Matrix4x4::MakeAffine(const Vector3& scale, const Vector3& rotation, const Vector3& translation) {
	Matrix4x4&& scaleMat = MakeScale(scale);
	Matrix4x4&& rotateMat = MakeRotate(rotation);
	Matrix4x4&& translateMat = MakeTranslate(translation);

	Matrix4x4&& result = scaleMat * rotateMat * translateMat;

	return result;
}

Matrix4x4 Matrix4x4::MakeTranspose(const Matrix4x4& matrix) {
	Matrix4x4 result{};
	for (size_t r = 0; r < 4; r++) {
		for (size_t c = 0; c < 4; c++) {
			result.m[r][c] = matrix.m[c][r];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::MakeInverse(const Matrix4x4& matrix) {
	/// ----- DirectXMathを使って逆行列を計算 ----- ///

	XMVECTOR determinant;
	XMMATRIX inverseMatrix = XMMatrixInverse(&determinant, Convert(matrix));

	return Convert(inverseMatrix);
}

Matrix4x4 Matrix4x4::MakeLookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up) {
	/// ----- 左手座標系のビュー行列作成 ----- ///

	Matrix4x4 result{};

	Vector3 zAxis = Vector3::Normalize(target - eye);
	Vector3 xAxis = Vector3::Normalize(Vector3::Cross(up, zAxis));
	Vector3 yAxis = Vector3::Cross(zAxis, xAxis);
	result.m[0][0] = xAxis.x; result.m[1][0] = xAxis.y; result.m[2][0] = xAxis.z;
	result.m[0][1] = yAxis.x; result.m[1][1] = yAxis.y; result.m[2][1] = yAxis.z;
	result.m[0][2] = zAxis.x; result.m[1][2] = zAxis.y; result.m[2][2] = zAxis.z;

	result.m[3][0] = -Vector3::Dot(xAxis, eye);
	result.m[3][1] = -Vector3::Dot(yAxis, eye);
	result.m[3][2] = -Vector3::Dot(zAxis, eye);
	result.m[3][3] = 1.0f;

	return result;
}

Vector3 Matrix4x4::Transform(const Vector3& vec, const Matrix4x4& m) {
	/// ----- Vector3に行列をかける ----- ///

	float vArray[3];
	for (size_t i = 0; i < 3; i++) {
		vArray[i] = vec.x * m.m[0][i] + vec.y * m.m[1][i] + vec.z * m.m[2][i] + 1.0f * m.m[3][i];
	}

	return { vArray[0], vArray[1], vArray[2] };
}

Vector3 Matrix4x4::TransformNormal(const Vector3& vec, const Matrix4x4& m) {
	/// ----- Vector3に行列をかける (平行移動無視) ----- ///

	float vArray[3];
	for (size_t i = 0; i < 3; i++) {
		vArray[i] = vec.x * m.m[0][i] + vec.y * m.m[1][i] + vec.z * m.m[2][i];
	}

	return { vArray[0], vArray[1], vArray[2] };
}

Vector4 Matrix4x4::Transform(const Vector4& vec, const Matrix4x4& m) {
	/// ----- Vector4に行列をかける ----- ///

	float vArray[4];
	for (size_t i = 0; i < 4; i++) {
		vArray[i] = vec.x * m.m[0][i] + vec.y * m.m[1][i] + vec.z * m.m[2][i] + vec.w * m.m[3][i];
	}

	return { vArray[0], vArray[1], vArray[2], vArray[3] };
}

Matrix4x4 Matrix4x4::Transpose() const {
	return MakeTranspose(*this);
}

Matrix4x4 Matrix4x4::Inverse() const {
	return MakeInverse(*this);
}

Vector3 Matrix4x4::ExtractScale() const {
	/// 3x3に変換
	float m3x3[3][3] = {
		{ m[0][0], m[0][1], m[0][2] },
		{ m[1][0], m[1][1], m[1][2] },
		{ m[2][0], m[2][1], m[2][2] }
	};

	/// スケール成分を計算
	float scale[3];
	for (size_t i = 0; i < 3; i++) {
		scale[i] = std::sqrt(m3x3[i][0] * m3x3[i][0] + m3x3[i][1] * m3x3[i][1] + m3x3[i][2] * m3x3[i][2]);
	}

	return Vector3(scale[0], scale[1], scale[2]);
}

Quaternion Matrix4x4::ExtractRotation() const {
	Matrix4x4 matrix = *this;
	Vector3 scale = matrix.ExtractScale();
	/// スケールを除去
	matrix.m[0][0] /= scale.x;
	matrix.m[0][1] /= scale.x;
	matrix.m[0][2] /= scale.x;
	matrix.m[1][0] /= scale.y;
	matrix.m[1][1] /= scale.y;
	matrix.m[1][2] /= scale.y;
	matrix.m[2][0] /= scale.z;
	matrix.m[2][1] /= scale.z;
	matrix.m[2][2] /= scale.z;

	/// 回転成分を抽出
	return Quaternion::FromRotationMatrix(matrix);
}

Vector3 Matrix4x4::ExtractTranslation() const {
	return Vector3(
		m[3][0],
		m[3][1],
		m[3][2]
	);
}


