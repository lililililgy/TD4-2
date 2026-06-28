#include "ParticleSystem2D.hlsli"

#include "../../ConstantBufferData/ViewProjection.hlsli"

struct Particle {
    float3 position;
    float3 velocity;
    float4 color;
    float startLifetime;
    float remainingLifetime;
    float size;
    float rotation;
    float4 startColor;
    float startSize;
    float3 baseVelocity;
    float randomValue;
    uint simulationSpace;
};

ConstantBuffer<ViewProjection> viewProjection : register(b0);
StructuredBuffer<Particle> particles : register(t0);

// Global camera data (Unused but kept for identical root signature / constants alignment if needed)
cbuffer GlobalCameraData : register(b1) {
    float4x4 billboardMatrix;
    float3 cameraPosition;
    float padding;
}

// Per-system data (Root Constants)
cbuffer PerSystemData : register(b2) {
    float4x4 emitterWorldMatrix;
    uint renderMode;
    uint renderAlignment;
    float speedScale;
    float lengthScale;
    uint instanceOffset;
}

VSOutput main(VSInput input, uint instanceId : SV_InstanceID) {
    VSOutput output;

    uint instanceIndex = instanceId + instanceOffset;
    Particle p = particles[instanceIndex];

    // Get particle center and velocity in world space
    float3 worldCenter;
    if (p.simulationSpace == 1) { // Local
        worldCenter = mul(float4(p.position, 1.0f), emitterWorldMatrix).xyz;
    } else { // World
        worldCenter = p.position;
    }

    // Apply rotation around the Z axis in 2D space
    float s = sin(p.rotation);
    float c = cos(p.rotation);
    
    float2 localPos = input.position.xy;
    float2 rotPos;
    rotPos.x = localPos.x * c - localPos.y * s;
    rotPos.y = localPos.x * s + localPos.y * c;

    // Apply scaling
    float currentSize = p.size;
    rotPos *= currentSize;

    // Output final world position (Z is preserved from the emitter center or kept flat)
    float4 worldPos;
    worldPos.xy = worldCenter.xy + rotPos;
    worldPos.z = worldCenter.z; // Keep the emitter's Z position for sorting purposes
    worldPos.w = 1.0f;

    output.position = mul(worldPos, viewProjection.matVP);
    output.worldPosition = worldPos;
    output.normal = float3(0.0f, 0.0f, -1.0f); // Face towards camera (2D)
    output.uv = input.uv;
    output.instanceId = instanceIndex;
    output.color = p.color;
    
    return output;
}
