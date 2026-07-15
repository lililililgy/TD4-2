#include "../../../ConstantBufferData/Material.hlsli"

struct DistortionParams {
    float strength;
    float speed;
    float frequency;
    float time;
    int2 offset;
    int2 padding;
};

ConstantBuffer<DistortionParams> gParams : register(b0);

Texture2D<float4> colorTex : register(t0);
RWTexture2D<float4> outputTex : register(u0);
SamplerState textureSampler : register(s0);

static const float2 screenSize = float2(1920.0f, 1080.0f);

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID) {
    uint2 pixelPos = dispatchId.xy + gParams.offset;
    if (pixelPos.x >= (uint)screenSize.x || pixelPos.y >= (uint)screenSize.y) return;
    float2 uv = pixelPos / screenSize;
    
    // サイン波で歪ませる
    float2 offset;
    offset.x = sin(uv.y * gParams.frequency + gParams.time * gParams.speed) * gParams.strength;
    offset.y = cos(uv.x * gParams.frequency + gParams.time * gParams.speed) * gParams.strength;
    
    float2 distortedUV = uv + offset;
    distortedUV = saturate(distortedUV); // 画面外に出ないようにクランプ
    
    float4 color = colorTex.Sample(textureSampler, distortedUV);
    outputTex[pixelPos] = color;
}
