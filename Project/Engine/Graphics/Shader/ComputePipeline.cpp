#include "ComputePipeline.h"

using namespace ONEngine;
#include <iostream>

/// engine
#include "Engine/Core/DirectX12/Device/DxDevice.h"
#include "Engine/Core/DirectX12/Command/DxCommand.h"
#include "Engine/Core/Utility/Tools/Assert.h"
#include "Engine/Core/Utility/Tools/Log.h"

ComputePipeline::ComputePipeline() {
	rootParameters_.reserve(16);
}
ComputePipeline::~ComputePipeline() = default;

void ComputePipeline::CreatePipeline(DxDevice* dxDevice) {
	CreateRootSignature(dxDevice);
	CreatePipelineStateObject(dxDevice);
}



void ComputePipeline::SetShader(Shader* shader) {
	shader_ = shader;
}

void ComputePipeline::AddCBV(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister) {
	/// ----- CBVの追加 ----- ///

	D3D12_ROOT_PARAMETER parameter{};
	parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameter.ShaderVisibility = shaderVisibility;
	parameter.Descriptor.ShaderRegister = shaderRegister;

	rootParameters_.push_back(parameter);
}

void ComputePipeline::Add32BitConstant(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister, uint32_t num32bitValue) {
	/// ----- 32bit constantの追加 ----- ///

	D3D12_ROOT_PARAMETER parameter{};
	parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	parameter.ShaderVisibility = shaderVisibility;
	parameter.Descriptor.ShaderRegister = shaderRegister;
	parameter.Constants.Num32BitValues = num32bitValue;

	rootParameters_.push_back(parameter);
}

void ComputePipeline::AddDescriptorRange(uint32_t baseShaderRegister, uint32_t numDescriptor, D3D12_DESCRIPTOR_RANGE_TYPE  rangeType) {
	/// ----- descriptor rangeの追加 ----- ///

	D3D12_DESCRIPTOR_RANGE range{};
	range.BaseShaderRegister = baseShaderRegister;
	range.NumDescriptors = numDescriptor;
	range.RangeType = rangeType;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRanges_.push_back(range);
}

void ComputePipeline::AddDescriptorTable(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t descriptorIndex) {
	/// ----- descriptor tableの追加 ----- ///

	/// 範囲外チェック
	Assert(descriptorRanges_.size() > descriptorIndex, "out of range...");

	D3D12_ROOT_PARAMETER parameter{};
	parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameter.ShaderVisibility = shaderVisibility;
	parameter.DescriptorTable.pDescriptorRanges = &descriptorRanges_[descriptorIndex];
	parameter.DescriptorTable.NumDescriptorRanges = 1;

	rootParameters_.push_back(parameter);
}

void ComputePipeline::AddStaticSampler(D3D12_SHADER_VISIBILITY shaderVisibility, uint32_t shaderRegister, bool isComparisonSampler) {
	/// ----- static samplerの追加 ----- ///

	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; /// 0~1の範囲外をリピート
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;               /// ありったけのMipMapを使う
	sampler.ShaderRegister = shaderRegister;                 /// 使用するRegister番号
	sampler.ShaderVisibility = shaderVisibility;

	if (isComparisonSampler) {
		// 比較サンプラー（シャドウマップ用）
		sampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	} else {
		// 通常のサンプラー
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	}

	staticSamplers_.push_back(sampler);
}

void ComputePipeline::SetFillMode(D3D12_FILL_MODE fillMode) {
	rasterizerDesc_.FillMode = fillMode;
}

void ComputePipeline::SetCullMode(D3D12_CULL_MODE cullMode) {
	rasterizerDesc_.CullMode = cullMode;
}

void ComputePipeline::SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType) {
	primitiveTopologyType_ = topologyType;
}

void ComputePipeline::SetPipelineStateForCommandList(DxCommand* dxCommand) {
	dxCommand->GetCommandList()->SetPipelineState(pipelineState_.Get());
	dxCommand->GetCommandList()->SetComputeRootSignature(rootSignature_.Get());
}



void ComputePipeline::CreateRootSignature(DxDevice* dxDevice) {
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
		Console::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		Assert(false, "error...");
	}

	/// バイナリを元に生成
	hr = dxDevice->GetDevice()->CreateRootSignature(
		0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_)
	);

	Assert(SUCCEEDED(hr), "error...");
}

void ComputePipeline::CreatePipelineStateObject(DxDevice* dxDevice) {
	/// ----- pipeline state objectの生成 ----- ///

	/// pipeline state desc
	D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = rootSignature_.Get();

	/// shader setting
	desc.CS = {
		shader_->GetCS()->GetBufferPointer(),
		shader_->GetCS()->GetBufferSize()
	};

	/// pipeline state objectの生成
	HRESULT result = dxDevice->GetDevice()->CreateComputePipelineState(
		&desc, IID_PPV_ARGS(&pipelineState_)
	);

	Assert(SUCCEEDED(result), "error...");
}
