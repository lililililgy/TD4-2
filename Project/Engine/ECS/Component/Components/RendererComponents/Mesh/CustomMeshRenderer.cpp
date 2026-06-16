#include "CustomMeshRenderer.h"

/// engine
#include "Engine/Asset/Collection/AssetCollection.h"

using namespace ONEngine;

CustomMeshRenderer::CustomMeshRenderer() {
	gpuMaterial_.baseColor = Vector4::White;
	gpuMaterial_.postEffectFlags = PostEffectFlags_Lighting;
}

CustomMeshRenderer::~CustomMeshRenderer() {}

void CustomMeshRenderer::MeshRecreate(DxDevice* pDxDevice) {
	mesh_.CreateBuffer(pDxDevice);
	mesh_.VertexBufferMapping();
	mesh_.IndexBufferMapping();
}

void CustomMeshRenderer::VertexMemcpy() {
	mesh_.MemcpyVertexData();
}

void CustomMeshRenderer::SetVertices(const std::vector<CustomMeshRenderer::Vertex>& vertices) {
	mesh_.SetVertices(vertices);
}

void CustomMeshRenderer::SetIndices(const std::vector<uint32_t>& indices) {
	mesh_.SetIndices(indices);
}

void CustomMeshRenderer::SetTexturePath(const std::string& path) {
	texturePath_ = path;
}

void CustomMeshRenderer::SetColor(const Vector4& color) {
	gpuMaterial_.baseColor = color;
}

void CustomMeshRenderer::SetIsVisible(bool isVisible) {
	isVisible_ = isVisible;
}

void CustomMeshRenderer::SetIsBufferRecreate(bool isBufferRecreate) {
	isBufferRecreate_ = isBufferRecreate;
}

const std::string& CustomMeshRenderer::GetTexturePath() const {
	return texturePath_;
}

const Vector4& CustomMeshRenderer::GetColor() const {
	return gpuMaterial_.baseColor;
}

const CustomMeshRenderer::CustomMesh* CustomMeshRenderer::GetMesh() const {
	return &mesh_;
}

bool CustomMeshRenderer::GetIsVisible() const {
	return isVisible_;
}

bool CustomMeshRenderer::GetIsBufferRecreate() const {
	return isBufferRecreate_;
}

const GPUMaterial& CustomMeshRenderer::GetGpuMaterial() {
	return gpuMaterial_;
}
