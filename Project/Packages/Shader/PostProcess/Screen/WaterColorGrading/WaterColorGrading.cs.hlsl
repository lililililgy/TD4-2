#include "../../../ConstantBufferData/Material.hlsli"
#include "../../../ConstantBufferData/ViewProjection.hlsli"

struct ColorGradingParams {
    float3 absorption;
    float contrast;
    float saturation;
    float3 colorFilter;
};

ConstantBuffer<ColorGradingParams> gParams : register(b0);
ConstantBuffer<Camera> gCamera : register(b1); // 残す

Texture2D<float4> colorTex : register(t0);
Texture2D<float4> worldPosTex : register(t1); // 残す
RWTexture2D<float4> outputTex : register(u0);
SamplerState textureSampler : register(s0);

static const float2 screenSize = float2(1920.0f, 1080.0f);

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID) {
    if (dispatchId.x >= (uint)screenSize.x || dispatchId.y >= (uint)screenSize.y) return;
    float2 uv = dispatchId.xy / screenSize;
    
    float4 color = colorTex.Sample(textureSampler, uv);
    
    // 2D水深に応じた赤色光の吸収をシミュレート
    // 下部へ行くほど（uv.yが1に近いほど）R（赤）成分がより強く吸収される
    float3 attenuation = exp(-gParams.absorption * uv.y * 5.0f);
    float3 gradedColor = color.rgb * attenuation;
    
    // カラーフィルター
    gradedColor *= gParams.colorFilter;
    
    // コントラスト調整
    gradedColor = (gradedColor - 0.5f) * gParams.contrast + 0.5f;
    
    // 彩度調整
    float luma = dot(gradedColor, float3(0.299f, 0.587f, 0.114f));
    gradedColor = lerp(float3(luma, luma, luma), gradedColor, gParams.saturation);
    
    outputTex[dispatchId.xy] = float4(saturate(gradedColor), color.a);
}
