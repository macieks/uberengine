float4x4 World;
float4x4 ViewProj;

float4x4 BoneTransforms[48];

sampler2D ColorMap;

#if defined(D3D11)
	#define BoneIndicesType uint4
#else
	#define BoneIndicesType float4
#endif

float4x4 GetSkinningTransform(float4 boneWeights, BoneIndicesType boneIndices)
{
	float4x4 skinningTransform;
	
	skinningTransform =  BoneTransforms[(int) boneIndices.x] * boneWeights.x;
	skinningTransform += BoneTransforms[(int) boneIndices.y] * boneWeights.y;
	skinningTransform += BoneTransforms[(int) boneIndices.z] * boneWeights.z;
	skinningTransform += BoneTransforms[(int) boneIndices.w] * boneWeights.w;

	return skinningTransform;
}

void Model_VS(
		in float3 IN_Pos : POSITION,
		in float3 IN_Normal : NORMAL,
		in float2 IN_Tex0 : TEXCOORD0,
#if defined(SKINNED)
		in float4 IN_BoneWeights : BLENDWEIGHT,
		in BoneIndicesType IN_BoneIndices : BLENDINDICES,
#endif
		out float3 OUT_Normal : TEXCOORD0,
		out float2 OUT_Tex0 : TEXCOORD1,
		out float4 OUT_Pos : POSITION)
{
#if defined(SKINNED)
	float4x4 World = GetSkinningTransform(IN_BoneWeights, IN_BoneIndices);
#endif

	float4 worldPos = mul(World, float4(IN_Pos, 1));
	OUT_Pos = mul(ViewProj, worldPos);

	OUT_Normal = mul((float3x3) World, IN_Normal.xyz * 2 - 1);
	//OUT_Normal = IN_Normal.xyz * 2 - 1;

	OUT_Tex0 = IN_Tex0;
}

float4 Model_FS(
	in float3 IN_Normal : TEXCOORD0,
	in float2 IN_Tex0 : TEXCOORD1) : COLOR0
{
	//return float4(1, 1, 1, 1);
	//return float4(normalize(IN_Normal) * 0.5 + 0.5, 1);
	//return tex2D(ColorMap, IN_Tex0);
	//return float4(fmod(IN_Tex0, 1), 1, 1);

	const float3 LightDir = normalize(float3(-0.6, 1, -0.6));

	float3 normal = normalize(IN_Normal);

	float diff = abs(dot(normal, LightDir)) * 0.8;
	float ambient = 0.5;
	float4 tex = tex2D(ColorMap, IN_Tex0);
	float4 color = tex * (diff + ambient);
	
	return color;
}