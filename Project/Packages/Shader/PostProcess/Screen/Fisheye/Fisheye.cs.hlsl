static const uint2 TextureSize = uint2(1920, 1080);

struct FisheyeParams {
	float strength;
	float scale;
	int2 offset;
};
ConstantBuffer<FisheyeParams> gFisheyeParams : register(b0);

/// Texture
Texture2D<float4> colorTexture : register(t0);
RWTexture2D<float4> outputTexture : register(u0);
SamplerState textureSampler : register(s0);

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
	uint2 pixelPos = dispatchThreadID.xy + gFisheyeParams.offset;
	
	// スレッド座標を[0, 1]のUV座標に変換
	float2 uv = float2(pixelPos) / float2(TextureSize.xy);
	
	// 中心座標
	float2 center = float2(0.5f, 0.5f);
	
	// アスペクト比補正 (横長画面での歪みを真円にするため)
	float aspect = (float)TextureSize.x / (float)TextureSize.y;
	
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
	float2 distortedUV = center + distortedOffset;
	
	float4 outputColor = float4(0.0f, 0.0f, 0.0f, 1.0f); // 範囲外は黒
	
	// 歪み後のUVが[0, 1]の範囲内ならサンプリングする
	if (distortedUV.x >= 0.0f && distortedUV.x <= 1.0f &&
		distortedUV.y >= 0.0f && distortedUV.y <= 1.0f) {
		outputColor = colorTexture.Sample(textureSampler, distortedUV);
	}
	
	outputTexture[pixelPos] = outputColor;
}
