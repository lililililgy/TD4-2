#include "GraphicsPipeline.h"

using namespace ONEngine;

/// directx
#include <DirectX-Headers/include/directx/d3dx12_pipeline_state_stream.h>
#include <comdef.h>

/// engine
#include "Engine/Core/DirectX12/Device/DxDevice.h"
#include "Engine/Core/DirectX12/Command/DxCommand.h"
#include "Engine/Core/Utility/Tools/Assert.h"
#include "Engine/Core/Utility/Tools/Log.h"

GraphicsPipeline::GraphicsPipeline() {

	/// デフォルトのRTVを設定
	SetRTVNum(static_cast<uint32_t>(RTVIndex::Count)); /// 色、ワールド座標、法線、フラグ
	SetRTVFormat(static_cast<DXGI_FORMAT>(RTVFormat::Color), static_cast<int>(RTVIndex::Color));
	SetRTVFormat(static_cast<DXGI_FORMAT>(RTVFormat::WorldPosition), static_cast<int>(RTVIndex::WorldPosition));
	SetRTVFormat(static_cast<DXGI_FORMAT>(RTVFormat::Normal), static_cast<int>(RTVIndex::Normal));
	SetRTVFormat(static_cast<DXGI_FORMAT>(RTVFormat::Flags), static_cast<int>(RTVIndex::Flags));


	/// メンバ変数の初期化
	rootSignature_ = nullptr;
	pipelineState_ = nullptr;

	inputElements_ = {};
	semanticNames_ = {};

	rasterizerDesc_ = {};
	primitiveTopologyType_ = {};
	blendDesc_ = {};

	rootParameters_ = {};
	staticSamplers_ = {};
	descriptorRanges_ = {};

	pShader_ = nullptr;
}
GraphicsPipeline::~GraphicsPipeline() {}

void GraphicsPipeline::CreatePipeline(DxDevice* dxDevice) {
	/// root signatureとpipeline state objectを生成する
	CreateRootSignature(dxDevice);

	if (pShader_->GetMS() != nullptr) {
		CreateMeshPipelineStateObject(dxDevice);
	} else {
		CreatePipelineStateObject(dxDevice);
	}
}


void GraphicsPipeline::SetShader(Shader* shader) {
	pShader_ = shader;
}

void GraphicsPipeline::AddInputElement(const std::string& semanticName, uint32_t semanticIndex, DXGI_FORMAT format, UINT inputSlot) {
	/// ----- Input Elementを追加 ----- ///

	D3D12_INPUT_ELEMENT_DESC element = {};
	element.SemanticName = semanticName.c_str();
	element.SemanticIndex = semanticIndex;
	element.Format = format;
	element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	element.InputSlot = inputSlot;

	inputElements_.push_back(element);
	semanticNames_.push_back(semanticName);
}

void GraphicsPipeline::AddCBV(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister) {
	/// ----- Constant Buffer Viewを追加 ----- ///

	D3D12_ROOT_PARAMETER parameter{};
	parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameter.ShaderVisibility = shaderVisibility;
	parameter.Descriptor.ShaderRegister = shaderRegister;

	rootParameters_.push_back(parameter);
}

void GraphicsPipeline::AddSRV(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister) {
	/// ----- Shader Resource Viewを追加 ----- ///

	D3D12_ROOT_PARAMETER parameter{};
	parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	parameter.ShaderVisibility = shaderVisibility;
	parameter.Descriptor.ShaderRegister = shaderRegister;

	rootParameters_.push_back(parameter);
}

void GraphicsPipeline::Add32BitConstant(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister, uint32_t num32bitValue) {
	/// ----- 32bit定数を追加 ----- ///

	D3D12_ROOT_PARAMETER parameter{};
	parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	parameter.ShaderVisibility = shaderVisibility;
	parameter.Descriptor.ShaderRegister = shaderRegister;
	parameter.Constants.Num32BitValues = num32bitValue;

	rootParameters_.push_back(parameter);
}

