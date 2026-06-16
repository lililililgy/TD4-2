#pragma once

/// directx
#include <d3d12.h>

/// std
#include <vector>
#include <string>
#include <optional>

/// engine
#include "Engine/Core/DirectX12/ComPtr/ComPtr.h"
#include "Shader.h"

/// @brief RenderTargetViewのフォーマット
enum class RTVFormat {
	Color         = DXGI_FORMAT_R8G8B8A8_UNORM,     ///< 色
	WorldPosition = DXGI_FORMAT_R16G16B16A16_FLOAT, ///< ワールド座標
	Normal        = DXGI_FORMAT_R16G16B16A16_FLOAT, ///< 法線
	Flags         = DXGI_FORMAT_R32G32B32A32_FLOAT  ///< フラグ
};

/// @brief MRT用RenderTargetViewのインデックス
enum class RTVIndex {
	Color         = 0, ///< 色
	WorldPosition = 1, ///< ワールド座標
	Normal        = 2, ///< 法線
	Flags         = 3, ///< フラグ
	Count /// 要素数
};


/// ///////////////////////////////////////////////////
/// グラフィクス用	pipeline
/// ///////////////////////////////////////////////////
namespace ONEngine {

class GraphicsPipeline {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	GraphicsPipeline();
	~GraphicsPipeline();

	/// @brief 今までにセットした値を使ってパイプラインを生成する
	void CreatePipeline(class DxDevice* dxDevice);


	/*--- root signature ---*/

	/// @brief 使用する shaderへのポインタをセットする
	/// @param shader 使用するshader
	void SetShader(Shader* shader);

	/// @brief InputElementを追加する
	/// @param semanticName   セマンティクスの名前
	/// @param semanticIndex  セマンティクスのインデックス
	/// @param format         フォーマットの種類
	void AddInputElement(const std::string& semanticName, uint32_t semanticIndex, DXGI_FORMAT format, UINT inputSlot = 0u);

	/// @brief constant buffer viewを追加する
	/// @param shaderVisibility shaderの種類
	/// @param shaderRegister  register(b0)の0の部分
	void AddCBV(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister);

	/// @brief shader resource viewを追加する
	/// @param shaderVisibility shaderの種類
	/// @param shaderRegister  register(t0)の0の部分
	void AddSRV(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister);

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
	void AddStaticSampler(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister);
	void AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& samplerDesc, D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister);

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

	/// @brief rasterizer descを設定する
	/// @param desc 
	void SetRasterizerDesc(const D3D12_RASTERIZER_DESC& desc);

	/// @brief depth stencil descを設定する
	/// @param desc 
	void SetDepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC& desc);

	/// @brief blend descを設定する
	/// @param desc 設定するblend desc
	void SetBlendDesc(const D3D12_BLEND_DESC& desc);

	/// @brief render target viewの数を設定する
	/// @param rtvNum rtvの数
	void SetRTVNum(uint32_t rtvNum);

	/// @brief rtvのフォーマットを設定する
	/// @param rtvFormats rtvのフォーマットarray
	void SetRTVFormats(const std::vector<DXGI_FORMAT>& rtvFormats);

	/// @brief rtvのフォーマットを設定する
	/// @param rtvFormat rtvのフォーマット
	/// @param rtvIndex setするrtvのインデックス
	void SetRTVFormat(DXGI_FORMAT rtvFormat, uint32_t rtvIndex);

	/// @brief コマンドリストにパイプラインステートをセットする
	/// @param dxCommand command listを管理しているクラスへのポインタ
	void SetPipelineStateForCommandList(class DxCommand* dxCommand);

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	/// @brief root signatureを生成する
	void CreateRootSignature(class DxDevice* dxDevice);

	/// @brief pipeline state objectを生成する
	void CreatePipelineStateObject(class DxDevice* dxDevice);

	void CreateMeshPipelineStateObject(class DxDevice* dxDevice);


private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	ComPtr<ID3D12RootSignature>             rootSignature_;
	ComPtr<ID3D12PipelineState>             pipelineState_;


	/// parameters
	std::vector<D3D12_INPUT_ELEMENT_DESC>   inputElements_;
	std::vector<std::string>                semanticNames_;

	D3D12_RASTERIZER_DESC                   rasterizerDesc_;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE           primitiveTopologyType_;
	D3D12_BLEND_DESC                        blendDesc_;

	std::vector<D3D12_ROOT_PARAMETER>       rootParameters_;
	std::vector<D3D12_STATIC_SAMPLER_DESC>  staticSamplers_;
	std::vector<D3D12_DESCRIPTOR_RANGE>     descriptorRanges_;

	Shader* pShader_;


	/// pipeline settings
	std::optional<D3D12_DEPTH_STENCIL_DESC> depthStancilDesc_;
	uint32_t                                rtvNum_ = 1;
	std::vector<DXGI_FORMAT>                rtvFormats_;
};

/// @brief DepthStencilDescのデフォルト値を返す
D3D12_DEPTH_STENCIL_DESC DefaultDepthStencilDesc();
D3D12_DEPTH_STENCIL_DESC DepthNone();
D3D12_DEPTH_STENCIL_DESC TelegraphDepthStencilDesc();
D3D12_DEPTH_STENCIL_DESC DepthRead();


/// @brief BlendMode別のBlendDescを返す名前空間
namespace BlendMode {
	D3D12_BLEND_DESC Normal();
	D3D12_BLEND_DESC Add();
	D3D12_BLEND_DESC Subtract();
	D3D12_BLEND_DESC Multiply();
	D3D12_BLEND_DESC Screen();
	D3D12_BLEND_DESC None();
}

namespace StaticSampler {
	D3D12_STATIC_SAMPLER_DESC ClampSampler();
}

} /// ONEngine
