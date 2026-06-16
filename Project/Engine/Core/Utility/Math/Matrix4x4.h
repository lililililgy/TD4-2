#pragma once

/// external
#include <DirectXMath.h>

/// math
#include "Vector3.h"
#include "Vector4.h"

using namespace DirectX;


/// //////////////////////////////////////////////////
/// 4x4行列クラス
/// //////////////////////////////////////////////////
namespace ONEngine {

struct Matrix4x4 final {
	/// ===================================================
	/// public : constructer
	/// ===================================================

	Matrix4x4();
	Matrix4x4(const Matrix4x4& matrix);
	Matrix4x4(const float matrix[4][4]);
	Matrix4x4(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33
	);



	/// ===================================================
	/// public : static methods
	/// ===================================================

	/// @brief 拡縮行列の作成
	/// @param v 拡縮度
	/// @return 拡縮行列
	static Matrix4x4 MakeScale(const Vector3& v);

	/// @brief 回転行列の作成 X軸
	/// @param angle 回転角
	/// @return 回転行列
	static Matrix4x4 MakeRotateX(float angle);

	/// @brief 回転行列の作成 Y軸
	/// @param angle 回転角
	/// @return 回転行列
	static Matrix4x4 MakeRotateY(float angle);

	/// @brief 回転行列の作成 Z軸
	/// @param angle 回転角
	/// @return 回転行列
	static Matrix4x4 MakeRotateZ(float angle);

	/// @brief 回転行列の作成
	/// @param v 回転率
	/// @return 回転行列
	static Matrix4x4 MakeRotate(const Vector3& v);
	static Matrix4x4 MakeRotate(const struct Quaternion& q);

	/// @brief 平行移動行列の作成
	/// @param v 平行移動成分
	/// @return 平行移動行列
	static Matrix4x4 MakeTranslate(const Vector3& v);

	/// @brief アフィン行列の作成
	/// @param scale 拡縮度
	/// @param rotation 回転率
	/// @param translation 平行移動成分
	/// @return アフィン行列
	static Matrix4x4 MakeAffine(const Vector3& scale, const Vector3& rotation, const Vector3& translation);

	/// @brief 転置行列の作成
	/// @param matrix 他の行列
	/// @return 転置行列
	static Matrix4x4 MakeTranspose(const Matrix4x4& matrix);

	/// @brief 逆行列の作成
	/// @param matrix 他の行列
	/// @return 逆行列
	static Matrix4x4 MakeInverse(const Matrix4x4& matrix);

	/// @brief 左手座標系のビュー行列を作成する
	/// @param eye 視線の位置
	/// @param target 視線の注視点
	/// @param up 視線の上方向
	/// @return 計算したビュー行列
	static Matrix4x4 MakeLookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up);

	/// @brief ベクトルに行列をかける
	/// @param v ベクトル
	/// @param m 行列
	/// @return 変換後のベクトル
	static Vector3 Transform(const Vector3& v, const Matrix4x4& m);

	/// @brief ベクトルの向きだけを行列で変換する（平行移動を無視）
	static Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

	/// @brief ベクトルに行列をかける
	/// @param v ベクトル
	/// @param m 行列
	/// @return 変換後のベクトル
	static Vector4 Transform(const Vector4& v, const Matrix4x4& m);

	/// ===================================================
	/// public : methods
	/// ===================================================

	/// @brief 転置行列の取得
	/// @return 転置行列
	Matrix4x4 Transpose() const;

	/// @brief 逆行列の取得
	/// @return 逆行列
	Matrix4x4 Inverse() const;

	/// @brief 行列から拡縮成分を抽出する
	Vector3 ExtractScale() const;

	/// @brief 行列から回転成分を抽出する
	Quaternion ExtractRotation() const;

	/// @brief 行列から平行移動成分を抽出する
	Vector3 ExtractTranslation() const;


	/// ===================================================
	/// public : static objects
	/// ===================================================

	static const Matrix4x4 kIdentity; ///< 単位行列



	/// ===================================================
	/// public : objects
	/// ===================================================

	float m[4][4];



	/// ===================================================
	/// public : operators
	/// ===================================================

	/// @brief 代入演算子 operator
	/// @param other 他行列
	/// @return 代入結果
	inline Matrix4x4& operator=(const Matrix4x4& other);

	/// @brief 乗算代入演算子 operator
	/// @param other 他行列
	/// @return 乗算代入結果
	inline Matrix4x4& operator*=(const Matrix4x4& other);

};


namespace {


	/// @brief DirectXの行列型から自作の行列型へ変換
	/// @param matrix DirectXの行列型
	/// @return 自作の行列型
	inline Matrix4x4 Convert(const XMMATRIX& matrix) {
		Matrix4x4  result;
		XMFLOAT4X4 tempMatrix;
		XMStoreFloat4x4(&tempMatrix, matrix);

		for (size_t i = 0; i < 4; ++i) {
			for (size_t j = 0; j < 4; ++j) {
				result.m[i][j] = tempMatrix.m[i][j];
			}
		}
		return result;
	}

	/// @brief 自作の行列型からDirectXの行列型へ変換
	/// @param matrix 自作の行列型
	/// @return DirectXの行列型
	inline XMMATRIX Convert(const Matrix4x4& matrix) {
		return XMMATRIX(
			matrix.m[0][0], matrix.m[0][1], matrix.m[0][2], matrix.m[0][3],
			matrix.m[1][0], matrix.m[1][1], matrix.m[1][2], matrix.m[1][3],
			matrix.m[2][0], matrix.m[2][1], matrix.m[2][2], matrix.m[2][3],
			matrix.m[3][0], matrix.m[3][1], matrix.m[3][2], matrix.m[3][3]
		);
	}
}




/// ===================================================
/// operators
/// ===================================================

inline Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) {
	return Convert(Convert(m1) * Convert(m2));
}

inline Vector3 operator*(const Vector3& v, const Matrix4x4& m) {
	return Matrix4x4::Transform(v, m);
}

inline Vector4 operator*(const Vector4& v, const Matrix4x4& m) {
	return Matrix4x4::Transform(v, m);
}

/// ===================================================
/// public : operators
/// ===================================================

inline Matrix4x4& Matrix4x4::operator=(const Matrix4x4& other) {
	for (size_t r = 0; r < 4; r++) {
		for (size_t c = 0; c < 4; c++) {
			m[r][c] = other.m[r][c];
		}
	}
	return *this;
}

inline Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& other) {
	*this = *this * other;
	return *this;
}

} /// ONEngine
