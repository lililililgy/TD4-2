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
/// Vector3のテンプレート版
/// ///////////////////////////////////////////////////
template <typename T>
struct Vector3T final {
	/// ===================================================
	/// public : objects
	/// ===================================================

	T x, y, z;


	/// ===================================================
	/// public : constants
	/// ===================================================

	static const Vector3T<T> Zero;
	static const Vector3T<T> One;

	static const Vector3T<T> Left;
	static const Vector3T<T> Right;
	static const Vector3T<T> Up;
	static const Vector3T<T> Down;
	static const Vector3T<T> Forward;
	static const Vector3T<T> Back;

	static const Vector3T<T> Infinity;
	static const Vector3T<T> NegativeInfinity;

	static const Vector3T<T> Max;
	static const Vector3T<T> Min;


	/// ===================================================
	/// public : constructors
	/// ===================================================

	Vector3T() : x(static_cast<T>(0)), y(static_cast<T>(0)), z(static_cast<T>(0)) {}
	Vector3T(T x, T y, T z) : x(x), y(y), z(z) {}
	Vector3T(const Vector3T&) = default;
	Vector3T(Vector3T&&) = default;

	Vector3T& operator=(const Vector3T&) = default;
	Vector3T& operator=(Vector3T&&) = default;


	/// ===================================================
	/// public : static methods
	/// ===================================================

	/// @brief ベクトルの長さを取得
	/// @param v ベクトル
	/// @return ベクトルの長さ
	static T Length(const Vector3T<T>& v) {
		return static_cast<T>(std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
	}

	/// @brief ベクトルの長さの二乗を取得
	/// @param v ベクトル
	/// @return ベクトルの長さの二乗
	static T LengthSquared(const Vector3T<T>& v) {
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}

	/// @brief ベクトルの正規化
	/// @param v ベクトル
	/// @return 正規化されたベクトル
	static Vector3T<T> Normalize(const Vector3T<T>& v) {
		T length = Length(v);
		if (length == static_cast<T>(0)) {
			return Vector3T<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));
		}
		return Vector3T<T>(v.x / length, v.y / length, v.z / length);
	}

	/// @brief ベクトルの内積を取得
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @return ベクトルの内積
	static T Dot(const Vector3T<T>& a, const Vector3T<T>& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	/// @brief ベクトルの外積
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @return ベクトルの外積
	static Vector3T<T> Cross(const Vector3T<T>& a, const Vector3T<T>& b) {
		return Vector3T<T>(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}

	/// @brief ベクトルの線形補完
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @param t 補間係数
	/// @return 補間されたベクトル
	static Vector3T<T> Lerp(const Vector3T<T>& a, const Vector3T<T>& b, T t) {
		return Vector3T<T>(
			a.x + (b.x - a.x) * t,
			a.y + (b.y - a.y) * t,
			a.z + (b.z - a.z) * t
		);
	}

	/// @brief ベクトルの球面線形補完
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @param t 補間係数
	/// @return 補間されたベクトル
	static Vector3T<T> Slerp(const Vector3T<T>& a, const Vector3T<T>& b, T t) {
		T dot = Dot(Normalize(a), Normalize(b));
		dot = std::clamp(dot, static_cast<T>(-1), static_cast<T>(1));
		T theta = std::acos(dot) * t;
		Vector3T<T> relativeVec = b - a * dot;
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
	Vector3T<T> Normalize() const {
		return Normalize(*this);
	}

	/// @brief ベクトルの内積を取得
	/// @param other もう一つのベクトル
	/// @return ベクトルの内積
	T Dot(const Vector3T<T>& other) const {
		return Dot(*this, other);
	}

	/// @brief ベクトルの外積を取得
	/// @param other もう一つのベクトル
	/// @return ベクトルの外積
	Vector3T<T> Cross(const Vector3T<T>& other) const {
		return Cross(*this, other);
	}


	/// ===================================================
	/// public : operators
	/// ===================================================

	Vector3T<T>& operator+=(const Vector3T<T>& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vector3T<T>& operator-=(const Vector3T<T>& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	Vector3T<T>& operator*=(const Vector3T<T>& other) {
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}

	Vector3T<T>& operator*=(T scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	Vector3T<T>& operator/=(const Vector3T<T>& other) {
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}

	Vector3T<T>& operator/=(T scalar) {
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}


};


// クラス外で定義
template <typename T>
const Vector3T<T> Vector3T<T>::Zero = Vector3T<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));

template <typename T>
const Vector3T<T> Vector3T<T>::One = Vector3T<T>(static_cast<T>(1), static_cast<T>(1), static_cast<T>(1));

template <typename T>
const Vector3T<T> Vector3T<T>::Left = Vector3T<T>(static_cast<T>(-1), static_cast<T>(0), static_cast<T>(0));

template <typename T>
const Vector3T<T> Vector3T<T>::Right = Vector3T<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));

template <typename T>
const Vector3T<T> Vector3T<T>::Up = Vector3T<T>(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0));

