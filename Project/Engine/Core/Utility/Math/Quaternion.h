#pragma once

/// std
#include <cmath>

/// engine
#include "Vector3.h"
#include "Matrix4x4.h"

/// //////////////////////////////////////////////////
/// 三次元での回転を表すクラス
/// //////////////////////////////////////////////////
namespace ONEngine {

struct Quaternion final {
	/// ===================================================
	/// public : methods
	/// ===================================================

	/// @brief デフォルトコンストラクタ
	Quaternion();

	/// @brief パラメータ付きコンストラクタ
	/// @param x x成分
	/// @param y y成分
	/// @param z z成分
	/// @param w w成分
	Quaternion(float x, float y, float z, float w);

	/// ===================================================
	/// public : objects
	/// ===================================================

	float x, y, z, w;

	static const Quaternion kIdentity; ///< 単位クォータニオン

	/// ===================================================
	/// public : static methods
	/// ===================================================

	/// @brief クォータニオンの長さを計算する
	/// @param q クォータニオン
	/// @return クォータニオンの長さ
	static float Length(const Quaternion& q);

	/// @brief クォータニオンを正規化する
	/// @param q クォータニオン
	/// @return 正規化されたクォータニオン
	static Quaternion Normalize(const Quaternion& q);

	/// @brief ベクトルをクォータニオンで変換する
	/// @param v ベクトル
	/// @param q クォータニオン
	/// @return 変換されたベクトル
	static Vector3 Transform(const Vector3& v, const Quaternion& q);

	/// @brief クォータニオンを線形補間する
	/// @param start 開始クォータニオン
	/// @param end 終了クォータニオン
	/// @param t 補間係数
	/// @return 補間されたクォータニオン
	static Quaternion Lerp(const Quaternion& start, const Quaternion& end, float t);

	/// @brief ある軸を基にクォータニオンを計算する
	/// @param axis 回転の軸となるベクトル
	/// @param theta 回転角度
	/// @return 軸を基に回転させたクォータニオン
	static Quaternion MakeFromAxis(const Vector3& axis, float theta);

	/// @brief 回転行列を生成する
	/// @param axis 回転軸
	/// @param theta 回転角度
	/// @return 回転行列
	static Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float theta);

	/// @brief 特定の方向を見るクォータニオンを生成する
	/// @param position 現在の位置
	/// @param target 目標位置
	/// @param up 上方向ベクトル
	/// @return 生成されたクォータニオン
	static Quaternion LookAt(const Vector3& position, const Vector3& target, const Vector3& up);

	/// @brief 特定の方向を見るクォータニオンを生成する
	/// @param position 現在の位置
	/// @param target 目標位置
	/// @return 生成されたクォータニオン
	static Quaternion LookAt(const Vector3& position, const Vector3& target);

	/// @brief クォータニオンを球面線形補間する
	/// @param start 開始クォータニオン
	/// @param end 終了クォータニオン
	/// @param t 補間係数
	/// @return 補間されたクォータニオン
	static Quaternion Slerp(const Quaternion& start, const Quaternion& end, float t);



	/// @brief Euler角からクォータニオンを作成
	/// @param euler Vector3形式のオイラー角（ラジアン）
	/// @return 生成されたクォータニオン
	static Quaternion FromEuler(const Vector3& euler);

	/// @brief QuaternionからEuler角を作成
	/// @param q Quaternion形式の回転情報
	/// @return Vector3形式のオイラー角（ラジアン）
	static Vector3 ToEuler(const Quaternion& q);


	/// @brief 行列からクォータニオンを作成
	/// @param m Matrix4x4形式の行列
	/// @return Quaternion形式の回転情報
	static Quaternion FromRotationMatrix(const Matrix4x4& m);



	/// ===================================================
	/// public : methods
	/// ===================================================

	/// @brief クォータニオンの共役を計算する
	/// @return 共役クォータニオン
	Quaternion Conjugate() const;

	/// @brief クォータニオンのノルムを計算する
	/// @return クォータニオンのノルム
	float Length() const;

	/// @brief 逆クォータニオンを計算する
	/// @return 逆クォータニオン
	Quaternion Inverse() const;

	/// @brief クォータニオンの内積を計算する
	/// @param other 他のクォータニオン
	/// @return 内積値
	float Dot(const Quaternion& other) const;

	/// ===================================================
	/// public : operator
	/// ===================================================

	inline Quaternion& operator*= (const Quaternion& other);
	inline Quaternion& operator+= (const Quaternion& other);
};



/// ===================================================
/// quaternion operator
/// ===================================================

inline Quaternion operator+ (const Quaternion& q1, const Quaternion& q2) {
	return { q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w };
}

inline Quaternion operator* (const Quaternion& q, float f) {
	return { q.x * f, q.y * f, q.z * f, q.w * f };
}

inline Quaternion operator* (float f, const Quaternion& q) {
	return q * f;
}

inline Quaternion operator* (const Quaternion& q1, const Quaternion& q2) {
	return {
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
		q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
	};
}

inline Quaternion operator/ (const Quaternion& q, float value) {
	Quaternion result;
	result.w = q.w / value;
	result.x = q.x / value;
	result.y = q.y / value;
	result.z = q.z / value;
	return result;
}


inline Quaternion& Quaternion::operator*=(const Quaternion& other) {
	*this = *this * other;
	return *this;
}

inline Quaternion& Quaternion::operator+=(const Quaternion& other) {
	*this = *this + other;
	return *this;
}

inline bool operator==(const Quaternion& q1, const Quaternion& q2) {
	return q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w;
}

inline bool operator!=(const Quaternion& q1, const Quaternion& q2) {
	return !(q1 == q2);
}

} /// ONEngine
