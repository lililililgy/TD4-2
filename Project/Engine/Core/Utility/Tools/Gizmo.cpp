#include "Gizmo.h"

using namespace ONEngine;

/// std
#include <memory>

/// engine
#include "Engine/Core/Config/EngineConfig.h"

namespace {

	/// ///////////////////////////////////////////////////
	/// gizmoの描画データを保持するクラス
	/// ///////////////////////////////////////////////////
	class GizmoSystem {
		friend class Gizmo;
	public:
		GizmoSystem(const size_t maxDrawInstanceCount)
			: maxInstanceCount_(maxDrawInstanceCount) {

			/// 描画データのメモリを確保しておく
			sphereData_.reserve(maxInstanceCount_);
			cubeData_.reserve(maxInstanceCount_);

			wireSphereData_.reserve(maxInstanceCount_);
			wireCubeData_.reserve(maxInstanceCount_);
			lineData_.reserve(maxInstanceCount_);
		}
		~GizmoSystem() = default;

	private:
		const size_t maxInstanceCount_;

		/// solid data
		std::vector<Gizmo::SphereData> sphereData_;
		std::vector<Gizmo::CubeData>   cubeData_;

		/// wire data
		std::vector<Gizmo::SphereData> wireSphereData_;
		std::vector<Gizmo::CubeData>   wireCubeData_;
		std::vector<Gizmo::LineData>   lineData_;

	};


	/// 宣言
	std::unique_ptr<GizmoSystem> gGizmoSystem = nullptr;

} /// namespace 



void Gizmo::Initialize(const size_t maxDrawInstanceCount) {
	gGizmoSystem = std::make_unique<GizmoSystem>(maxDrawInstanceCount);
}



const std::vector<Gizmo::SphereData>& Gizmo::GetSphereData() {
	return gGizmoSystem->sphereData_;
}

const std::vector<Gizmo::SphereData>& Gizmo::GetWireSphereData() {
	return gGizmoSystem->wireSphereData_;
}

const std::vector<Gizmo::CubeData>& Gizmo::GetCubeData() {
	return gGizmoSystem->cubeData_;
}

const std::vector<Gizmo::CubeData>& Gizmo::GetWireCubeData() {
	return gGizmoSystem->wireCubeData_;
}

const std::vector<Gizmo::LineData>& Gizmo::GetLineData() {
	return gGizmoSystem->lineData_;
}

void Gizmo::Reset() {
	gGizmoSystem->sphereData_.clear();
	gGizmoSystem->cubeData_.clear();
	gGizmoSystem->wireSphereData_.clear();
	gGizmoSystem->wireCubeData_.clear();
	gGizmoSystem->lineData_.clear();
}


#ifdef DEBUG_MODE

void Gizmo::DrawSphere(const Vector3& position, float radius, const Vector4& color) {
	gGizmoSystem->sphereData_.push_back({ position, radius, color });
}

void Gizmo::DrawWireSphere(const Vector3& position, float radius, const Vector4& color) {
	gGizmoSystem->wireSphereData_.push_back({ position, radius, color });
}

void Gizmo::DrawCube(const Vector3& position, const Vector3& size, const Quaternion& rotate, const Vector4& color) {
	gGizmoSystem->cubeData_.push_back({ position, size, rotate, color });
}

void Gizmo::DrawWireCube(const Vector3& position, const Vector3& size, const Quaternion& rotate, const Vector4& color) {
	gGizmoSystem->wireCubeData_.push_back({ position, size, rotate, color });
}

void Gizmo::DrawLine(const Vector3& startPosition, const Vector3& endPosition, const Vector4& color, float thickness) {
	if (!gGizmoSystem) return;
	gGizmoSystem->lineData_.push_back({ startPosition, endPosition, color, thickness });
}

void Gizmo::DrawRay(const Vector3& position, const Vector3& direction, const Vector4& color, float thickness) {
	if (!gGizmoSystem) return;
	gGizmoSystem->lineData_.push_back({ position, position + direction, color, thickness });
}

#else /// RELEASE_BUILD
/// リリース用に空の関数を定義
void Gizmo::DrawSphere(const Vector3&, float, const Vector4&) {}
void Gizmo::DrawWireSphere(const Vector3&, float, const Vector4&) {}
void Gizmo::DrawCube(const Vector3&, const Vector3&, const Quaternion&, const Vector4&) {}
void Gizmo::DrawWireCube(const Vector3&, const Vector3&, const Quaternion&, const Vector4&) {}
void Gizmo::DrawLine(const Vector3&, const Vector3&, const Vector4&, float) {}
void Gizmo::DrawRay(const Vector3&, const Vector3&, const Vector4&, float) {}
#endif // DEBUG_BUILD
