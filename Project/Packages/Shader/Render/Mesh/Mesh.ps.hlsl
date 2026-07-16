#include "Mesh.hlsli"

#include "../../ConstantBufferData/Material.hlsli"

struct TextureId {
	uint id;
};

StructuredBuffer<Material> materials : register(t0);
StructuredBuffer<TextureId> textureIds : register(t1);
Texture2D<float4> textures[] : register(t2);
SamplerState textureSampler : register(s0);

PSOutput main(VSOutput input) {
	PSOutput output;

	Material material = materials[input.instanceId];
	float2 uv = mul(float3(input.uv, 1), MatUVTransformToMatrix(material.uvTransform)).xy;
	float4 textureColor = textures[textureIds[input.instanceId].id].Sample(textureSampler, uv);
	
	output.color = textureColor * material.baseColor;
	output.worldPosition = input.worldPosition;
	output.normal = float4(input.normal, 1.0f);
	uint packedIntensityRadius = f32tof16(material.bloomIntensity) | (f32tof16(material.bloomRadius) << 16);
	output.flags = float4(material.postEffectFlags, material.entityId, asfloat(packedIntensityRadius), material.bloomThreshold);

	if (output.color.a == 0.0f) { ///< alpha == 0.0f ? pixel discard
		discard;
	}
	
	
	return output;
}