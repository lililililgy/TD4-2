#include "../../../ConstantBufferData/ViewProjection.hlsli"
#include "../../../ConstantBufferData/Material.hlsli"

struct DirectionalLight {
    float4 position;
    float4 color;
    float3 direction;
    float intensity;
};

struct PointLight {
    float4 position;
    float4 color;
    float intensity;
    float radius;
    float2 padding;
};

struct SpotLight {
    float4 position;
    float4 color;
    float3 direction;
    float intensity;
    float radius;
    float innerAngle;
    float outerAngle;
    float padding;
};

struct LightCount {
    uint directionalLightCount;
    uint pointLightCount;
    uint spotLightCount;
    uint padding;
};

ConstantBuffer<LightCount> lightCounts : register(b0);
ConstantBuffer<Camera> camera : register(b1);

StructuredBuffer<DirectionalLight> directionalLights : register(t0);
StructuredBuffer<PointLight> pointLights : register(t1);
StructuredBuffer<SpotLight> spotLights : register(t2);

Texture2D<float4> colorTex : register(t3);
Texture2D<float4> positionTex : register(t4);
Texture2D<float4> normalTex : register(t5);
Texture2D<float4> flagsTex : register(t6);
TextureCube<float4> environmentTexture : register(t7);

RWTexture2D<float4> outputTex : register(u0);
SamplerState textureSampler : register(s0);

static const float2 kTextureSize = float2(1920.0f, 1080.0f);

//////////////////////////////////////////////////////////////
// Lambert Diffuse
//////////////////////////////////////////////////////////////
float3 LambertDiffuse(float3 albedo, float3 N, float3 L)
{
    float NdotL = saturate(dot(N, L));
    return albedo * NdotL;
}

//////////////////////////////////////////////////////////////
// Half Lambert Diffuse
//////////////////////////////////////////////////////////////
float3 HalfLambertDiffuse(float3 albedo, float3 N, float3 L)
{
    float halfLambert = dot(N, L) * 0.5 + 0.5;
    return albedo * (halfLambert * halfLambert);
}

//////////////////////////////////////////////////////////////
// Blinn-Phong Specular
//////////////////////////////////////////////////////////////
float3 BlinnPhongSpecular(float3 N, float3 L, float3 V)
{
    float3 H = normalize(L + V);

    float specPower = 32.0f; // 適宜調整
    float spec = pow(saturate(dot(N, H)), specPower);

    return spec.xxx;
}

//////////////////////////////////////////////////////////////
// Fresnel (Schlick approximation)
//////////////////////////////////////////////////////////////
float FresnelSchlick(float3 N, float3 V)
{
    float f = 1.0f - saturate(dot(N, V));
    return pow(f, 5.0f);
}

//////////////////////////////////////////////////////////////
// Environment Reflection
//////////////////////////////////////////////////////////////
float3 EnvironmentReflection(float3 position, float3 normal)
{
    float3 V = normalize(camera.position.xyz - position);

    float3 reflectVec = reflect(-V, normal);

    float3 env = environmentTexture.Sample(textureSampler, reflectVec).rgb;

    float fresnel = FresnelSchlick(normal, V);

    return env * fresnel;
}

//////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float2 texCoord = float2(DTid.xy + 0.5f) / kTextureSize;

    float4 color = colorTex.Sample(textureSampler, texCoord);
    float4 position = positionTex.Sample(textureSampler, texCoord);
    float4 normal = normalTex.Sample(textureSampler, texCoord);
    float4 flags = flagsTex.Sample(textureSampler, texCoord);

    float3 albedo = color.rgb;
    float3 worldPos = position.xyz;
    float3 N = normalize(normal.xyz);

    //////////////////////////////////////////////////////////
    // Lighting無効
    //////////////////////////////////////////////////////////
    if (!IsPostEffectEnabled((int)flags.x, PostEffectFlags_Lighting))
    {
        float3 outputColor = albedo;

        if (IsPostEffectEnabled((int)flags.x, PostEffectFlags_EnvironmentReflection))
        {
            outputColor += EnvironmentReflection(worldPos, N);
        }

        outputTex[DTid.xy] = float4(outputColor, 1.0f);
        return;
    }

    //////////////////////////////////////////////////////////
    // Lighting
    //////////////////////////////////////////////////////////
    float3 lighting = 0.0f;
    float3 V = normalize(camera.position.xyz - worldPos);

    // Directional Lights
    for (uint i = 0; i < lightCounts.directionalLightCount; ++i)
    {
        float3 L = normalize(-directionalLights[i].direction);
        float3 diffuse = HalfLambertDiffuse(albedo, N, L);
        lighting += diffuse * directionalLights[i].color.rgb * directionalLights[i].intensity;
    }

    // Point Lights
    for (uint j = 0; j < lightCounts.pointLightCount; ++j)
    {
        float3 L = pointLights[j].position.xyz - worldPos;
        float dist = length(L);
        if (dist < pointLights[j].radius)
        {
            L = normalize(L);
            float attenuation = pow(saturate(1.0f - dist / pointLights[j].radius), 2.0f);
            float3 diffuse = HalfLambertDiffuse(albedo, N, L);
            lighting += diffuse * pointLights[j].color.rgb * pointLights[j].intensity * attenuation;
        }
    }

    // Spot Lights
    for (uint k = 0; k < lightCounts.spotLightCount; ++k)
    {
        float3 L = spotLights[k].position.xyz - worldPos;
        float dist = length(L);
        if (dist < spotLights[k].radius)
        {
            L = normalize(L);
            float attenuation = pow(saturate(1.0f - dist / spotLights[k].radius), 2.0f);
            
            float cosAngle = dot(normalize(spotLights[k].direction), -L);
            float spotAttenuation = saturate((cosAngle - spotLights[k].innerAngle) / (spotLights[k].outerAngle - spotLights[k].innerAngle));
            
            float3 diffuse = HalfLambertDiffuse(albedo, N, L);
            lighting += diffuse * spotLights[k].color.rgb * spotLights[k].intensity * attenuation * spotAttenuation;
        }
    }

    //////////////////////////////////////////////////////////
    // Environment Reflection
    //////////////////////////////////////////////////////////
    if (IsPostEffectEnabled((int)flags.x, PostEffectFlags_EnvironmentReflection))
    {
        lighting += EnvironmentReflection(worldPos, N);
    }

    outputTex[DTid.xy] = float4(lighting, 1.0f);
}
