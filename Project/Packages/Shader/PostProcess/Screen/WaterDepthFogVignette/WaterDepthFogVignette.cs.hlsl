#include "../../../ConstantBufferData/Material.hlsli"
#include "../../../ConstantBufferData/ViewProjection.hlsli"

struct DepthFogVignetteParams {
    float3 fogColor;
    float fogDensity;
    float fogWaterSurfaceY;
    float vignetteStrength;
    int2 offset;
};

ConstantBuffer<DepthFogVignetteParams> gParams : register(b0);
ConstantBuffer<Camera> gCamera : register(b1); // C++バインド維持のため残す

Texture2D<float4> colorTex : register(t0);
Texture2D<float4> worldPosTex : register(t1); // C++バインド維持のため残す
RWTexture2D<float4> outputTex : register(u0);
SamplerState textureSampler : register(s0);

static const float2 screenSize = float2(1920.0f, 1080.0f);

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID) {
    uint2 pixelPos = dispatchId.xy + gParams.offset;
    if (pixelPos.x >= (uint)screenSize.x || pixelPos.y >= (uint)screenSize.y) return;
    float2 uv = pixelPos / screenSize;
    
    float4 color = colorTex.Sample(textureSampler, uv);
    
    // 2D用の深度（水深）フォグエミュレーション
    // 画面の下部（uv.yが1に近い）ほど水深が深くフォグが濃くなるようにする
    float depthFactor = saturate(uv.y * gParams.fogDensity * 10.0f);
    
    float3 finalColor = lerp(color.rgb, gParams.fogColor, depthFactor);
    
    // ビネット効果 (画面端を暗くする)
    float2 d = abs(uv - 0.5f) * gParams.vignetteStrength;
    float vignette = saturate(1.0f - dot(d, d));
    finalColor *= vignette;
    
    outputTex[pixelPos] = float4(finalColor, color.a);
}
