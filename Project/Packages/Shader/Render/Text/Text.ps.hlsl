#include "Text.hlsli"

#include "../../ConstantBufferData/Material.hlsli"

StructuredBuffer<Material> materials : register(t0);
Texture2D<float4> textures[] : register(t1);
SamplerState textureSampler : register(s0);

PSOutput main(VSOutput input) {
	PSOutput output;
	Material material = materials[input.instanceId];
	
	/// TextureSampling
	float3x3 matUV = MatUVTransformToMatrix(material.uvTransform);
	float2 uv = mul(float3(input.uv, 1), matUV).xy;
	float4 baseTexColor = textures[material.baseTextureId].Sample(textureSampler, uv);

	// フォントアトラスのR/Gチャンネルから本体アルファとフチアルファを取り出す
	float4 outputColor;
	if (IsPostEffectEnabled((int)material.postEffectFlags, PostEffectFlags_Shadow)) {
		// 影パスの場合：文字全体（本体＋フチ）のアルファを抽出
		float glyphAlpha = max(baseTexColor.r, baseTexColor.g);
		outputColor = float4(material.baseColor.rgb, glyphAlpha * material.baseColor.a);
	} else {
		// 通常パスの場合：本体色とフチ色を動的合成
		float bodyAlpha = baseTexColor.r;
		float outlineAlpha = baseTexColor.g;

		float4 outline = material.outlineColor * outlineAlpha;
		float4 body = material.baseColor * bodyAlpha;

		// フチの上に本体を重ねるブレンド
		outputColor = outline * (1.0 - bodyAlpha) + body;
	}

	if (outputColor.a < 0.01) {
		discard;
	}
	
	output.color = outputColor;
	output.worldPosition = input.position;
	output.normal = float4(0, 0, 1, 1);
	uint packedIntensityRadius = f32tof16(material.bloomIntensity) | (f32tof16(material.bloomRadius) << 16);
	output.flags = float4(material.postEffectFlags, material.entityId, asfloat(packedIntensityRadius), material.bloomThreshold);
	
	return output;
}
