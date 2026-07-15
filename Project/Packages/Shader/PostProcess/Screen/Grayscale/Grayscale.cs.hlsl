#include "../../../ConstantBufferData/Material.hlsli"

struct GrayscaleParams {
	int2 offset;
	int2 virtualSize;
};
ConstantBuffer<GrayscaleParams> gParams : register(b0);

/// texture
Texture2D<float4> colorTex : register(t0);
RWTexture2D<float4> outputTex : register(u0);
SamplerState textureSampler : register(s0);

[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID) {
	uint2 localPos = dispatchId.xy;
	uint2 pixelPos = localPos + gParams.offset;
	float2 localUV = float2(localPos) / float2(gParams.virtualSize);
	float2 texCoord = (localUV * float2(gParams.virtualSize) + float2(gParams.offset)) / float2(1920.0f, 1080.0f);
	float4 color = colorTex.Sample(textureSampler, texCoord);

	float value = dot(color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
	outputTex[pixelPos] = float4(value, value, value, color.a);
}