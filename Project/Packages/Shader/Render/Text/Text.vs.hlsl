#include "Text.hlsli"

#include "../../ConstantBufferData/ViewProjection.hlsli"

ConstantBuffer<ViewProjection> viewProjection : register(b0);

VSOutput main(VSInput input) {
	VSOutput output;

	output.position   = mul(float4(input.position, 1.0), viewProjection.matVP);
	output.uv         = input.uv;
	output.instanceId = input.materialId;

	return output;
}
