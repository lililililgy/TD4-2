#pragma once

/// std
#include <array>

/// engine
#include "Engine/Core/Utility/Math/Vector3.h"

namespace ONEngine {

/// ///////////////////////////////////////////////////
/// CollisionCheck 
/// ///////////////////////////////////////////////////
namespace CollisionCheck {

/// @brief 線分と球の当たり判定を取る
/// @param lineStart 線分の始点
/// @param lineEnd 線分の終点
/// @param sphereCenter 球の中心
/// @param sphereRadius 球の半径
/// @return true: 衝突している false: 衝突していない
bool LineVsSphere(
	const Vector3& lineStart, const Vector3& lineEnd,
	const Vector3& sphereCenter, float sphereRadius
);

/// @brief 線と三角形の当たり判定を取る
/// @param lineStart 線の始点
/// @param lineEnd 線の終点
/// @param triangleVertices 三角形の3頂点 
/// @return true: 衝突している false: 衝突していない
bool LineVsTriangle(
	const Vector3& lineStart, const Vector3& lineEnd,
	const std::array<Vector3, 3>& triangleVertices
);

/// @brief Rayと球の当たり判定を取る
/// @param rayDirection Rayの方向ベクトル
/// @param sphereCenter 球の中心
/// @param sphereRadius 球の半径
/// @return true: 衝突している false: 衝突していない
bool RayVsSphere(
	const Vector3& rayStartPosition, const Vector3& rayDirection,
	const Vector3& sphereCenter, float sphereRadius
);


/// @brief Rayと箱の当たり判定を取る
/// @param rayStartPosition rayの始点
/// @param rayDirection rayのベクトル
/// @param cubePosition cubeの中心点
/// @param cubeSize cubeのサイズ
/// @return true: 衝突している false: 衝突していない
bool RayVsCube(
	const Vector3& rayStartPosition, const Vector3& rayDirection,
	const Vector3& cubePosition, const Vector3& cubeSize
);

/// @brief AABB同士の当たり判定を取る
/// @param cube1Position 一つ目のCubeの中心点
/// @param cube1Size 一つ目のCubeのサイズ
/// @param cube2Position 二つ目のCubeの中心点
/// @param cube2Size 二つ目のCubeのサイズ
/// @return true: 衝突している false: 衝突していない
bool CubeVsCube(
	const Vector3& cube1Position, const Vector3& cube1Size,
	const Vector3& cube2Position, const Vector3& cube2Size,
	Vector3* outNormal = nullptr,
	float* outPenetration = nullptr
);

/// @brief AABBと球の当たり判定を取る
/// @param cubePosition AABBの中心点
/// @param cubeSize AABBのサイズ
/// @param sphereCenter Sphereの中心点
/// @param sphereRadius Sphereの半径
/// @param outClosestPoint AABB上の最も近い点の出力先ポインタ
/// @param outDistanceSq 球の中心とAABB上の最も近い点の距離の出力先ポインタ
/// @return true: 衝突している false: 衝突していない
bool CubeVsSphere(
	const Vector3& cubePosition, const Vector3& cubeSize,
	const Vector3& sphereCenter, float sphereRadius,
	Vector3* outClosestPoint = nullptr,
	float* outDistance = nullptr
);

/// @brief AABBとカプセルの当たり判定を取る
/// @param cubePosition AABBの中心点
/// @param cubeSize AABBのサイズ
/// @param capsuleStart Capsuleの始点
/// @param capsuleEnd Capsuleの終点
/// @param capsuleRadius Capsuleの半径
/// @return true: 衝突している false: 衝突していない
bool CubeVsCapsule(
	const Vector3& cubePosition, const Vector3& cubeSize,
	const Vector3& capsuleStart, const Vector3& capsuleEnd, float capsuleRadius
);

/// @brief Sphere同士の当たり判定を取る
/// @param sphere1Center Sphere1の中心
/// @param sphere1Radius Sphere1の半径
/// @param sphere2Center Sphere2の中心
/// @param sphere2Radius Sphere2の半径
/// @return true: 衝突している false: 衝突していない
bool SphereVsSphere(
	const Vector3& sphere1Center, float sphere1Radius,
	const Vector3& sphere2Center, float sphere2Radius
);

/// @brief SphereとCapsuleの当たり判定を取る
/// @param sphereCenter Sphereの中心
/// @param sphereRadius Sphereの半径
/// @param capsuleStart Capsuleの始点
/// @param capsuleEnd Capsuleの終点
/// @param capsuleRadius Capsuleの半径
/// @return true: 衝突している false: 衝突していない
bool SphereVsCapsule(
	const Vector3& sphereCenter, float sphereRadius,
	const Vector3& capsuleStart, const Vector3& capsuleEnd, float capsuleRadius
);


}



namespace CollisionMath {

/// @brief pointに最も近いAABB上の点を求める
/// @param point 
/// @param aabbMin AABBの最小点
/// @param aabbMax AABBの最大点
/// @return pointに最も近いAABB上の点
Vector3 ClosestPointOnAABB(const Vector3& point, const Vector3& aabbMin, const Vector3& aabbMax);

/// @brief LineとAABBの最近接点を求める
/// @param lineStart Lineの始点
/// @param lineEnd Lineの終点
/// @param aabbMin AABBの最小点
/// @param aabbMax AABBの最大点
/// @param outSegmentPoint Segment上の最近接点
/// @param outAABBPoint AABB上の最近接点
void ClosestPointsSegmentAABB(
	const Vector3& lineStart, const Vector3& lineEnd,
	const Vector3& aabbMin, const Vector3& aabbMax,
	Vector3& outSegmentPoint, Vector3& outAABBPoint
);

}

} /// namespace ONEngine