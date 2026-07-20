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

// Global camera data
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
    uint flipMode;
    uint textureSheetEnabled;
    uint tilesX;
    uint tilesY;
    float fps;
    uint pad;
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
    float angle = p.rotation;
    if (renderAlignment == 1) { // RenderAlignment::Velocity
        float3 worldVelocity;
        if (p.simulationSpace == 1) { // Local
            worldVelocity = mul(float4(p.velocity, 0.0f), emitterWorldMatrix).xyz;
        } else { // World
            worldVelocity = p.velocity;
        }
        if (length(worldVelocity.xy) > 0.0001f) {
            angle += atan2(worldVelocity.y, worldVelocity.x) - 1.57079632f;
        }
    }

    float s = sin(angle);
    float c = cos(angle);
    
    float2 localPos = input.position.xy;
    float2 rotPos;
    rotPos.x = localPos.x * c - localPos.y * s;
    rotPos.y = localPos.x * s + localPos.y * c;

    // Apply scaling
    float currentSize = p.size;
    rotPos *= currentSize;

    // Output final world position
    float4 worldPos;
    worldPos.xy = worldCenter.xy + rotPos;
    worldPos.z = worldCenter.z;
    worldPos.w = 1.0f;

    output.position = mul(worldPos, viewProjection.matVP);
    output.worldPosition = worldPos;
    output.normal = float3(0.0f, 0.0f, -1.0f);
    
    // --- UV Calculation ---
    float2 uv = input.uv;
    
    // Texture Sheet Animation
    if (textureSheetEnabled != 0 && tilesX > 0 && tilesY > 0) {
        float elapsed = max(p.startLifetime - p.remainingLifetime, 0.0f);
        uint totalFrames = tilesX * tilesY;
        
        uint frameIndex = (uint)(elapsed * fps);
        frameIndex = frameIndex % totalFrames;
        
        uint col = frameIndex % tilesX;
        uint row = frameIndex / tilesX;
        
        float tileW = 1.0f / (float)tilesX;
        float tileH = 1.0f / (float)tilesY;
        
        uv.x = (float)col * tileW + uv.x * tileW;
        uv.y = (float)row * tileH + uv.y * tileH;
    }
    
    // Random Texture Flip
    // flipMode: 0=None, 1=X, 2=Y, 3=Both
    // Use randomValue to decide per-particle (50% chance)
    if (flipMode != 0) {
        // Use fractional parts of randomValue to get two independent random bits
        float rv = p.randomValue;
        bool flipX = (flipMode == 1 || flipMode == 3) && (frac(rv * 13.37f) > 0.5f);
        bool flipY = (flipMode == 2 || flipMode == 3) && (frac(rv * 7.91f) > 0.5f);
        
        if (textureSheetEnabled != 0 && tilesX > 0 && tilesY > 0) {
            // Flip within the current tile
            float elapsed = max(p.startLifetime - p.remainingLifetime, 0.0f);
            uint totalFrames = tilesX * tilesY;
            uint frameIndex = (uint)(elapsed * fps) % totalFrames;
            uint col = frameIndex % tilesX;
            uint row = frameIndex / tilesX;
            float tileW = 1.0f / (float)tilesX;
            float tileH = 1.0f / (float)tilesY;
            float tileStartX = (float)col * tileW;
            float tileStartY = (float)row * tileH;
            
            if (flipX) uv.x = tileStartX + tileW - (uv.x - tileStartX);
            if (flipY) uv.y = tileStartY + tileH - (uv.y - tileStartY);
        } else {
            if (flipX) uv.x = 1.0f - uv.x;
            if (flipY) uv.y = 1.0f - uv.y;
        }
    }
    
    output.uv = uv;
    output.instanceId = instanceIndex;
    output.color = p.color;
    
    return output;
}
