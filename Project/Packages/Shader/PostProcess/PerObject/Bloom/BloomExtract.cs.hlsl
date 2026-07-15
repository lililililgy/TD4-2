#include "../../../ConstantBufferData/Material.hlsli"

Texture2D<float4> colorTex : register(t0);
Texture2D<float4> flagsTex : register(t1);
RWTexture2D<float4> outputTex : register(u0);
SamplerState textureSampler : register(s0);

[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID) {
	float4 flags = flagsTex[dispatchId.xy];
	
	if (!IsPostEffectEnabled((int)flags.x, PostEffectFlags_Bloom)) {
		outputTex[dispatchId.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
		return;
	}

	float2 texCoord = float2(dispatchId.x / 1920.0f, dispatchId.y / 1080.0f);
	float4 color = colorTex.SampleLevel(textureSampler, texCoord, 0.0f);

	float luminance = dot(color.rgb, float3(0.2125f, 0.7154f, 0.0721f));

	float threshold = flags.w;

	if (luminance > threshold) {
		uint packed = asuint(flags.z);
		float bloomRadius = f16tof32(packed >> 16);
		outputTex[dispatchId.xy] = float4(color.rgb, bloomRadius / 30.0f);
	} else {
		outputTex[dispatchId.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}
