#include "../../../ConstantBufferData/Material.hlsli"
#include "../../../ConstantBufferData/ViewProjection.hlsli"

struct CausticsParams {
    float scale;
    float speed;
    float intensity;
    float lightShaftsIntensity;
    float3 lightDir;
    float time;
};

ConstantBuffer<CausticsParams> gParams : register(b0);
ConstantBuffer<Camera> gCamera : register(b1); // 残す

Texture2D<float4> colorTex : register(t0);
Texture2D<float4> worldPosTex : register(t1); // 残す
RWTexture2D<float4> outputTex : register(u0);
SamplerState textureSampler : register(s0);

static const float2 screenSize = float2(1920.0f, 1080.0f);

// 簡易乱数
float2 hash22(float2 p) {
    p = float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)));
    return frac(sin(p) * 43758.5453);
}

// 簡易Voronoiノイズ (コースティクス用)
float voronoi(float2 x) {
    float2 n = floor(x);
    float2 f = frac(x);
    float F1 = 8.0;
    
    for (int j = -1; j <= 1; j++) {
        for (int i = -1; i <= 1; i++) {
            float2 g = float2((float)i, (float)j);
            float2 o = hash22(n + g);
            // 時間経過で揺らす
            o = 0.5 + 0.5 * sin(gParams.time * gParams.speed + 6.2831 * o);
            float2 r = g - f + o;
            float d = dot(r, r);
            if (d < F1) {
                F1 = d;
            }
        }
    }
    return sqrt(F1);
}

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID) {
    if (dispatchId.x >= (uint)screenSize.x || dispatchId.y >= (uint)screenSize.y) return;
    float2 uv = dispatchId.xy / screenSize;
    
    float4 color = colorTex.Sample(textureSampler, uv);
    
    // 2Dスクリーン空間コースティクス
    // UVを少しサイン波でゆがめて、水中の揺らめき感を出す
    float2 distortedUV = uv;
    distortedUV.x += sin(uv.y * 10.0f + gParams.time * 2.0f) * 0.01f;
    distortedUV.y += cos(uv.x * 10.0f + gParams.time * 2.0f) * 0.01f;
    
    float2 cUV = distortedUV * gParams.scale * 15.0f;
    
    // 2レイヤーのVoronoiを重ねてディテールを出す
    float v1 = voronoi(cUV);
    float v2 = voronoi(cUV * 1.5f + float2(gParams.time * 0.2f, -gParams.time * 0.1f));
    
    float caustics = pow(saturate(1.0f - min(v1, v2)), 3.0f) * gParams.intensity;
    
    // 簡易ライトシャフトエミュレーション (斜めから降り注ぐ光の筋)
    // 光源の進行方向（lightDir）に沿ったスクロールノイズを重ねる
    float2 shaftUV = uv;
    // lightDirのXY成分を使って光の差し込む向きにスクロールさせる
    float2 scrollDir = normalize(gParams.lightDir.xy + float2(1e-5f, 1e-5f));
    float shaftNoise = sin((uv.x * 5.0f - uv.y * 3.0f * scrollDir.x) + gParams.time * gParams.speed * 0.5f) * 0.5f + 0.5f;
    shaftNoise += sin((uv.x * 12.0f + uv.y * 8.0f * scrollDir.y) - gParams.time * gParams.speed * 1.1f) * 0.25f;
    
    // 画面上部ほど光の筋が強く、下部ほど減衰する
    float shaftDepthAttenuation = saturate(1.0f - uv.y);
    float shaft = pow(saturate(shaftNoise), 2.0f) * gParams.lightShaftsIntensity * shaftDepthAttenuation;
    
    // 合成 (加算合成)
    float3 finalColor = color.rgb + color.rgb * caustics + float3(0.8f, 0.95f, 1.0f) * shaft;
    
    outputTex[dispatchId.xy] = float4(saturate(finalColor), color.a);
}
