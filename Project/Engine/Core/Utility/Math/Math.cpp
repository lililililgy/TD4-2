#include "Math.h"

/// std
#include <filesystem>
#include <fstream>

/// engine
#include "Engine/Core/Utility/Utility.h"

/// namespaceの短縮
namespace fs = std::filesystem;

using namespace ONEngine;

float Math::Cot(float t) {
	/// 逆タンジェント
	return 1.0f / std::tan(t);
}


uint32_t Math::DivideAndRoundUp(uint32_t numerator, uint32_t denominator) {
	if (denominator == 0) {
		return 0; // ゼロ除算防止
	}

	return (numerator + denominator - 1) / denominator;
}


Vector3 Math::CatmullRomPosition(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t) {
	float t2 = t * t;
	float t3 = t2 * t;

	return 0.5f * (
		(2.0f * p1) +
		(-p0 + p2) * t +
		(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
		(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

bool ONEngine::Math::Inside(const Vector2& point, const Vector2& min, const Vector2& max) {
	/// 点が矩形の内側にあるか判定
	return (point.x >= min.x && point.x <= max.x
		&& point.y >= min.y && point.y <= max.y);
}

Vector4 ONEngine::Math::ConvertToVector4(const Vector3& v3, float w) {
	return Vector4(
		v3.x,
		v3.y,
		v3.z,
		w
	);
}
