struct VSInput {
	float3 position   : POSITION0;
	float2 uv         : TEXCOORD0;
	uint   materialId : MATERIAL_ID0;
};

struct VSOutput {
	float4 position   : SV_POSITION;
	float2 uv         : TEXCOORD0;
	nointerpolation uint   instanceId : TEXCOORD1;
};

struct PSOutput {
	float4 color         : SV_TARGET0;
	float4 worldPosition : SV_TARGET1;
	float4 normal        : SV_TARGET2;
	float4 flags         : SV_TARGET3;
};
