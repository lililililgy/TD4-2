struct PixelateParams {
	float pixelSizeX;
	float pixelSizeY;
	int2 offset;
};
ConstantBuffer<PixelateParams> gPixelateParams : register(b0);

/// Texture
Texture2D<float4> colorTexture : register(t0);
RWTexture2D<float4> outputTexture : register(u0);
SamplerState textureSampler : register(s0);

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
	uint2 pixelPos = dispatchThreadID.xy + gPixelateParams.offset;
	uint width, height;
	colorTexture.GetDimensions(width, height);
	
	float2 pixelSize = float2(gPixelateParams.pixelSizeX, gPixelateParams.pixelSizeY);
	
	// 1.0 未満のピクセルサイズを防ぐためのクランプ (0による除算やスナップ異常の防止)
	pixelSize = max(pixelSize, float2(1.0f, 1.0f));
	
	// ピクセルサイズでスナップ
	float2 snappedPos = floor(float2(pixelPos) / pixelSize) * pixelSize;
	// ブロックの中心を求める
	uint2 samplePos = uint2(snappedPos + pixelSize * 0.5f);
	
	// 範囲外アクセス防止のためにクランプ
	samplePos = clamp(samplePos, uint2(0, 0), uint2(width - 1, height - 1));
	
	// 直接サンプリング
	float4 color = colorTexture[samplePos];
	
	outputTexture[pixelPos] = color;
}
