#pragma once


/// std
#include <vector>

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Asset/Assets/Mesh/Model.h"
#include "Engine/Graphics/Buffer/Data/GPUMaterial.h"

/// ///////////////////////////////////////////////////
/// CustomMeshRenderer
/// ///////////////////////////////////////////////////
namespace ONEngine {

class CustomMeshRenderer final : public IRenderComponent {
public:

	struct Vertex {
		Vector4 position;
		Vector2 uv;
		Vector3 normal;
	};

	using CustomMesh = Mesh<Vertex>;

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	CustomMeshRenderer();
	~CustomMeshRenderer();

	/// @brief meshの再作成
	/// @param pDxDevice DxDeviceへのポインタ
	void MeshRecreate(class DxDevice* pDxDevice);

	/// @brief 頂点をGPUに転送する
	void VertexMemcpy();

private:
	/// ===================================================
	/// private : objects
	/// ====================================================

	CustomMesh mesh_;
	std::string texturePath_ = "Packages/Textures/uvChecker.png";
	bool isVisible_ = true; ///< 描画するかどうか
	bool isBufferRecreate_ = false; ///< バッファを再作成するかどうか

	GPUMaterial gpuMaterial_;

public:
	/// ====================================================
	/// public : accessor
	/// ====================================================

	/// @brief verticesのセッタ
	/// @param vertices meshの頂点データ
	void SetVertices(const std::vector<Vertex>& vertices);

	/// @brief indicesのセッタ
	/// @param indices 頂点インデックスデータ
	void SetIndices(const std::vector<uint32_t>& indices);

	/// @brief textureのpathのセッタ
	/// @param path Textureのpath
	void SetTexturePath(const std::string& path);

	/// @brief 色の設定
	/// @param color RGBAの色を設定する
	void SetColor(const Vector4& color);

	/// @brief 描画するかどうかのセッタ
	/// @param isVisible 描画フラグ
	void SetIsVisible(bool isVisible);

	/// @brief bufferを再作成するかどうかのセッタ
	/// @param isBufferRecreate bufferを再作成するフラグ
	void SetIsBufferRecreate(bool isBufferRecreate);



	/// @brief textureのpathのゲッタ
	/// @return textureのpath
	const std::string& GetTexturePath() const;

	/// @brief 色のゲッタ
	/// @return RGBAの色
	const Vector4& GetColor() const;

	/// @brief meshのゲッタ
	/// @return Meshのポインタ
	const CustomMesh* GetMesh() const;

	/// @brief 描画するかどうかのゲッタ
	/// @return 描画フラグ
	bool GetIsVisible() const;

	/// @brief bufferを再作成するかどうかのゲッタ
	/// @return bufferを再作成するフラグ
	bool GetIsBufferRecreate() const;

	const GPUMaterial& GetGpuMaterial();

};



} /// ONEngine
