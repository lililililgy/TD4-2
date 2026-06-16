#include "GizmoPrimitiveVertices.h"
#include <cmath>
#include <numbers>

#include "Engine/Core/Utility/Math/Matrix4x4.h"

using namespace ONEngine;
using namespace GizmoPrimitive;

namespace {
	/// @brief 1つの線分から6頂点（厚みのある矩形）を生成する
	void AddThickLineSegment(std::vector<VertexData>& out, const Vector3& v0, const Vector3& v1, const Vector4& color, float thickness) {
		Vector4 p0 = Math::ConvertToVector4(v0, 1.0f);
		Vector4 p1 = Math::ConvertToVector4(v1, 1.0f);

		// expansionDir: x=側(-1 or 1), y=端点(0=start, 1=end)
		// 頂点の配置(始点から終点へ向かって、左=L, 右=R)
		// p0L(0) -- p1L(2)
		//   |    \    |
		// p0R(1) -- p1R(3)
		
		VertexData v[4];
		for (int i = 0; i < 4; ++i) {
			v[i].position = p0;      // 全ての頂点に共通の「始点」
			v[i].otherPosition = p1; // 全ての頂点に共通の「終点」
			v[i].color = color;
			v[i].thickness = thickness;
		}

		// expansionDir.x : 法線方向へのオフセット (-1 or 1)
		// expansionDir.y : 始点なら0, 終点なら1（シェーダーでどちらを使うか選択）
		v[0].expansionDir = Vector2(-1.0f, 0.0f); // 始点・左
		v[1].expansionDir = Vector2(1.0f, 0.0f);  // 始点・右
		v[2].expansionDir = Vector2(-1.0f, 1.0f); // 終点・左
		v[3].expansionDir = Vector2(1.0f, 1.0f);  // 終点・右

		// CW(時計回り)で2つの三角形を生成
		// Tri 1: (0, 2, 1) -> 始点左, 終点左, 始点右
		out.push_back(v[0]); out.push_back(v[2]); out.push_back(v[1]);
		// Tri 2: (1, 2, 3) -> 始点右, 終点左, 終点右
		out.push_back(v[1]); out.push_back(v[2]); out.push_back(v[3]);
	}
}

std::vector<VertexData> ONEngine::GetSphereVertices(const Vector3& center, float radius, const Vector4& color, float thickness, size_t segment) {
	const float deltaAngle = 2.0f * std::numbers::pi_v<float> / (float)segment;
	std::vector<VertexData> outVertices;

	auto addCircle = [&](const Vector3& axis1, const Vector3& axis2) {
		for (size_t i = 0; i < segment; ++i) {
			float angle0 = (float)i * deltaAngle;
			float angle1 = (float)(i + 1) * deltaAngle;

			Vector3 dir0 = Vector3::Normalize(axis1 * std::cos(angle0) + axis2 * std::sin(angle0));
			Vector3 dir1 = Vector3::Normalize(axis1 * std::cos(angle1) + axis2 * std::sin(angle1));

			AddThickLineSegment(outVertices, center + dir0 * radius, center + dir1 * radius, color, thickness);
		}
	};

	addCircle(Vector3::Right, Vector3::Up);
	addCircle(Vector3::Up, Vector3::Forward);
	addCircle(Vector3::Forward, Vector3::Right);

	return outVertices;
}

std::vector<VertexData> ONEngine::GetCubeVertices(const Vector3& center, const Vector3& size, const Quaternion& rotate, const Vector4& color, float thickness) {
	Vector3 halfSize = size * 0.5f;
	std::vector<VertexData> outVertices;

	// 回転行列の作成
	Matrix4x4 rotateMat = Matrix4x4::MakeRotate(rotate);

	Vector3 baseVertices[8] = {
		Vector3(-halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(halfSize.x, halfSize.y, -halfSize.z),
		Vector3(-halfSize.x, halfSize.y, -halfSize.z),
		Vector3(-halfSize.x, -halfSize.y, halfSize.z),
		Vector3(halfSize.x, -halfSize.y, halfSize.z),
		Vector3(halfSize.x, halfSize.y, halfSize.z),
		Vector3(-halfSize.x, halfSize.y, halfSize.z)
	};

	Vector3 vertices[8];
	for (int i = 0; i < 8; i++) {
		// 回転を適用してから中心座標を足す
		vertices[i] = center + Matrix4x4::Transform(baseVertices[i], rotateMat);
	}

	int32_t indices[] = {
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	};

	for (size_t i = 0; i < sizeof(indices) / sizeof(int); i += 2) {
		AddThickLineSegment(outVertices, vertices[indices[i]], vertices[indices[i + 1]], color, thickness);
	}

	return outVertices;
}

std::vector<GizmoPrimitive::VertexData> ONEngine::GetRectVertices(const Matrix4x4& matWorld, const Vector4& color, float thickness, const Vector2& rectSize) {
	std::vector<GizmoPrimitive::VertexData> outVertices;

	Vector3 vertices[4] = {
		Vector3(-rectSize.x, 0.0f, -rectSize.y),
		Vector3(+rectSize.x, 0.0f, -rectSize.y),
		Vector3(+rectSize.x, 0.0f, +rectSize.y),
		Vector3(-rectSize.x, 0.0f, +rectSize.y)
	};

	for (int i = 0; i < 4; ++i) {
		vertices[i] = Matrix4x4::Transform(vertices[i], matWorld);
	}

	for (int i = 0; i < 4; ++i) {
		AddThickLineSegment(outVertices, vertices[i], vertices[(i + 1) % 4], color, thickness);
	}

	return outVertices;
}

std::vector<VertexData> ONEngine::GetLineVertices(const Vector3& v0, const Vector3& v1, const Vector4& color, float thickness) {
	std::vector<VertexData> outVertices;
	AddThickLineSegment(outVertices, v0, v1, color, thickness);
	return outVertices;
}
