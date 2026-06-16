#pragma once

/// std
#include <vector>
#include <cstdint>

/// engine
#include "Engine/Core/Utility/Math/Vector4.h"
#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Vector2.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"
#include "Engine/Graphics/Buffer/StructuredBuffer.h"
#include "Engine/Graphics/Buffer/Data/GPUMaterial.h"

/// ///////////////////////////////////////////////////
/// 川のコントロールポイント
/// ///////////////////////////////////////////////////
namespace ONEngine {

struct RiverControlPoint {
	Vector3 position;
	float width;
};

/// ///////////////////////////////////////////////////
/// 川のメッシュ頂点
/// ///////////////////////////////////////////////////
struct RiverVertex {
	Vector4 position;
	Vector2 uv;
	Vector3 normal;
};


/// ///////////////////////////////////////////////////
/// spline曲線の計算関数
/// ///////////////////////////////////////////////////
RiverControlPoint CatmullRom(
	const RiverControlPoint& p0,
	const RiverControlPoint& p1,
	const RiverControlPoint& p2,
	const RiverControlPoint& p3,
	float t
);

std::vector<RiverControlPoint> SampleRiverSpline(
	const std::vector<RiverControlPoint>& points,
	int samplePerSegment
);


/// ///////////////////////////////////////////////////
/// 川
/// ///////////////////////////////////////////////////
class River {
public:

	struct Param {
		uint32_t totalSegments;
		uint32_t totalVertices;
		uint32_t totalSamples;
		uint32_t samplePerSegment;
	};

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	River();
	~River();

	/// ----- edit ----- ///
	void Edit(class EntityComponentSystem* ecs);
	void SaveToJson(const std::string& name);
	void LoadFromJson(const std::string& name);


	/// @brief Spline曲線をGizmoで描画する
	void DrawSplineCurve();

	/// @brief Bufferを生成する
	/// @param dxDevice DxDeviceへのポインタ
	/// @param dxSRVHeap DxSRVHeapへのポインタ
	/// @param dxCommand DxCommandへのポインタ
	void CreateBuffers(class DxDevice* dxDevice, class DxSRVHeap* dxSRVHeap, class DxCommand* dxCommand);

	/// @brief Bufferデータをセットする
	void SetBufferData();


	/// @brief MaterialDataをセットする
	/// @param entityId OwnerEntityのID
	/// @param texIndex 川に使用するテクスチャのIndex
	void SetMaterialData(int32_t entityId, int32_t texIndex);


	/// @brief VBVとIBVのバリアを生成する(描画用に)
	/// @param dxCommand DxCommandへのポインタ
	void CreateRenderingBarriers(class DxCommand* dxCommand);

	/// @brief VBVとIBVのバリアを復元する(計算用に)
	/// @param dxCommand DxCommandへのポインタ
	void RestoreResourceBarriers(class DxCommand* dxCommand);


	/// @brief 描画用にVBVを生成する
	D3D12_VERTEX_BUFFER_VIEW CreateVBV();
	/// @brief 描画用にIBVを生成する
	D3D12_INDEX_BUFFER_VIEW CreateIBV();

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	/// splineのコントロールポイント
	std::vector<RiverControlPoint> controlPoints_;
	std::vector<RiverControlPoint> createdPoints_;
	int samplePerSegment_;

	/// buffer
	ConstantBuffer<Param> paramBuf_;
	ConstantBuffer<GPUMaterial> materialBuffer_;
	StructuredBuffer<RiverControlPoint> controlPointBuf_;
	StructuredBuffer<RiverVertex> rwVertices_;
	StructuredBuffer<uint32_t> rwIndices_;
	bool isCreatedBuffers_;
	UINT totalVertices_;
	UINT totalIndices_;

	/// edit
	bool isGenerateMeshRequest_;

public:
	/// ==================================================
	/// public : accessors
	/// ==================================================

	int GetSamplePerSegment() const;
	int GetNumControlPoint() const;
	bool GetIsGenerateMeshRequest() const;
	void SetIsGenerateMeshRequest(bool request);

	const ConstantBuffer<Param>& GetParamBuffer() const;
	const ConstantBuffer<GPUMaterial>& GetMaterialBuffer() const;
	const StructuredBuffer<RiverVertex>& GetRwVertices() const;
	const StructuredBuffer<uint32_t>& GetRwIndices() const;
	const StructuredBuffer<RiverControlPoint>& GetControlPointBuffer() const;
	bool GetIsCreatedBuffers() const;
	UINT GetTotalIndices() const;
	UINT GetTotalVertices() const;
};

} /// ONEngine
