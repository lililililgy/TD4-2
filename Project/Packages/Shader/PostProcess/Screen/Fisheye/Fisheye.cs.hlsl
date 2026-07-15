static const uint2 TextureSize = uint2(1920, 1080);

struct FisheyeParams {
	float strength;
	float scale;
	int2 offset;
	int2 virtualSize;
};
ConstantBuffer<FisheyeParams> gFisheyeParams : register(b0);

/// Texture
Texture2D<float4> colorTexture : register(t0);
RWTexture2D<float4> outputTexture : register(u0);
SamplerState textureSampler : register(s0);

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
	uint2 localPos = dispatchThreadID.xy;
	uint2 pixelPos = localPos + gFisheyeParams.offset;
	
	// ローカルUV座標 [0, 1]
	float2 uv = float2(localPos) / float2(gFisheyeParams.virtualSize);
	
	// 中心座標
	float2 center = float2(0.5f, 0.5f);
	
	// アスペクト比補正 (仮想画面のアスペクト比)
	float aspect = (float)gFisheyeParams.virtualSize.x / (float)gFisheyeParams.virtualSize.y;
	
	// 中心からのオフセット
	float2 offset = uv - center;
	float2 offsetAspect = offset * float2(aspect, 1.0f);
	
	// 中心からの距離の2乗
	float r2 = dot(offsetAspect, offsetAspect);
	
	// 魚眼歪み係数 (k)。
	float k = gFisheyeParams.strength; 
	
	// 魚眼歪みの適用
	float scale = gFisheyeParams.scale;
	float2 distortedOffset = offset * (1.0f + k * r2) * scale;
	float2 distortedLocalUV = center + distortedOffset;
	
	float4 outputColor = float4(0.0f, 0.0f, 0.0f, 1.0f); // 範囲外は黒
	
	// 歪み後のUVが[0, 1]の範囲内ならサンプリングする
	if (distortedLocalUV.x >= 0.0f && distortedLocalUV.x <= 1.0f &&
		distortedLocalUV.y >= 0.0f && distortedLocalUV.y <= 1.0f) {
		// 全画面UVに変換してサンプリング
		float2 sampleUV = (distortedLocalUV * float2(gFisheyeParams.virtualSize) + float2(gFisheyeParams.offset)) / float2(TextureSize.xy);
		outputColor = colorTexture.Sample(textureSampler, sampleUV);
	}
	
	outputTexture[pixelPos] = outputColor;
}
