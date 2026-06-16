#pragma once

#ifdef max
#undef max
#endif


/// std
#include <cmath>
#include <limits>
#include <format>
#include <type_traits>
#include <algorithm>

/// externals
#include <nlohmann/json.hpp>


namespace ONEngine {

/// ///////////////////////////////////////////////////
/// Vector4のテンプレート版
/// ///////////////////////////////////////////////////
template <typename T>
struct Vector4T final {
	/// ===================================================
	/// public : objects
	/// ===================================================

	T x, y, z, w;


	/// ===================================================
	/// public : constants
	/// ===================================================

	static const Vector4T<T> Zero;
	static const Vector4T<T> One;

	static const Vector4T<T> Red;
	static const Vector4T<T> Green;
	static const Vector4T<T> Blue;
	static const Vector4T<T> White;

	static const Vector4T<T> Infinity;
	static const Vector4T<T> NegativeInfinity;

	static const Vector4T<T> Max;
	static const Vector4T<T> Min;


	/// ===================================================
	/// public : constructors
	/// ===================================================

	Vector4T() : x(static_cast<T>(0)), y(static_cast<T>(0)), z(static_cast<T>(0)), w(static_cast<T>(0)) {}
	Vector4T(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
	Vector4T(const Vector4T&) = default;
	Vector4T(Vector4T&&) = default;

	Vector4T& operator=(const Vector4T&) = default;
	Vector4T& operator=(Vector4T&&) = default;


	/// ===================================================
	/// public : static methods
	/// ===================================================

	/// @brief ベクトルの長さを取得
	/// @param v ベクトル
	/// @return ベクトルの長さ
	static T Length(const Vector4T<T>& v) {
		return static_cast<T>(std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w));
	}

	/// @brief ベクトルの長さの二乗を取得
	/// @param v ベクトル
	/// @return ベクトルの長さの二乗
	static T LengthSquared(const Vector4T<T>& v) {
		return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
	}

	/// @brief ベクトルの正規化
	/// @param v ベクトル
	/// @return 正規化されたベクトル
	static Vector4T<T> Normalize(const Vector4T<T>& v) {
		T length = Length(v);
		if (length == static_cast<T>(0)) {
			return Vector4T<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));
		}
		return Vector4T<T>(v.x / length, v.y / length, v.z / length, v.w / length);
	}

	/// @brief ベクトルの内積を取得
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @return ベクトルの内積
	static T Dot(const Vector4T<T>& a, const Vector4T<T>& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	/// @brief ベクトルの線形補完
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @param t 補間係数
	/// @return 補間されたベクトル
	static Vector4T<T> Lerp(const Vector4T<T>& a, const Vector4T<T>& b, T t) {
		return Vector4T<T>(
			a.x + (b.x - a.x) * t,
			a.y + (b.y - a.y) * t,
			a.z + (b.z - a.z) * t,
			a.w + (b.w - a.w) * t
		);
	}

	/// @brief ベクトルの球面線形補完
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @param t 補間係数
	/// @return 補間されたベクトル
	static Vector4T<T> Slerp(const Vector4T<T>& a, const Vector4T<T>& b, T t) {
		T dot = Dot(Normalize(a), Normalize(b));
		dot = std::clamp(dot, static_cast<T>(-1), static_cast<T>(1));
		T theta = std::acos(dot) * t;
		Vector4T<T> relativeVec = b - a * dot;
		relativeVec = Normalize(relativeVec);
		return (a * std::cos(theta)) + (relativeVec * std::sin(theta));
	}


	/// ===================================================
	/// public : methods
	/// ===================================================

	/// @brief ベクトルの長さを取得
	/// @return ベクトルの長さ
	T Length() const {
		return Length(*this);
	}

	/// @brief ベクトルの長さの二乗を取得
	/// @return ベクトルの長さの二乗
	T LengthSquared() const {
		return LengthSquared(*this);
	}

	/// @brief ベクトルの正規化
	/// @return 正規化されたベクトル
	Vector4T<T> Normalize() const {
		return Normalize(*this);
	}

	/// @brief ベクトルの内積を取得
	/// @param other もう一つのベクトル
	/// @return ベクトルの内積
	T Dot(const Vector4T<T>& other) const {
		return Dot(*this, other);
	}


	/// ===================================================
	/// public : operators
	/// ===================================================

	Vector4T<T>& operator+=(const Vector4T<T>& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}

	Vector4T<T>& operator-=(const Vector4T<T>& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}

	Vector4T<T>& operator*=(const Vector4T<T>& other) {
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
		return *this;
	}

	Vector4T<T>& operator*=(T scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
		return *this;
	}

	Vector4T<T>& operator/=(const Vector4T<T>& other) {
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
		return *this;
	}

	Vector4T<T>& operator/=(T scalar) {
		x /= scalar;
		y /= scalar;
		z /= scalar;
		w /= scalar;
		return *this;
	}


};


