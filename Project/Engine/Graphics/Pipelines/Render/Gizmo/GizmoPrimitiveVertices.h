#pragma once

/// std
#include <vector>

/// engine
#include "Engine/Core/Utility/Utility.h"
#include "Engine/Core/Utility/Math/Quaternion.h"


namespace ONEngine {

/// @brief Gizmo用のプリミティブ頂点データ
namespace GizmoPrimitive {

struct VertexData {
	Vector4 position;      ///< 自身の位置 (start or end)
	Vector4 otherPosition; ///< もう一方の端点
	Vector4 color;         ///< 頂点の色
	float thickness;       ///< 線の太さ (pixel)
	Vector2 expansionDir;  ///< 拡大方向 (-1 or 1)
};
}

/// @brief Sphereの頂点データを取得する
/// @param center Sphereの中心
/// @param radius Sphereの半径
/// @param color Sphereの色
/// @param thickness 線の太さ
/// @param segment 線の分割数
/// @return 1セグメントあたり6頂点(TRIANGLELIST)の頂点データ
std::vector<GizmoPrimitive::VertexData> GetSphereVertices(const Vector3& center, float radius, const Vector4& color, float thickness = 1.0f, size_t segment = 24, bool is2D = false);

/// @brief Cubeの頂点データを取得する
/// @param center Cubeの中心
/// @param size Cube of size
/// @param rotate Cubeの回転
/// @param color Cubeの色
/// @param thickness 線の太さ
/// @return 1セグメントあたり6頂点(TRIANGLELIST)の頂点データ
std::vector<GizmoPrimitive::VertexData> GetCubeVertices(const Vector3& center, const Vector3& size, const Quaternion& rotate, const Vector4& color, float thickness = 1.0f, bool is2D = false);

/// @brief 矩形の頂点データを取得する
/// @param matWorld ワールド座標
/// @param color 色
/// @param thickness 線の太さ
/// @param rectSize 矩形の大きさ
/// @return 1セグメントあたり6頂点(TRIANGLELIST)の頂点データ
std::vector<GizmoPrimitive::VertexData> GetRectVertices(const Matrix4x4& matWorld, const Vector4& color, float thickness = 1.0f, const Vector2& rectSize = Vector2::One);

/// @brief 線分の頂点データを取得する
/// @param v0 始点
/// @param v1 終点
/// @param color 色
/// @param thickness 太さ
/// @return 1セグメントあたり6頂点(TRIANGLELIST)の頂点データ
std::vector<GizmoPrimitive::VertexData> GetLineVertices(const Vector3& v0, const Vector3& v1, const Vector4& color, float thickness = 1.0f);

} /// ONEngine