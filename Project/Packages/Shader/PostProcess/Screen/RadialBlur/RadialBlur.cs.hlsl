

static const uint2 TextureSize = uint2(1920, 1080);

struct RadialBlurParams {
	int2 offset;
	int2 virtualSize;
};
ConstantBuffer<RadialBlurParams> gParams : register(b0);

/// Texture
Texture2D<float4> colorTexture : register(t0);
RWTexture2D<float4> outputTexture : register(u0);
SamplerState textureSampler : register(s0);

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
	uint2 localPos = dispatchThreadID.xy;
	uint2 pixelPos = localPos + gParams.offset;

	const float2 kCenter = float2(0.5, 0.5);
	const int kNumSamples = 10;
	const float kBlurWidth = 0.01f;

	float2 uv = float2(localPos) / float2(gParams.virtualSize);
	float2 direction = uv - kCenter;
	float4 outputColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for (int sampleIndex = 0; sampleIndex < kNumSamples; ++sampleIndex) {
		float2 texcoord = uv + direction * kBlurWidth * float(sampleIndex);
		// 全画面のサンプリングUVに変換
		float2 sampleUV = (texcoord * float2(gParams.virtualSize) + float2(gParams.offset)) / float2(TextureSize.xy);
		outputColor += colorTexture.Sample(textureSampler, sampleUV);
	}

	outputColor *= rcp(float(kNumSamples));

	outputTexture[pixelPos] = outputColor;
}