template <typename T>
const Vector3T<T> Vector3T<T>::Down = Vector3T<T>(static_cast<T>(0), static_cast<T>(-1), static_cast<T>(0));

template <typename T>
const Vector3T<T> Vector3T<T>::Forward = Vector3T<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

template <typename T>
const Vector3T<T> Vector3T<T>::Back = Vector3T<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(-1));

template <typename T>
const Vector3T<T> Vector3T<T>::Infinity = Vector3T<T>(std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity());

template <typename T>
const Vector3T<T> Vector3T<T>::NegativeInfinity = Vector3T<T>(-std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity());

template <typename T>
const Vector3T<T> Vector3T<T>::Max = Vector3T<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max());

template <typename T>
const Vector3T<T> Vector3T<T>::Min = Vector3T<T>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());


/// ///////////////////////////////////////////////////
/// operator
/// ///////////////////////////////////////////////////

template <typename T>
inline Vector3T<T> operator+(const Vector3T<T>& a, const Vector3T<T>& b) {
	return Vector3T<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template <typename T>
inline Vector3T<T> operator-(const Vector3T<T>& a, const Vector3T<T>& b) {
	return Vector3T<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template <typename T>
inline Vector3T<T> operator*(const Vector3T<T>& a, const Vector3T<T>& b) {
	return Vector3T<T>(a.x * b.x, a.y * b.y, a.z * b.z);
}

template <typename T>
inline Vector3T<T> operator*(const Vector3T<T>& v, T scalar) {
	return Vector3T<T>(v.x * scalar, v.y * scalar, v.z * scalar);
}

template <typename T>
inline Vector3T<T> operator*(T scalar, const Vector3T<T>& v) {
	return Vector3T<T>(v.x * scalar, v.y * scalar, v.z * scalar);
}

template <typename T>
inline Vector3T<T> operator/(const Vector3T<T>& a, const Vector3T<T>& b) {
	return Vector3T<T>(a.x / b.x, a.y / b.y, a.z / b.z);
}

template <typename T>
inline Vector3T<T> operator/(const Vector3T<T>& v, T scalar) {
	return Vector3T<T>(v.x / scalar, v.y / scalar, v.z / scalar);
}

template <typename T>
inline Vector3T<T> operator-(const Vector3T<T>& v) {
	return Vector3T<T>(-v.x, -v.y, -v.z);
}

template <typename T>
inline Vector3T<T> operator+(const Vector3T<T>& v) {
	return v;
}

template <typename T>
bool operator==(const Vector3T<T>& a, const Vector3T<T>& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

template <typename T>
bool operator!=(const Vector3T<T>& a, const Vector3T<T>& b) {
	return !(a == b);
}



template <typename T>
void from_json(const nlohmann::json& j, Vector3T<T>& v) {
	v.x = j.at("x").get<T>();
	v.y = j.at("y").get<T>();
	v.z = j.at("z").get<T>();
}

template <typename T>
void to_json(nlohmann::json& j, const Vector3T<T>& v) {
	j = nlohmann::json{
		{ "x", v.x },
		{ "y", v.y },
		{ "z", v.z }
	};
}

}