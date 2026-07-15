// RadialBlurPerObject.cs.hlsl
#include "../../../ConstantBufferData/Material.hlsli"

Texture2D<float4> sceneTexture : register(t0);
Texture2D<float4> flagsTex : register(t1);
RWTexture2D<float4> outputTexture : register(u0);
SamplerState textureSampler : register(s0);

static const float2 TextureSize = float2(1920.0f, 1080.0f);
static const float2 texelSize = float2(1.0f / 1920.0f, 1.0f / 1080.0f);

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
	int2 pixelPos = dispatchThreadId.xy;
	float2 uv = (float2)pixelPos * texelSize;

	// ラジアルブラーのパラメータ
	const float2 kCenter = float2(0.5f, 0.5f);
	const int kNumSamples = 16;
	const float kBlurWidth = 0.015f;

	float2 direction = uv - kCenter;
	float4 sumColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float totalWeight = 0.0f;

	// サンプリングタップを回す
	for (int i = 0; i < kNumSamples; ++i) {
		float2 sampleUV = uv - direction * kBlurWidth * float(i);
		if (sampleUV.x < 0.0f || sampleUV.x > 1.0f || sampleUV.y < 0.0f || sampleUV.y > 1.0f) {
			continue;
		}

		// テクセル座標を直接計算して Load する（線形補間によるフラグ破損を防止）
		int2 samplePixelPos = (int2)(sampleUV * TextureSize);
		float4 sampleFlags = flagsTex[samplePixelPos];
		
		int flagsVal = (int)(sampleFlags.x + 0.5f); // 丸め処理をしてキャスト
		
		if (IsPostEffectEnabled(flagsVal, PostEffectFlags_RadialBlur)) {
			// サンプリング対象がラジアルブラー対象の場合のみ、その色を加算
			float4 sampleColor = sceneTexture[samplePixelPos];
			float weight = 1.0f - (float)i / (float)kNumSamples; // 遠いサンプリングほど重みを下げる
			sumColor += sampleColor * weight;
			totalWeight += weight;
		}
	}

	// 寄与が得られた場合はブラー色を出力、そうでなければ元のシーンカラーをそのまま Load して出力
	if (totalWeight > 0.0f) {
		outputTexture[pixelPos] = sumColor / totalWeight;
	} else {
		outputTexture[pixelPos] = sceneTexture[pixelPos];
	}
}
