// GaussianBlur.compute.hlsl

cbuffer BlurParams : register(b0) {
	int horizontal;
};

struct TextureParams {
	float2 texelSize; // (1.0f / width, 1.0f / height)
};

Texture2D<float4> sceneTexture : register(t0);
RWTexture2D<float4> outputTexture : register(u0);
SamplerState textureSampler : register(s0);

static const TextureParams texParams = {
	float2(1.0f / 1920.0f, 1.0f / 1080.0f) // テクセルサイズ
};

static const int MAX_BLUR_RADIUS = 30;

float Gaussian(int x, float sigma) {
	return exp(-((x * x) / (2.0f * sigma * sigma)));
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
	float2 uv = (float2) dispatchThreadId.xy * texParams.texelSize;

	float4 sum = float4(0, 0, 0, 0);
	float totalWeight = 0.0;

	// 常に最大半径の範囲から周囲のテクセルをサンプリング
	for (int i = -MAX_BLUR_RADIUS; i <= MAX_BLUR_RADIUS; ++i) {
		int2 offset = horizontal != 0 ? int2(i, 0) : int2(0, i);
		float2 offsetUV = uv + texParams.texelSize * offset;

		// サンプリング対象ピクセルの高輝度抽出カラーと半径情報を取得
		float4 sampleColor = sceneTexture.SampleLevel(textureSampler, offsetUV, 0.0);
		float sampleRadius = sampleColor.a * 30.0f;

		// サンプリング対象がブルーム対象でない場合は寄与させない
		if (sampleRadius <= 0.01f) {
			continue;
		}

		// 距離がそのサンプリング対象のブラー半径内にある場合のみ、その光を届かせる
		if (abs(i) <= sampleRadius) {
			float sampleSigma = max(1.0f, sampleRadius / 3.0f);
			float weight = Gaussian(i, sampleSigma);
			
			// 色と半径（アルファ）の両方をウェイト付きで加算
			sum += sampleColor * weight;
			totalWeight += weight;
		}
	}

	// 周囲からのブルームの寄与がある場合はそのブラー色とぼけた半径を出力、
	// そうでなければ（黒の領域など）、黒を出力
	if (totalWeight > 0.0) {
		outputTexture[dispatchThreadId.xy] = sum / totalWeight;
	} else {
		outputTexture[dispatchThreadId.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}