void GraphicsPipeline::AddDescriptorRange(uint32_t baseShaderRegister, uint32_t numDescriptor, D3D12_DESCRIPTOR_RANGE_TYPE  rangeType) {
	/// ----- Descriptor Rangeを追加 ----- ///

	D3D12_DESCRIPTOR_RANGE range{};
	range.BaseShaderRegister = baseShaderRegister;
	range.NumDescriptors = numDescriptor;
	range.RangeType = rangeType;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRanges_.push_back(range);
}

void GraphicsPipeline::AddDescriptorTable(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t descriptorIndex) {
	/// ----- Descriptor Tableを追加 ----- ///

	Assert(descriptorRanges_.size() >= descriptorIndex, "out of range...");

	D3D12_ROOT_PARAMETER parameter{};
	parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameter.ShaderVisibility = shaderVisibility;
	parameter.DescriptorTable.pDescriptorRanges = &descriptorRanges_[descriptorIndex];
	parameter.DescriptorTable.NumDescriptorRanges = 1;

	rootParameters_.push_back(parameter);
}

void GraphicsPipeline::AddStaticSampler(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister) {
	/// ----- Static Samplerを追加 ----- ///

	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; /// バイリニアフィルタ
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; /// 0~1の範囲外をリピート
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;     /// 比較しない
	sampler.MaxLOD = D3D12_FLOAT32_MAX;               /// ありったけのMipMapを使う
	sampler.ShaderRegister = shaderRegister;                 /// 使用するRegister番号
	sampler.ShaderVisibility = shaderVisibility;

	staticSamplers_.push_back(sampler);
}

void GraphicsPipeline::AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& samplerDesc, D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister) {
	/// ----- Static Samplerを追加 ----- ///

	D3D12_STATIC_SAMPLER_DESC sampler = samplerDesc;
	sampler.ShaderRegister   = shaderRegister;
	sampler.ShaderVisibility = shaderVisibility;

	staticSamplers_.push_back(sampler);
}

void GraphicsPipeline::SetFillMode(D3D12_FILL_MODE fillMode) {
	rasterizerDesc_.FillMode = fillMode;
}

void GraphicsPipeline::SetCullMode(D3D12_CULL_MODE cullMode) {
	rasterizerDesc_.CullMode = cullMode;
}

void GraphicsPipeline::SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType) {
	primitiveTopologyType_ = topologyType;
}

void GraphicsPipeline::SetRasterizerDesc(const D3D12_RASTERIZER_DESC& desc) {
	rasterizerDesc_ = desc;
}

void GraphicsPipeline::SetDepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC& desc) {
	depthStancilDesc_ = desc;
}

void GraphicsPipeline::SetBlendDesc(const D3D12_BLEND_DESC& desc) {
	blendDesc_ = desc;
}

void GraphicsPipeline::SetRTVNum(uint32_t rtvNum) {
	Assert(rtvNum <= 8, "the number of rtv is less than 8"); /// RTVの数は8以下
	rtvNum_ = rtvNum;
}

void GraphicsPipeline::SetRTVFormats(const std::vector<DXGI_FORMAT>& rtvFormats) {
	Assert(rtvFormats.size() <= 8, "the number of rtv is less than 8"); /// RTVの数は8以下
	rtvFormats_ = rtvFormats;
}

void GraphicsPipeline::SetRTVFormat(DXGI_FORMAT rtvFormat, uint32_t rtvIndex) {
	Assert(rtvIndex < rtvNum_, "out of range...");
	if (rtvFormats_.size() <= rtvIndex) {
		rtvFormats_.resize(rtvIndex + 1);
	}
	rtvFormats_[rtvIndex] = rtvFormat;
}

void GraphicsPipeline::SetPipelineStateForCommandList(DxCommand* dxCommand) {
	dxCommand->GetCommandList()->SetPipelineState(pipelineState_.Get());
	dxCommand->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
}




