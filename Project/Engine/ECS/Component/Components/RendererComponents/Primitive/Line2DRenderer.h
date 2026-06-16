#pragma once

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Graphics/Pipelines/Render/Primitive/Line2DRenderingPipeline.h"
#include "Engine/Core/Utility/Math/Vector2.h"


/// ///////////////////////////////////////////////////
/// 2Dライン描画コンポーネント
/// ///////////////////////////////////////////////////
namespace ONEngine {

class Line2DRenderer final : public IRenderComponent {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================
	
	Line2DRenderer();
	~Line2DRenderer();

	/// @brief lineの設定
	/// @param start 初期値
	/// @param end   終了値
	/// @param color 色
	void SetLine(const Vector2& start, const Vector2& end, const Vector4& color);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	size_t lineCount_;
	Line2DRenderingPipeline::RenderingData renderingData_;


public:
	/// ===================================================
	/// public : accessor
	/// ===================================================
	
	const Line2DRenderingPipeline::RenderingData& GetRenderingData() const { return renderingData_; }
	Line2DRenderingPipeline::RenderingData* GetRenderingDataPtr() { return &renderingData_; }

};


} /// ONEngine
