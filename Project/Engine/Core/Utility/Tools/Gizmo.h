#pragma once

#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Color.h"
#include "Engine/Core/Utility/Math/Quaternion.h"

/// ///////////////////////////////////////////////////
/// gizmoのクラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

class Gizmo final {
	friend class GizmoRenderingPipeline;
	friend class RenderingFramework;
public:
	/// ====================================
	/// private : sub class 
	/// ====================================

	struct SphereData {
		Vector3 position; ///< 球の位置
		float radius;     ///< 球の半径
		Vector4 color;    ///< 球の色
	};

	struct CubeData {
		Vector3 position;    ///< 箱の位置
		Vector3 size;        ///< 箱のサイズ
		Quaternion rotate;   ///< 箱の回転
		Vector4 color;       ///< 箱の色
	};

	struct LineData {
		Vector3 startPosition; ///< 線の開始地点
		Vector3 endPosition;   ///< 線の終了地点
		Vector4 color;         ///< 線の色
		float thickness;       ///< 線の太さ
	};

private:
	/// ====================================
	/// private : methods
	/// ====================================

	Gizmo() = default;
	~Gizmo() = default;

	/// @brief 初期化関数、rendering pipelineクラスで呼び出す
	/// @param maxDrawInstanceCount 描画するインスタンスの最大数を返す
	static void Initialize(const size_t maxDrawInstanceCount);

	/// ----- 各形状の配列を返す ----- ///
	static const std::vector<SphereData>& GetSphereData();
	static const std::vector<SphereData>& GetWireSphereData();
	static const std::vector<CubeData>& GetCubeData();
	static const std::vector<CubeData>& GetWireCubeData();
	static const std::vector<LineData>& GetLineData();

	/// @brief データのリセット
	static void Reset();

public:
	/// ====================================
	/// public : static methods
	/// ====================================

	/// @brief 球の描画
	/// @param position ワールド座標
	/// @param radius 球の半径
	/// @param color 球の色
	static void DrawSphere(const Vector3& position, float radius, const Vector4& color = Color::kWhite);

	/// @brief ワイヤーフレームの球を描画
	/// @param position ワールド座標
	/// @param radius 球の半径
	/// @param color 球の色
	static void DrawWireSphere(const Vector3& position, float radius, const Vector4& color = Color::kWhite);

	/// @brief 箱の描画
	/// @param position ワールド座標
	/// @param size 箱のサイズ
	/// @param rotate 箱の回転
	/// @param color 箱の色
	static void DrawCube(const Vector3& position, const Vector3& size, const Quaternion& rotate = Quaternion::kIdentity, const Vector4& color = Color::kWhite);

	/// @brief ワイヤーフレームの箱を描画
	/// @param position ワールド座標
	/// @param size 箱의サイズ
	/// @param rotate 箱の回転
	/// @param color 箱の色 
	static void DrawWireCube(const Vector3& position, const Vector3& size, const Quaternion& rotate = Quaternion::kIdentity, const Vector4& color = Color::kWhite);

	/// @brief 線の描画
	/// @param startPosition 線の始点
	/// @param endPosition 線の終点
	/// @param color 線の色
	/// @param thickness 線の太さ
	static void DrawLine(const Vector3& startPosition, const Vector3& endPosition, const Vector4& color = Color::kWhite, float thickness = 1.0f);

	/// @brief Rayの描画
	/// @param position 線の始点 
	/// @param direction 線の方向
	/// @param color 線の色
	/// @param thickness 線の太さ
	static void DrawRay(const Vector3& position, const Vector3& direction, const Vector4& color = Color::kWhite, float thickness = 1.0f);

};


} /// ONEngine
