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
	static const float PI = 3.14159265f;

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
			
			// 1Dガウス分布の積分値（カーネルウェイト総和）で事前正規化
			float kernelSum = sqrt(2.0f * PI) * sampleSigma;
			float weight = Gaussian(i, sampleSigma) / kernelSum;
			
			// 色をウェイト付きで加算（これで距離による輝度減衰が正しく行われる）
			sum.rgb += sampleColor.rgb * weight;
			
			// 半径（アルファ）は最大の伝播度合いを保つため、ガウス減衰させた値の最大値を取る
			sum.a = max(sum.a, sampleColor.a * Gaussian(i, sampleSigma));
			totalWeight += weight;
		}
	}

	if (totalWeight > 0.0) {
		// すでに事前正規化されているため、そのまま出力（アルファはクランプ）
		outputTexture[dispatchThreadId.xy] = float4(sum.rgb, saturate(sum.a));
	} else {
		outputTexture[dispatchThreadId.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}
