#pragma once

/// engine
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

namespace ONEngine::Math {

/// @brief 線形補間
template <typename T>
inline T Lerp(const T& a, const T& b, float t) {
    return a + (b - a) * t;
}

/// @brief ステップ（補間なし）
template <typename T>
inline T Step(const T& a, const T& /*b*/, float /*t*/) {
    return a;
}

/// @brief Vector2 の線形補間
inline Vector2 Lerp(const Vector2& a, const Vector2& b, float t) {
    return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t) };
}

/// @brief Vector3 の線形補間
inline Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
    return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t) };
}

/// @brief Vector4 の線形補間
inline Vector4 Lerp(const Vector4& a, const Vector4& b, float t) {
    return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t), Lerp(a.w, b.w, t) };
}

} /// namespace ONEngine::Math
