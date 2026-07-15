Texture2D<float4> colorTex : register(t0);
Texture2D<float4> blurTex : register(t1);
Texture2D<float4> flagsTex : register(t2);
RWTexture2D<float4> outputTex : register(u0);
SamplerState textureSampler : register(s0);

[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID) {
	float2 texCoord = float2(dispatchId.x / 1920.0f, dispatchId.y / 1080.0f);
	float4 originalColor = colorTex.SampleLevel(textureSampler, texCoord, 0.0f);
	float4 blurredColor = blurTex.SampleLevel(textureSampler, texCoord, 0.0f);
	float4 flags = flagsTex[dispatchId.xy];

	float intensity = flags.z;

	float3 finalColor = originalColor.rgb + (blurredColor.rgb * intensity);

	outputTex[dispatchId.xy] = float4(finalColor, originalColor.a);
}
