#pragma once

/// std
#include <vector>
#include <array>
#include <span>

/// externals
#include <nlohmann/json.hpp>

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Asset/Assets/Material/Material.h"
#include "Engine/Core/Utility/Utility.h"
#include "Engine/Graphics/Buffer/StructuredBuffer.h"

#include "TerrainVertex.h"
#include "River/River.h"


namespace Editor {
class TerrainVertexEditorCompute;
}

namespace ONEngine {
class Terrain;
class EntityComponentSystem;
class DxDevice;
class DxCommand;
class DxSRVHeap;
}

namespace ONEngine::Asset {
class AssetCollection;
}


/// ComponentDebugで使用するための前方宣言
namespace ONEngine {

static const uint32_t kMaxTerrainTextureNum = 4u;

namespace ComponentDebug {
void TerrainDebug(Terrain* terrain, EntityComponentSystem* ecs, Asset::AssetCollection* assetCollection);

/// テクスチャモードの編集
bool TerrainTextureEditModeDebug(std::array<std::string, kMaxTerrainTextureNum>* texturePaths, int32_t* usedTextureIndex, Asset::AssetCollection* assetCollection);
} // namespace ComponentDebug

/// Json変換
void from_json(const nlohmann::json& j, Terrain& t);
void to_json(nlohmann::json& j, const Terrain& t);


/// ///////////////////////////////////////////////////
/// 地形のエディター情報
/// ///////////////////////////////////////////////////
struct TerrainEditorInfo {
	float brushRadius;        ///< ブラシの半径
	float brushStrength;      ///< ブラシの強さ
	int32_t editMode;         ///< 編集モード
	int32_t usedTextureIndex; ///< 使用しているテクスチャのインデックス
};


/// ///////////////////////////////////////////////////
/// 地形のコンポーネント
/// ///////////////////////////////////////////////////
class Terrain : public IComponent {
	friend class ::Editor::TerrainVertexEditorCompute;

	friend void ComponentDebug::TerrainDebug(Terrain* terrain, EntityComponentSystem* ecs, Asset::AssetCollection* assetCollection);
	friend void from_json(const nlohmann::json& j, Terrain& t);
	friend void to_json(nlohmann::json& j, const Terrain& t);
public:
	/// =========================================
	/// public : sub class
	/// =========================================

	enum SPLAT_TEX {
		GRASS,
		DIRT,
		ROCK,
		SNOW,
		SPLAT_TEX_COUNT
	};


	enum class EditMode : int32_t {
		None,    /// 操作なし
		Vertex,	 /// 勾配の操作
		Texture, /// テクスチャの操作
		Count
	};


public:
	/// =========================================
	/// public : methods
	/// =========================================

	Terrain();
	~Terrain() override;

	/// @brief VerticesとIndicesのUAVBufferを作成する
	void CreateVerticesAndIndicesBuffers(DxDevice* dxDevice, DxCommand* dxCommand, DxSRVHeap* dxSrvHeap);


	/// @brief VBVとIBVのバリアを生成する(描画用に)
	/// @param dxCommand DxCommandへのポインタ
	void CreateRenderingBarriers(DxCommand* dxCommand);

	/// @brief VBVとIBVのバリアを復元する(計算用に)
	/// @param dxCommand DxCommandへのポインタ
	void RestoreResourceBarriers(DxCommand* dxCommand);

	/// @brief 描画用にVBVを生成する
	D3D12_VERTEX_BUFFER_VIEW CreateVBV();
	/// @brief 描画用にIBVを生成する
	D3D12_INDEX_BUFFER_VIEW CreateIBV();

	/// @brief 自身のMaterialデータをGPUマテリアルデータに変換して返す
	/// @return GPUMaterialデータ
	GPUMaterial GetMaterialData();

private:
	/// =========================================
	/// private : objects
	/// =========================================

	/// ----- buffer ----- ///
	StructuredBuffer<TerrainVertex> rwVertices_;
	StructuredBuffer<uint32_t> rwIndices_;
	bool isCreated_;

	/// ----- edit ----- ///
	TerrainEditorInfo editorInfo_;

	/// ----- terrain ----- ///
	Vector2 terrainSize_ = Vector2(1000.0f, 1000.0f); ///< 地形のサイズ
	uint32_t maxVertexNum_;
	uint32_t maxIndexNum_;

	Asset::Material material_;

	/// ----- river ----- ///
	River river_;

	/// ----- splatting ----- ///
	std::array<std::string, kMaxTerrainTextureNum> splattingTexPaths_;

	/// ----- flags ----- ///
	bool isRenderingProcedural_;


public:
	/// ===================================================
	/// public : accessor
	/// ====================================================

	const std::array<std::string, kMaxTerrainTextureNum>& GetSplatTexPaths() const;

	/// ----- buffer ----- ///
	const StructuredBuffer<TerrainVertex>& GetRwVertices() const;
	const StructuredBuffer<uint32_t>& GetRwIndices() const;
	DxResource& GetVerticesResource();

	void SetIsCreated(bool isCreated);
	bool GetIsCreated() const;

	uint32_t GetMaxVertexNum();
	uint32_t GetMaxIndexNum();

	const Vector2& GetSize() const;

	/// ----- edit ----- ///
	const TerrainEditorInfo& GetEditorInfo() const;

	/// ----- river ----- ///
	River* GetRiver();

	/// ----- flags ----- ///
	bool GetIsRenderingProcedural() const;
	void SetIsRenderingProcedural(bool isRenderingProcedural);

};



} /// ONEngine
