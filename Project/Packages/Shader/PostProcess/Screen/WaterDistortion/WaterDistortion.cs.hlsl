#include "../../../ConstantBufferData/Material.hlsli"

struct DistortionParams {
    float strength;
    float speed;
    float frequency;
    float time;
    int2 offset;
    int2 virtualSize;
};

ConstantBuffer<DistortionParams> gParams : register(b0);

Texture2D<float4> colorTex : register(t0);
RWTexture2D<float4> outputTex : register(u0);
SamplerState textureSampler : register(s0);

static const float2 screenSize = float2(1920.0f, 1080.0f);

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID) {
    uint2 localPos = dispatchId.xy;
    uint2 pixelPos = localPos + gParams.offset;
    if (pixelPos.x >= (uint)screenSize.x || pixelPos.y >= (uint)screenSize.y) return;
    
    float2 uv = float2(localPos) / float2(gParams.virtualSize);
    
    // サイン波で歪ませる
    float2 offset;
    offset.x = sin(uv.y * gParams.frequency + gParams.time * gParams.speed) * gParams.strength;
    offset.y = cos(uv.x * gParams.frequency + gParams.time * gParams.speed) * gParams.strength;
    
    float2 distortedLocalUV = uv + offset;
    distortedLocalUV = saturate(distortedLocalUV); // 画面外に出ないようにクランプ
    
    float2 sampleUV = (distortedLocalUV * float2(gParams.virtualSize) + float2(gParams.offset)) / screenSize;
    float4 color = colorTex.Sample(textureSampler, sampleUV);
    outputTex[pixelPos] = color;
}
