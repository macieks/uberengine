#include "Shading.fx"

float3 LightPos;
float3 LightColor;
float2 LightNearFarRadius;

float CalculateAttenuation(float3 pos)
{
#if LIGHT_ATTENUATION == LIGHT_ATTENUATION_NONE
	return 1;
#elif LIGHT_ATTENUATION == LIGHT_ATTENUATION_LINEAR
	return Attenuation_Linear(LightPos, LightNearFarRadius, pos);
#elif LIGHT_ATTENUATION == LIGHT_ATTENUATION_QUADRATIC
	return Attenuation_Quadratic(LightPos, LightNearFarRadius, pos);
#endif
}

struct VS_INPUT
{
	float3 Pos : POSITION0;
};

struct VS_OUTPUT
{
	float4 PosCS : POSITION0;
	float2 TexTS : TEXCOORD0;
	float3 PosVS : TEXCOORD1;
};

struct FS_OUTPUT
{
	float4 Lighting : COLOR0;
};

void DeferredShading_LPP_Light_VS(in VS_INPUT IN, out VS_OUTPUT OUT)
{
	float4 pos4 = float4(IN.Pos, 1);

	OUT.PosCS = mul(WorldViewProj, pos4);
	OUT.PosVS = mul(WorldView, pos4).xyz;
	OUT.TexTS = OUT.PosCS.xy / OUT.PosCS.w * 0.5 + 0.5;
}

void DeferredShading_LPP_Light_FS(in VS_OUTPUT IN, out FS_OUTPUT OUT)
{
	// Reconstruct normal in VS (and get specular power)

	float4 normalVS_specularPower = tex2D(SceneNormal, IN.TexTS);
	normalVS_specularPower.rgb = normalVS_specularPower.rgb * 2 - 1;

	// Reconstruct position in VS

	float3 frustumVec = IN.PosVS.xyz * (GetFarPlane() / IN.PosVS.z);
	float3 posVS = tex2D(SceneDepth, IN.TexTS).x * frustumVec;

	// Calculate diffuse lighting term

	float att = CalculateAttenuation(posVS);
	
	float3 lightVecVS = normalize(LightPos - posVS);
	float NL = saturate(dot(normalVS_specularPower.rgb, lightVecVS));

	OUT.Lighting.rgb = NL * att * LightColor;

	// Calculate specular lighting term

	float3 reflVecVS = reflect(lightVecVS, normalVS_specularPower.rgb);
	float RL = saturate(dot(viewVecVS, reflVecVS));
	
	OUT.Lighting.a = CalcLuminance(pow(RL, normalVS_specularPower.a) * att * LightColor);
}