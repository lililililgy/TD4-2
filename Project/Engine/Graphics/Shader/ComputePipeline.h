#pragma once

/// directX
#include <d3d12.h>

/// std
#include <vector>
#include <string>

/// engine
#include "Engine/Core/DirectX12/ComPtr/ComPtr.h"
#include "Shader.h"

/// ///////////////////////////////////////////////////
/// ComputePipeline
/// ///////////////////////////////////////////////////
namespace ONEngine {

class ComputePipeline {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	ComputePipeline();
	~ComputePipeline();

	/// @brief pipelineを生成する
	/// @param dxDevice DxDeviceへのポインタ
	void CreatePipeline(class DxDevice* dxDevice);


	/*--- root signature ---*/

	/// @brief 使用する shaderへのポインタをセットする
	/// @param shader 使用するshader
	void SetShader(Shader* shader);

	/// @brief constant buffer viewを追加する
	/// @param shaderVisibility shaderの種類
	/// @param shaderRegister  register(b0)の0の部分
	void AddCBV(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister);

	/// @brief 32bit constantを追加する
	/// @param shaderVisibility shaderの種類
	/// @param shaderRegister   register(b0)の0の部分
	void Add32BitConstant(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister, uint32_t num32bitValue = 1u);

	/// @brief descriptor rangeを追加する
	/// @param baseShaderRegister register(b0)の0の部分
	/// @param numDescriptor      descriptorの数
	/// @param rangeType          descriptorの種類(CBV, SRV, UAV)
	void AddDescriptorRange(uint32_t baseShaderRegister, uint32_t numDescriptor, D3D12_DESCRIPTOR_RANGE_TYPE  rangeType);

	/// @brief descriptor tableを追加する
	/// @param shaderVisibility 使用するshaderの種類(vs, ps)
	/// @param descriptorIndex descriptor rangeの配列のインデックス
	void AddDescriptorTable(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t descriptorIndex);

	/// @brief static samplerを追加する
	/// @param shaderVisibility 使用するshaderの種類(vs, ps)
	/// @param shaderRegister   shaderのregister(s0)の0の部分
	/// @param isComparisonSampler 比較サンプラーにするかどうか
	void AddStaticSampler(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister, bool isComparisonSampler = false);

	/// @brief fill modeを設定する
	/// @param fillMode 設定するfill mode
	void SetFillMode(D3D12_FILL_MODE fillMode);

	/// @brief カリングの設定
	/// @param cullMode カリングモード
	void SetCullMode(D3D12_CULL_MODE cullMode);

	/// @brief TopologyTypeを設定する
	/// @param topologyType 設定するtopology type
	void SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);


	/*--- pipeline state ---*/

	/// @brief コマンドリストにパイプラインステートをセットする
	/// @param dxCommand command listを管理しているクラスへのポインタ
	void SetPipelineStateForCommandList(class DxCommand* dxCommand);


private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	/// @brief root signatureを生成する
	/// @param dxDevice DxDeviceへのポインタ
	void CreateRootSignature(class DxDevice* dxDevice);

	/// @brief pipeline state objectを生成する
	/// @param dxDevice DxDeviceへのポインタ
	void CreatePipelineStateObject(class DxDevice* dxDevice);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	Shader* shader_;

	ComPtr<ID3D12RootSignature>             rootSignature_;
	ComPtr<ID3D12PipelineState>             pipelineState_;

	D3D12_RASTERIZER_DESC                   rasterizerDesc_;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE           primitiveTopologyType_;
	D3D12_BLEND_DESC                        blendDesc_;

	std::vector<D3D12_ROOT_PARAMETER>       rootParameters_;
	std::vector<D3D12_STATIC_SAMPLER_DESC>  staticSamplers_;
	std::vector<D3D12_DESCRIPTOR_RANGE>     descriptorRanges_;
};


} /// ONEngine