void GraphicsPipeline::CreateRootSignature(DxDevice* dxDevice) {
	/// ----- root signatureの生成 ----- ///

	HRESULT hr = S_FALSE;
	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;

	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	desc.pParameters = rootParameters_.data();					/// RootParameter配列へのポインタ
	desc.NumParameters = static_cast<UINT>(rootParameters_.size());	/// RootParameterの配列の長さ
	desc.pStaticSamplers = staticSamplers_.data();					/// StaticSampler配列へのポインタ
	desc.NumStaticSamplers = static_cast<UINT>(staticSamplers_.size());	/// StaticSamplerの配列の長さ

	/// シリアライズしてバイナリ
	hr = D3D12SerializeRootSignature(
		&desc, D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob, &errorBlob
	);

	if (FAILED(hr)) {
		Assert(false, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	}

	/// バイナリを元に生成
	hr = dxDevice->GetDevice()->CreateRootSignature(
		0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_)
	);

	Assert(SUCCEEDED(hr), "error...");
}

void GraphicsPipeline::CreatePipelineStateObject(DxDevice* dxDevice) {
	/// ----- pipeline state objectの生成 ----- ///

	/// input layoutの設定
	for (uint32_t i = 0; i < inputElements_.size(); ++i) {
		inputElements_[i].SemanticName = semanticNames_[i].c_str();
	}
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElements_.data();
	inputLayoutDesc.NumElements = static_cast<UINT>(inputElements_.size());


	/// pipeline state desc
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = rootSignature_.Get();
	desc.InputLayout = inputLayoutDesc;

	/// shader setting
	desc.VS = {
		pShader_->GetVS()->GetBufferPointer(),
		pShader_->GetVS()->GetBufferSize()
	};

	desc.PS = {
		pShader_->GetPS()->GetBufferPointer(),
		pShader_->GetPS()->GetBufferSize()
	};

	/// depth stencil desc
	if (depthStancilDesc_.has_value()) {
		desc.DepthStencilState = depthStancilDesc_.value();
	}

	desc.BlendState = blendDesc_;           /// blend desc
	desc.RasterizerState = rasterizerDesc_; /// rasterizer desc

	desc.NumRenderTargets = rtvNum_;
	for (uint32_t i = 0; i < rtvNum_; ++i) {
		Assert(rtvFormats_.size() > i, "out of range...");
		desc.RTVFormats[i] = rtvFormats_[i];
	}

	desc.PrimitiveTopologyType = primitiveTopologyType_;
	desc.SampleDesc.Count = 1;
	desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	/// pipeline state objectの生成
	HRESULT result = dxDevice->GetDevice()->CreateGraphicsPipelineState(
		&desc, IID_PPV_ARGS(&pipelineState_)
	);

	if (FAILED(result)) {
		Console::Log("[error] " + HrToString(result));
		Assert(false, HrToString(result).c_str());
	}
}

void GraphicsPipeline::CreateMeshPipelineStateObject(DxDevice* dxDevice) {
	/// ----- pipeline state objectの生成(MeshShader用) ----- ///

	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC meshDesc = {};
	meshDesc.pRootSignature = rootSignature_.Get();


	if (pShader_->GetAS()) {
		meshDesc.AS = {
			pShader_->GetAS()->GetBufferPointer(),
			pShader_->GetAS()->GetBufferSize()
		};
	}

	meshDesc.MS = {
		pShader_->GetMS()->GetBufferPointer(),
		pShader_->GetMS()->GetBufferSize()
	};

	meshDesc.PS = {
		pShader_->GetPS()->GetBufferPointer(),
		pShader_->GetPS()->GetBufferSize()
	};

	/// depth stencil desc
	if (depthStancilDesc_.has_value()) {
		meshDesc.DepthStencilState = depthStancilDesc_.value();
	}

	meshDesc.BlendState = blendDesc_;
	meshDesc.RasterizerState = rasterizerDesc_;

	meshDesc.NumRenderTargets = rtvNum_;
	for (uint32_t i = 0; i < rtvNum_; ++i) {
		Assert(rtvFormats_.size() > i, "out of range...");
		meshDesc.RTVFormats[i] = rtvFormats_[i];
	}

	meshDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	meshDesc.SampleDesc.Count = 1;
	meshDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	meshDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	CD3DX12_PIPELINE_MESH_STATE_STREAM psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(meshDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);

	/// pipeline state objectの生成
	HRESULT result = dxDevice->GetDevice10()->CreatePipelineState(
		&streamDesc, IID_PPV_ARGS(&pipelineState_)
	);

	Assert(SUCCEEDED(result), HrToString(result).c_str());
}

