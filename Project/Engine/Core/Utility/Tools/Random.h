#pragma once  

#define NOMINMAX 

#include <random>  
#include <limits> 

#include "../Math/Vector2.h"  
#include "../Math/Vector3.h"  
#include "../Math/Vector4.h"  

namespace ONEngine {

class Random final {
public:
	static int Int(int min, int max);
	static int Int();

	static uint64_t UInt64(uint64_t min, uint64_t max);
	static uint64_t UInt64();

	static float Float(float min, float max);
	static float Float();

	static Vector2 Vec2(const Vector2& min, const Vector2& max);
	static Vector3 Vec3(const Vector3& min, const Vector3& max);
	static Vector4 Vec4(const Vector4& min, const Vector4& max);

	static Vector3 InsideUnitSphere();

private:
	static std::mt19937 generator_;
};

} /// ONEngine
