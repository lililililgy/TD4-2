#include "Gizmo.h"

using namespace ONEngine;

/// std
#include <memory>

/// engine
#include "Engine/Core/Config/EngineConfig.h"

namespace {

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

			sphere2DData_.reserve(maxInstanceCount_);
			cube2DData_.reserve(maxInstanceCount_);

			wireSphere2DData_.reserve(maxInstanceCount_);
			wireCube2DData_.reserve(maxInstanceCount_);
			line2DData_.reserve(maxInstanceCount_);
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

		/// solid data 2D
		std::vector<Gizmo::SphereData> sphere2DData_;
		std::vector<Gizmo::CubeData>   cube2DData_;

		/// wire data 2D
		std::vector<Gizmo::SphereData> wireSphere2DData_;
		std::vector<Gizmo::CubeData>   wireCube2DData_;
		std::vector<Gizmo::LineData>   line2DData_;

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

const std::vector<Gizmo::SphereData>& Gizmo::GetSphere2DData() {
	return gGizmoSystem->sphere2DData_;
}

const std::vector<Gizmo::SphereData>& Gizmo::GetWireSphere2DData() {
	return gGizmoSystem->wireSphere2DData_;
}

const std::vector<Gizmo::CubeData>& Gizmo::GetCube2DData() {
	return gGizmoSystem->cube2DData_;
}

const std::vector<Gizmo::CubeData>& Gizmo::GetWireCube2DData() {
	return gGizmoSystem->wireCube2DData_;
}

const std::vector<Gizmo::LineData>& Gizmo::GetLine2DData() {
	return gGizmoSystem->line2DData_;
}

void Gizmo::Reset() {
	gGizmoSystem->sphereData_.clear();
	gGizmoSystem->cubeData_.clear();
	gGizmoSystem->wireSphereData_.clear();
	gGizmoSystem->wireCubeData_.clear();
	gGizmoSystem->lineData_.clear();

	gGizmoSystem->sphere2DData_.clear();
	gGizmoSystem->cube2DData_.clear();
	gGizmoSystem->wireSphere2DData_.clear();
	gGizmoSystem->wireCube2DData_.clear();
	gGizmoSystem->line2DData_.clear();
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

void Gizmo::DrawSphere2D(const Vector3& position, float radius, const Vector4& color) {
	gGizmoSystem->sphere2DData_.push_back({ position, radius, color });
}

void Gizmo::DrawWireSphere2D(const Vector3& position, float radius, const Vector4& color) {
	gGizmoSystem->wireSphere2DData_.push_back({ position, radius, color });
}

void Gizmo::DrawCube2D(const Vector3& position, const Vector3& size, const Quaternion& rotate, const Vector4& color) {
	gGizmoSystem->cube2DData_.push_back({ position, size, rotate, color });
}

void Gizmo::DrawWireCube2D(const Vector3& position, const Vector3& size, const Quaternion& rotate, const Vector4& color) {
	gGizmoSystem->wireCube2DData_.push_back({ position, size, rotate, color });
}

void Gizmo::DrawLine2D(const Vector3& startPosition, const Vector3& endPosition, const Vector4& color, float thickness) {
	if (!gGizmoSystem) return;
	gGizmoSystem->line2DData_.push_back({ startPosition, endPosition, color, thickness });
}

void Gizmo::DrawRay2D(const Vector3& position, const Vector3& direction, const Vector4& color, float thickness) {
	if (!gGizmoSystem) return;
	gGizmoSystem->line2DData_.push_back({ position, position + direction, color, thickness });
}

#else /// RELEASE_BUILD
/// リリース用に空の関数を定義
void Gizmo::DrawSphere(const Vector3&, float, const Vector4&) {}
void Gizmo::DrawWireSphere(const Vector3&, float, const Vector4&) {}
void Gizmo::DrawCube(const Vector3&, const Vector3&, const Quaternion&, const Vector4&) {}
void Gizmo::DrawWireCube(const Vector3&, const Vector3&, const Quaternion&, const Vector4&) {}
void Gizmo::DrawLine(const Vector3&, const Vector3&, const Vector4&, float) {}
void Gizmo::DrawRay(const Vector3&, const Vector3&, const Vector4&, float) {}

void Gizmo::DrawSphere2D(const Vector3&, float, const Vector4&) {}
void Gizmo::DrawWireSphere2D(const Vector3&, float, const Vector4&) {}
void Gizmo::DrawCube2D(const Vector3&, const Vector3&, const Quaternion&, const Vector4&) {}
void Gizmo::DrawWireCube2D(const Vector3&, const Vector3&, const Quaternion&, const Vector4&) {}
void Gizmo::DrawLine2D(const Vector3&, const Vector3&, const Vector4&, float) {}
void Gizmo::DrawRay2D(const Vector3&, const Vector3&, const Vector4&, float) {}
#endif // DEBUG_BUILD
