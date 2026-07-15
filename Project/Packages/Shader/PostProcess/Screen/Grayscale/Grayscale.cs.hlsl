#include "../../../ConstantBufferData/Material.hlsli"

struct GrayscaleParams {
	int2 offset;
	int2 padding;
};
ConstantBuffer<GrayscaleParams> gParams : register(b0);

/// texture
Texture2D<float4> colorTex : register(t0);
RWTexture2D<float4> outputTex : register(u0);
SamplerState textureSampler : register(s0);

[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID) {
	uint2 pixelPos = dispatchId.xy + gParams.offset;
	float2 texCoord = float2(pixelPos.x / 1920.0f, pixelPos.y / 1080.0f);
	float4 color = colorTex.Sample(textureSampler, texCoord);

	float value = dot(color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
	outputTex[pixelPos] = float4(value, value, value, color.a);
}