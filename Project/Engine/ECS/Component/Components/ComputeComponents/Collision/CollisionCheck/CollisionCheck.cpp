#define NOMINMAX
#include "CollisionCheck.h"

using namespace ONEngine;

bool CollisionCheck::LineVsSphere(const Vector3& lineStart, const Vector3& lineEnd, const Vector3& /*sphereCenter*/, float /*sphereRadius*/) {
	Vector3 lineDiff = lineEnd - lineStart;

	return false;
}

bool CollisionCheck::LineVsTriangle(const Vector3& lineStart, const Vector3& lineEnd, const std::array<Vector3, 3>& triangleVertices) {

	Vector3&& lineDiff = lineEnd - lineStart;

	/// 三角形の頂点同士のベクトルを計算
	Vector3&& v01 = triangleVertices[1] - triangleVertices[0];
	Vector3&& v12 = triangleVertices[2] - triangleVertices[1];
	Vector3&& v20 = triangleVertices[0] - triangleVertices[2];

	/// 三角形の法線ベクトルと平面の距離を計算
	Vector3&& normal = Vector3::Cross(v01, v12);
	float distance = Vector3::Dot(
		(triangleVertices[0] + triangleVertices[1] + triangleVertices[2]) / 3.0f, /// 3頂点の平均が中心
		normal
	);

	/// 線分と平面の交点を計算
	float dot = Vector3::Dot(normal, lineDiff);

	/// 平面と線分が平行の場合は衝突しない
	if (dot == 0.0f) {
		return false;
	}

	float t = (distance - Vector3::Dot(normal, lineStart)) / dot;
	Vector3&& planePoint = lineStart + (lineDiff * t);

	Vector3&& cross01 = Vector3::Cross(v01, planePoint - triangleVertices[1]);
	Vector3&& cross12 = Vector3::Cross(v12, planePoint - triangleVertices[2]);
	Vector3&& cross20 = Vector3::Cross(v20, planePoint - triangleVertices[0]);

	if (Vector3::Dot(cross01, normal) >= 0.0f
		&& Vector3::Dot(cross12, normal) >= 0.0f
		&& Vector3::Dot(cross20, normal) >= 0.0f) {
		return true;
	}

	return false;
}

bool CollisionCheck::RayVsSphere(const Vector3& rayStartPosition, const Vector3& rayDirection, const Vector3& sphereCenter, float sphereRadius) {

	Vector3&& rayDir = rayDirection.Normalize();
	Vector3&& sphereToRay = sphereCenter - rayDir + rayStartPosition;

	/// 最近接点を計算
	float dot = Vector3::Dot(sphereToRay, rayDir);
	Vector3&& nearPos = rayDir * dot;

	/// 球の中心からRayの最近接点までの距離を計算
	float distance = Vector3::Length(sphereToRay - nearPos);
	return distance <= sphereRadius;
}

bool CollisionCheck::RayVsCube(const Vector3& rayStartPosition, const Vector3& rayDirection, const Vector3& cubePosition, const Vector3& cubeSize) {
	Vector3 aabbMin = cubePosition - cubeSize / 2.0f;
	Vector3 aabbMax = cubePosition + cubeSize / 2.0f;

	Vector3 min = (aabbMin - rayStartPosition) / rayDirection;
	Vector3 max = (aabbMax - rayStartPosition) / rayDirection;

	Vector3 nearPoint = {
		std::min(min.x, max.x),
		std::min(min.y, max.y),
		std::min(min.z, max.z)
	};

	Vector3 farPoint = {
		std::max(min.x, max.x),
		std::max(min.y, max.y),
		std::max(min.z, max.z)
	};

	float tmin = std::max({ nearPoint.x, nearPoint.y, nearPoint.z });
	float tmax = std::min({ farPoint.x, farPoint.y, farPoint.z });

	/// Ray用の制限
	if (tmax < 0.0f) {
		return false;
	}

	if (tmin <= tmax) {
		return true;
	}

	return false;
}

bool CollisionCheck::CubeVsCube(
	const Vector3& cube1Position, const Vector3& cube1Size, const Vector3& cube2Position, const Vector3& cube2Size,
	Vector3* outNormal, float* outPenetration) {

	Vector3&& aMin = (cube1Position - cube1Size / 2.0f);
	Vector3&& aMax = (cube1Position + cube1Size / 2.0f);
	Vector3&& bMin = (cube2Position - cube2Size / 2.0f);
	Vector3&& bMax = (cube2Position + cube2Size / 2.0f);

	if (!(aMin.x <= bMax.x && aMax.x >= bMin.x)) { return false; }
	if (!(aMin.y <= bMax.y && aMax.y >= bMin.y)) { return false; }
	if (!(aMin.z <= bMax.z && aMax.z >= bMin.z)) { return false; }


	/// 各軸ごとのめりこみ量を計算
	float overlapX = std::min(aMax.x - bMin.x, bMax.x - aMin.x);
	float overlapY = std::min(aMax.y - bMin.y, bMax.y - aMin.y);
	float overlapZ = std::min(aMax.z - bMin.z, bMax.z - aMin.z);
	float penetration = overlapX;
	Vector3 normal;

	Vector3 delta = cube2Position - cube1Position;

	/// どの軸が最も浅いめりこみかを調べる かつ 法線を設定
	if (overlapY < penetration) {
		penetration = overlapY;
		normal = (delta.y > 0.0f) ? Vector3::Up : Vector3::Down;
	} else {
		normal = (delta.x > 0.0f) ? Vector3::Right : Vector3::Left;
	}

	if (overlapZ < penetration) {
		penetration = overlapZ;
		normal = (delta.z > 0.0f) ? Vector3::Forward : Vector3::Back;
	}

	if (outNormal) {
		*outNormal = normal;
	}
	if (outPenetration) {
		*outPenetration = penetration;
	}

	return true;
}