// クラス外で定義
template <typename T>
const Vector4T<T> Vector4T<T>::Zero = Vector4T<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));

template <typename T>
const Vector4T<T> Vector4T<T>::One = Vector4T<T>(static_cast<T>(1), static_cast<T>(1), static_cast<T>(1), static_cast<T>(1));

template <typename T>
const Vector4T<T> Vector4T<T>::White = Vector4T<T>(static_cast<T>(1), static_cast<T>(1), static_cast<T>(1), static_cast<T>(1));

template <typename T>
const Vector4T<T> Vector4T<T>::Red = Vector4T<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

template <typename T>
const Vector4T<T> Vector4T<T>::Green = Vector4T<T>(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(1));

template <typename T>
const Vector4T<T> Vector4T<T>::Blue = Vector4T<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(1));

template <typename T>
const Vector4T<T> Vector4T<T>::Infinity = Vector4T<T>(std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity());

template <typename T>
const Vector4T<T> Vector4T<T>::NegativeInfinity = Vector4T<T>(-std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity());

template <typename T>
const Vector4T<T> Vector4T<T>::Max = Vector4T<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max());

template <typename T>
const Vector4T<T> Vector4T<T>::Min = Vector4T<T>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());


/// ///////////////////////////////////////////////////
/// operator
/// ///////////////////////////////////////////////////

template <typename T>
inline Vector4T<T> operator+(const Vector4T<T>& a, const Vector4T<T>& b) {
	return Vector4T<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

template <typename T>
inline Vector4T<T> operator-(const Vector4T<T>& a, const Vector4T<T>& b) {
	return Vector4T<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

template <typename T>
inline Vector4T<T> operator*(const Vector4T<T>& a, const Vector4T<T>& b) {
	return Vector4T<T>(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

template <typename T>
inline Vector4T<T> operator*(const Vector4T<T>& v, T scalar) {
	return Vector4T<T>(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
}

template <typename T>
inline Vector4T<T> operator*(T scalar, const Vector4T<T>& v) {
	return Vector4T<T>(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
}

template <typename T>
inline Vector4T<T> operator/(const Vector4T<T>& a, const Vector4T<T>& b) {
	return Vector4T<T>(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

template <typename T>
inline Vector4T<T> operator/(const Vector4T<T>& v, T scalar) {
	return Vector4T<T>(v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar);
}

template <typename T>
inline Vector4T<T> operator-(const Vector4T<T>& v) {
	return Vector4T<T>(-v.x, -v.y, -v.z, -v.w);
}

template <typename T>
inline Vector4T<T> operator+(const Vector4T<T>& v) {
	return v;
}



template <typename T>
void from_json(const nlohmann::json& j, Vector4T<T>& v) {
	v.x = j.at("x").get<T>();
	v.y = j.at("y").get<T>();
	v.z = j.at("z").get<T>();
	v.w = j.at("w").get<T>();
}

template <typename T>
void to_json(nlohmann::json& j, const Vector4T<T>& v) {
	j = nlohmann::json{
		{ "x", v.x },
		{ "y", v.y },
		{ "z", v.z },
		{ "w", v.w }
	};
}

}