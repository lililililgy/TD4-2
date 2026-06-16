#pragma once

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Graphics/Pipelines/Render/Primitive/Line3DRenderingPipeline.h"
#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Vector4.h"

/// ///////////////////////////////////////////////////
/// 3Dライン描画クラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

class Line3DRenderer final : public IComponent {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	Line3DRenderer();
	~Line3DRenderer();

	/// @brief lineの設定
	/// @param start 初期地
	/// @param end   終了地
	/// @param color ラインの色
	void SetLine(const Vector3& start, const Vector3& end, const Vector4& color);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	size_t lineCount_;
	std::vector<Line3DRenderingPipeline::VertexData> vertices_;


public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	const std::vector<Line3DRenderingPipeline::VertexData>& GetVertices() const { return vertices_; }

};


} /// ONEngine
