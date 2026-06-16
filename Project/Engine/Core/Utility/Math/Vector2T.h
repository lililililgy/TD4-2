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
/// Vector2のテンプレート版
/// ///////////////////////////////////////////////////
template <typename T>
struct Vector2T final {
	/// ===================================================
	/// public : objects
	/// ===================================================

	T x, y;


	/// ===================================================
	/// public : constants
	/// ===================================================

	static const Vector2T<T> Zero;
	static const Vector2T<T> One;

	static const Vector2T<T> Left;
	static const Vector2T<T> Right;
	static const Vector2T<T> Up;
	static const Vector2T<T> Down;

	static const Vector2T<T> Infinity;
	static const Vector2T<T> NegativeInfinity;

	static const Vector2T<T> Max;
	static const Vector2T<T> Min;

	static const Vector2T<T> HD;
	static const Vector2T<T> FHD;

	static const Vector2T<T> Half;


	/// ===================================================
	/// public : constructors
	/// ===================================================

	Vector2T() : x(static_cast<T>(0)), y(static_cast<T>(0)) {}
	Vector2T(T x, T y) : x(x), y(y) {}
	Vector2T(const Vector2T&) = default;
	Vector2T(Vector2T&&) = default;

	Vector2T& operator=(const Vector2T&) = default;
	Vector2T& operator=(Vector2T&&) = default;


	/// ===================================================
	/// public : static methods
	/// ===================================================

	/// @brief ベクトルの長さを取得
	/// @param v ベクトル
	/// @return ベクトルの長さ
	static T Length(const Vector2T<T>& v) {
		return static_cast<T>(std::sqrt(v.x * v.x + v.y * v.y));
	}

	/// @brief ベクトルの長さの二乗を取得
	/// @param v ベクトル
	/// @return ベクトルの長さの二乗
	static T LengthSquared(const Vector2T<T>& v) {
		return v.x * v.x + v.y * v.y;
	}

	/// @brief ベクトルの正規化
	/// @param v ベクトル
	/// @return 正規化されたベクトル
	static Vector2T<T> Normalize(const Vector2T<T>& v) {
		T length = Length(v);
		if (length == static_cast<T>(0)) {
			return Vector2T<T>(static_cast<T>(0), static_cast<T>(0));
		}
		return Vector2T<T>(v.x / length, v.y / length);
	}

	/// @brief ベクトルの内積を取得
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @return ベクトルの内積
	static T Dot(const Vector2T<T>& a, const Vector2T<T>& b) {
		return a.x * b.x + a.y * b.y;
	}

	/// @brief ベクトルの外積
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @return ベクトルの外積
	static T Cross(const Vector2T<T>& a, const Vector2T<T>& b) {
		return a.x * b.y - a.y * b.x;
	}

	/// @brief ベクトルの線形補完
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @param t 補間係数
	/// @return 補間されたベクトル
	static Vector2T<T> Lerp(const Vector2T<T>& a, const Vector2T<T>& b, T t) {
		return Vector2T<T>(
			a.x + (b.x - a.x) * t,
			a.y + (b.y - a.y) * t
		);
	}

	/// @brief ベクトルの球面線形補完
	/// @param a ベクトルA
	/// @param b ベクトルB
	/// @param t 補間係数
	/// @return 補間されたベクトル
	static Vector2T<T> Slerp(const Vector2T<T>& a, const Vector2T<T>& b, T t) {
		T dot = Dot(Normalize(a), Normalize(b));
		dot = std::clamp(dot, static_cast<T>(-1), static_cast<T>(1));
		T theta = std::acos(dot) * t;
		Vector2T<T> relativeVec = b - a * dot;
		relativeVec = Normalize(relativeVec);
		return (a * std::cos(theta)) + (relativeVec * std::sin(theta));
	}


	/// ===================================================
	/// public : methods
	/// ===================================================

	/// @brief ベクトルの長さを取得
	/// @return ベクトルの長さ
	float Length() const {
		return Length(*this);
	}

	/// @brief ベクトルの長さの二乗を取得
	/// @return ベクトルの長さの二乗
	float LengthSquared() const {
		return LengthSquared(*this);
	}

	/// @brief ベクトルの正規化
	/// @return 正規化されたベクトル
	Vector2T<T> Normalize() const {
		return Normalize(*this);
	}

	/// @brief ベクトルの内積を取得
	/// @param other もう一つのベクトル
	/// @return ベクトルの内積
	float Dot(const Vector2T<T>& other) const {
		return Dot(*this, other);
	}

	/// @brief ベクトルの外積を取得
	/// @param other もう一つのベクトル
	/// @return ベクトルの外積
	float Cross(const Vector2T<T>& other) const {
		return Cross(*this, other);
	}


