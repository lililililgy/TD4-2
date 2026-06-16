#pragma once

/// std
#include <array>

/// engine
#include "Vector3.h"

/// @brief 球
namespace ONEngine {

struct Sphere {
	Vector3 center;
	float radius;
};

/// @brief 箱
struct Cube {
	Vector3 center;
	Vector3 size;
};

/// @brief 線分
struct Line {
	Vector3 start;
	Vector3 end;
};

/// @brief レイ(光)
struct Ray {
	Vector3 origin;
	Vector3 direction;
};

/// @brief 平面
struct Plane {
	/// @brief 面の法線
	Vector3 normal;
	/// @brief 面から原点までの距離
	float d;
};

/// @brief 視錐台
struct Frustum {
	static constexpr size_t kPlaneCount = 6;
	std::array<Plane, kPlaneCount> planes;
};


/// @brief 円錐
struct Cone {
	Vector3 center;
	float angle;  // 円錐の角度
	float radius; // 円の半径
	float height; // 円錐の高さ
};

/// ///////////////////////////////////////////////////
/// 以下より上の構造体のJson変換関数
/// ///////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Sphere& s);
void to_json(nlohmann::json& j, const Sphere& s);

void from_json(const nlohmann::json& j, Cube& c);
void to_json(nlohmann::json& j, const Cube& c);

void from_json(const nlohmann::json& j, Line& l);
void to_json(nlohmann::json& j, const Line& l);

void from_json(const nlohmann::json& j, Ray& r);
void to_json(nlohmann::json& j, const Ray& r);

void from_json(const nlohmann::json& j, Plane& p);
void to_json(nlohmann::json& j, const Plane& p);

void from_json(const nlohmann::json& j, Frustum& f);
void to_json(nlohmann::json& j, const Frustum& f);

void from_json(const nlohmann::json& j, Cone& c);
void to_json(nlohmann::json& j, const Cone& c);

} /// ONEngine
