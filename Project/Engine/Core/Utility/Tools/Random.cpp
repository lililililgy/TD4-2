#include "Random.h"
#include <algorithm>

using namespace ONEngine;

std::mt19937 Random::generator_(std::random_device{}());

int Random::Int(int min, int max) {
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(generator_);
}

int Random::Int() {
	return Int((std::numeric_limits<int>::min)(), (std::numeric_limits<int>::max)());
}

uint64_t Random::UInt64(uint64_t min, uint64_t max) {
	std::uniform_int_distribution<uint64_t> distribution(min, max);
	return distribution(generator_);
}

uint64_t Random::UInt64() {
	return UInt64((std::numeric_limits<uint64_t>::min)(), (std::numeric_limits<uint64_t>::max)());
}

float Random::Float(float min, float max) {
	if (min > max) {
		std::swap(min, max);
	}
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(generator_);
}

float Random::Float() {
	return Float((std::numeric_limits<float>::min)(), (std::numeric_limits<float>::max)());
}

Vector2 Random::Vec2(const Vector2& min, const Vector2& max) {
	return Vector2(
		Float(min.x, max.x),
		Float(min.y, max.y)
	);
}

Vector3 Random::Vec3(const Vector3& min, const Vector3& max) {
	return Vector3(
		Float(min.x, max.x),
		Float(min.y, max.y),
		Float(min.z, max.z)
	);
}

Vector4 Random::Vec4(const Vector4& min, const Vector4& max) {
	return Vector4(
		Float(min.x, max.x),
		Float(min.y, max.y),
		Float(min.z, max.z),
		Float(min.w, max.w)
	);
}

Vector3 Random::InsideUnitSphere() {
	while (true) {
		Vector3 p = Vector3(Float(-1, 1), Float(-1, 1), Float(-1, 1));
		if (p.LengthSquared() < 1.0f) return p;
	}
}