D3D12_BLEND_DESC BlendMode::Normal() {
	D3D12_BLEND_DESC blendDesc = {};
	for (size_t i = 0; i < 8; i++) {
		blendDesc.RenderTarget[i].BlendEnable = TRUE;
		blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	return blendDesc;
}

D3D12_BLEND_DESC BlendMode::Add() {
	D3D12_BLEND_DESC blendDesc = {};
	for (size_t i = 0; i < 8; i++) {
		blendDesc.RenderTarget[i].BlendEnable = TRUE;
		blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	return blendDesc;
}

D3D12_BLEND_DESC BlendMode::Subtract() {
	D3D12_BLEND_DESC blendDesc = {};
	for (size_t i = 0; i < 8; i++) {
		blendDesc.RenderTarget[i].BlendEnable = TRUE;
		blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	return blendDesc;
}

D3D12_BLEND_DESC BlendMode::Multiply() {
	D3D12_BLEND_DESC blendDesc = {};
	for (size_t i = 0; i < 8; i++) {
		blendDesc.RenderTarget[i].BlendEnable = TRUE;
		blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_DEST_COLOR;
		blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	return blendDesc;
}

D3D12_BLEND_DESC BlendMode::Screen() {
	D3D12_BLEND_DESC blendDesc = {};
	for (size_t i = 0; i < 8; i++) {
		blendDesc.RenderTarget[i].BlendEnable = TRUE;
		blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_COLOR;
		blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	return blendDesc;
}

D3D12_BLEND_DESC BlendMode::None() {
	D3D12_BLEND_DESC blendDesc = {};

	D3D12_RENDER_TARGET_BLEND_DESC rtBlend = {};
	rtBlend.BlendEnable = FALSE;   // ブレンド無効
	rtBlend.LogicOpEnable = FALSE; // 論理演算も無効
	rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


	for (size_t i = 0; i < 8; i++) {
		blendDesc.RenderTarget[i] = rtBlend;
	}

	return blendDesc;
}

D3D12_DEPTH_STENCIL_DESC ONEngine::DefaultDepthStencilDesc() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	return depthStencilDesc;
}

D3D12_DEPTH_STENCIL_DESC ONEngine::DepthNone() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.StencilEnable = FALSE;
	return depthStencilDesc;
}

D3D12_DEPTH_STENCIL_DESC ONEngine::TelegraphDepthStencilDesc() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 深度は書き込まない
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;    // 常に描画（タイルの下に隠れない）
	return depthStencilDesc;
}

D3D12_DEPTH_STENCIL_DESC ONEngine::DepthRead() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 深度は書き込まない
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 読み取りは行う
	return depthStencilDesc;
}

D3D12_STATIC_SAMPLER_DESC StaticSampler::ClampSampler() {
	D3D12_STATIC_SAMPLER_DESC sampler = {};

	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	sampler.MipLODBias = 0.0f;
	sampler.MaxAnisotropy = 16;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK; // Clamp では不要だが初期化上書き
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;

	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	return sampler;
}

D3D12_STATIC_SAMPLER_DESC StaticSampler::PointSampler() {
	D3D12_STATIC_SAMPLER_DESC sampler = {};

	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	sampler.MipLODBias = 0.0f;
	sampler.MaxAnisotropy = 16;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;

	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	return sampler;
}