bool CollisionCheck::CubeVsSphere(
	const Vector3& cubePosition, const Vector3& cubeSize, const Vector3& sphereCenter, float sphereRadius,
	Vector3* outClosestPoint, float* outDistance) {

	Vector3&& cubeMin = cubePosition - cubeSize / 2.0f;
	Vector3&& cubeMax = cubePosition + cubeSize / 2.0f;

	Vector3&& closestPoint = {
		std::clamp(sphereCenter.x, cubeMin.x, cubeMax.x),
		std::clamp(sphereCenter.y, cubeMin.y, cubeMax.y),
		std::clamp(sphereCenter.z, cubeMin.z, cubeMax.z)
	};

	float distance = Vector3::Length(sphereCenter - closestPoint);

	/// 出力パラメータの設定
	if (outClosestPoint) {
		*outClosestPoint = closestPoint;
	}
	if (outDistance) {
		*outDistance = distance;
	}


	if (distance <= sphereRadius) {
		return true;
	}

	return false;
}

bool CollisionCheck::CubeVsCapsule(const Vector3& cubePosition, const Vector3& cubeSize, const Vector3& capsuleStart, const Vector3& capsuleEnd, float capsuleRadius) {
	Vector3 capsulePoint, boxPoint;
	CollisionMath::ClosestPointsSegmentAABB(
		capsuleStart, capsuleEnd,
		cubePosition - cubeSize / 2.0f, cubePosition + cubeSize / 2.0f,
		capsulePoint, boxPoint
	);

	float distance = Vector3::Length(capsulePoint - boxPoint);

	return distance < capsuleRadius;
}

bool CollisionCheck::SphereVsSphere(const Vector3& sphere1Center, float sphere1Radius, const Vector3& sphere2Center, float sphere2Radius) {
	float distance = Vector3::Length(sphere1Center - sphere2Center);
	if (distance <= sphere1Radius + sphere2Radius) {
		return true;
	}

	return false;
}

bool CollisionCheck::SphereVsCapsule(const Vector3& sphereCenter, float sphereRadius, const Vector3& capsuleStart, const Vector3& capsuleEnd, float capsuleRadius) {
	/// 最近接点を求める
	Vector3 capsuleDirection = capsuleEnd - capsuleStart;
	float capsuleLength = Vector3::Length(capsuleDirection);

	if (capsuleLength == 0.0f) {
		/// カプセルの長さが0の場合、カプセルは点として扱う
		return SphereVsSphere(
			sphereCenter, sphereRadius,
			capsuleStart, capsuleRadius
		);
	}

	Vector3 dir = capsuleDirection * (1.0f / capsuleLength);
	float t = Vector3::Dot(sphereCenter - capsuleStart, dir);
	if (t < 0.0f) {
		/// 球の中心がカプセルの始点より前にある場合
		t = 0.0f;
	} else if (t > capsuleLength) {
		/// 球の中心がカプセルの終点より後ろにある場合
		t = capsuleLength;
	}

	Vector3 closestPoint = capsuleStart + dir * t;
	float distance = Vector3::Length(sphereCenter - closestPoint);
	return distance < (sphereRadius + capsuleRadius);
}

Vector3 CollisionMath::ClosestPointOnAABB(const Vector3& point, const Vector3& aabbMin, const Vector3& aabbMax) {
	/// 各軸ごとにクランプして最も近い点を求める
	return {
		std::max(aabbMin.x, std::min(point.x, aabbMax.x)),
		std::max(aabbMin.y, std::min(point.y, aabbMax.y)),
		std::max(aabbMin.z, std::min(point.z, aabbMax.z))
	};
}

void CollisionMath::ClosestPointsSegmentAABB(const Vector3& lineStart, const Vector3& lineEnd, const Vector3& aabbMin, const Vector3& aabbMax, Vector3& outSegmentPoint, Vector3& outAABBPoint) {
	Vector3 segmentDirection = lineEnd - lineStart;
	float segmentLength = Vector3::Length(segmentDirection);

	/// 線分の長さが0の場合、始点を返す
	if (segmentLength == 0.0f) {
		outSegmentPoint = lineStart;
		outAABBPoint = ClosestPointOnAABB(lineStart, aabbMin, aabbMax);
		return;
	}

	/// Segmentの方向を正規化
	Vector3 dir = segmentDirection * (1.0f / segmentLength);

	float t = 0.0f;
	Vector3 closest = lineStart;

	/// 各軸ごとにAABBの範囲外にある場合、tを更新
	for (int i = 0; i < 3; ++i) {

		/// 各軸の成分を取得
		float segStart = (&lineStart.x)[i];
		float segEnd = (&lineEnd.x)[i];
		float aabbMinVal = (&aabbMin.x)[i];
		float aabbMaxVal = (&aabbMax.x)[i];

		/// 線分の成分の差を計算
		float segDelta = segEnd - segStart;

		if (segStart < aabbMinVal && segDelta > 0.0f) {
			t = std::max(t, (aabbMinVal - segStart) / segDelta);
		} else if (segStart > aabbMaxVal && segDelta < 0.0f) {
			t = std::max(t, (aabbMaxVal - segStart) / segDelta);
		}
	}


	/// 0.0 - 1.0の範囲にクランプ
	t = std::clamp(t, 0.0f, 1.0f);

	/// 線分上の最近接点を計算
	outSegmentPoint = lineStart + segmentDirection * t;

	/// AABB上の最近接点を計算
	outAABBPoint = ClosestPointOnAABB(outSegmentPoint, aabbMin, aabbMax);

}