	/// ===================================================
	/// public : operators
	/// ===================================================

	Vector2T<T>& operator+=(const Vector2T<T>& other) {
		x += other.x;
		y += other.y;
		return *this;
	}

	Vector2T<T>& operator-=(const Vector2T<T>& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	Vector2T<T>& operator*=(const Vector2T<T>& other) {
		x *= other.x;
		y *= other.y;
		return *this;
	}

	Vector2T<T>& operator*=(T scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}

	Vector2T<T>& operator/=(const Vector2T<T>& other) {
		x /= other.x;
		y /= other.y;
		return *this;
	}

	Vector2T<T>& operator/=(T scalar) {
		x /= scalar;
		y /= scalar;
		return *this;
	}


};


// クラス外で定義
template <typename T>
const Vector2T<T> Vector2T<T>::Zero = Vector2T<T>(static_cast<T>(0), static_cast<T>(0));

template <typename T>
const Vector2T<T> Vector2T<T>::One = Vector2T<T>(static_cast<T>(1), static_cast<T>(1));

template <typename T>
const Vector2T<T> Vector2T<T>::Left = Vector2T<T>(static_cast<T>(-1), static_cast<T>(0));

template <typename T>
const Vector2T<T> Vector2T<T>::Right = Vector2T<T>(static_cast<T>(1), static_cast<T>(0));

template <typename T>
const Vector2T<T> Vector2T<T>::Up = Vector2T<T>(static_cast<T>(0), static_cast<T>(1));

template <typename T>
const Vector2T<T> Vector2T<T>::Down = Vector2T<T>(static_cast<T>(0), static_cast<T>(-1));

template <typename T>
const Vector2T<T> Vector2T<T>::Infinity = Vector2T<T>(std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity());

template <typename T>
const Vector2T<T> Vector2T<T>::NegativeInfinity = Vector2T<T>(-std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity());

template <typename T>
const Vector2T<T> Vector2T<T>::Max = Vector2T<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());

template <typename T>
const Vector2T<T> Vector2T<T>::Min = Vector2T<T>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());

template <typename T>
const Vector2T<T> Vector2T<T>::HD = Vector2T<T>(static_cast<T>(1280), static_cast<T>(720));

template <typename T>
const Vector2T<T> Vector2T<T>::FHD = Vector2T<T>(static_cast<T>(1920), static_cast<T>(1080));

template <typename T>
const Vector2T<T> Vector2T<T>::Half = Vector2T<T>(static_cast<T>(0.5), static_cast<T>(0.5));


/// ///////////////////////////////////////////////////
/// operator
/// ///////////////////////////////////////////////////

template <typename T>
inline Vector2T<T> operator+(const Vector2T<T>& a, const Vector2T<T>& b) {
	return Vector2T<T>(a.x + b.x, a.y + b.y);
}

template <typename T>
inline Vector2T<T> operator-(const Vector2T<T>& a, const Vector2T<T>& b) {
	return Vector2T<T>(a.x - b.x, a.y - b.y);
}

template <typename T>
inline Vector2T<T> operator*(const Vector2T<T>& a, const Vector2T<T>& b) {
	return Vector2T<T>(a.x * b.x, a.y * b.y);
}

template <typename T>
inline Vector2T<T> operator*(const Vector2T<T>& v, T scalar) {
	return Vector2T<T>(v.x * scalar, v.y * scalar);
}

template <typename T>
inline Vector2T<T> operator*(T scalar, const Vector2T<T>& v) {
	return Vector2T<T>(v.x * scalar, v.y * scalar);
}

template <typename T>
inline Vector2T<T> operator/(const Vector2T<T>& a, const Vector2T<T>& b) {
	return Vector2T<T>(a.x / b.x, a.y / b.y);
}

template <typename T>
inline Vector2T<T> operator/(const Vector2T<T>& v, T scalar) {
	return Vector2T<T>(v.x / scalar, v.y / scalar);
}

template <typename T>
inline Vector2T<T> operator-(const Vector2T<T>& v) {
	return Vector2T<T>(-v.x, -v.y);
}

template <typename T>
inline Vector2T<T> operator+(const Vector2T<T>& v) {
	return v;
}



template <typename T>
void from_json(const nlohmann::json& j, Vector2T<T>& v) {
	v.x = j.at("x").get<T>();
	v.y = j.at("y").get<T>();
}

template <typename T>
void to_json(nlohmann::json& j, const Vector2T<T>& v) {
	j = nlohmann::json{
		{ "x", v.x },
		{ "y", v.y }
	};
}

}