#include "Line2DRenderer.h"

/// engine
#include "Engine/Graphics/Pipelines/Collection/RenderingPipelineCollection.h"

using namespace ONEngine;

Line2DRenderer::Line2DRenderer() {
	renderingData_.vertices.reserve(32);
	lineCount_ = 0;
}
Line2DRenderer::~Line2DRenderer() {}


void Line2DRenderer::SetLine(const Vector2& start, const Vector2& end, const Vector4& color) {

	///< 今回追加するラインの頂点数が、現在の頂点数を超える場合は、頂点数を増やす
	if (renderingData_.vertices.size() < lineCount_ * 2 + 2) {
		renderingData_.vertices.resize(lineCount_ * 2 + 2);
	}

	renderingData_.vertices[lineCount_ * 2 + 0].position = { start.x, start.y, 0.0f, 1.0f };
	renderingData_.vertices[lineCount_ * 2 + 1].position = { end.x,   end.y,   0.0f, 1.0f };

	renderingData_.vertices[lineCount_ * 2 + 0].color = color;
	renderingData_.vertices[lineCount_ * 2 + 1].color = color;

	lineCount_++;
}
