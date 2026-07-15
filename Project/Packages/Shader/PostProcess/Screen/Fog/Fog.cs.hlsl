#include "../../../ConstantBufferData/Material.hlsli"
#include "../../../ConstantBufferData/ViewProjection.hlsli"

struct FogParams {
    float3 fogColor;      // フォグ色
    float fogStart;       // フォグ開始距離（オフセットとして使用）
    float fogEnd;         // フォグ到達距離（濃さ）
    int2 offset;
    int2 virtualSize;
};

/// Buffers
ConstantBuffer<FogParams>   gFogParams               : register(b0);
ConstantBuffer<Camera>      gCamera                  : register(b1);

Texture2D<float4>           gColorTexture            : register(t0);
Texture2D<float4>           gWorldPositionTexture    : register(t1);
RWTexture2D<float4>         gOutputTexture           : register(u0);
SamplerState                gTextureSampler          : register(s0);

static const float2 screenSize = float2(1920.0f, 1080.0f);

/// 軽量ノイズ
float HashNoise(float2 p)
{
    return frac(sin(dot(p, float2(12.9898, 78.233))) * 43758.5453);
}

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 localPos = DTid.xy;
    uint2 pixelPos = localPos + gFogParams.offset;
    if (pixelPos.x >= (uint)screenSize.x || pixelPos.y >= (uint)screenSize.y) return;

    float2 localUV = float2(localPos) / float2(gFogParams.virtualSize);
    float2 uv = (localUV * float2(gFogParams.virtualSize) + float2(gFogParams.offset)) / screenSize;

    float4 color    = gColorTexture.Sample(gTextureSampler, uv);
    float3 worldPos = gWorldPositionTexture.Sample(gTextureSampler, uv).xyz;

    //========================
    // 距離フォグ（指数）
    //========================
    float dist = length(worldPos - gCamera.position.xyz);

    // fogStart までは霧なし
    float d = max(dist - gFogParams.fogStart, 0.0f);

    // 距離に強い指数フォグ
    float fogFactor = 1.0f - exp(-d / max(gFogParams.fogEnd, 0.0001f));

    //========================
    // ノイズでわずかなムラ（控えめ）
    //========================
    float noise = HashNoise(worldPos.xz * 0.03);
    fogFactor *= lerp(0.95f, 1.05f, noise);

    fogFactor = saturate(fogFactor);

    //========================
    // 最終合成
    //========================
    float3 finalColor = lerp(color.rgb, gFogParams.fogColor, fogFactor);

    gOutputTexture[pixelPos] = float4(finalColor, color.a);
}
