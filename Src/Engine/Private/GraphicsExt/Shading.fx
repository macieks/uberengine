// Transforms

float4x4 World;
float4x4 View;
float4x4 ViewProj;

float4x4 BoneTransforms[MAX_BONES];

// Miscellaneous

float AlphaTestMinThreshold;

float2 NearFarPlanes;
float GetNearPlane() { return NearFarPlanes.x; }
float GetFarPlane() { return NearFarPlanes.y; }

// Samplers

sampler2D ColorMap;
sampler2D NormalMap;
sampler2D HeightMap;
sampler2D SpecularMap;

sampler2D SceneColor;
sampler2D SceneDepth;
sampler2D SceneNormal;
sampler2D SceneLighting;

// Lighting models
// ---------------

void Light_Lambert(in float3 normal, in float3 lightDir, out float intensity)
{
	intensity = max(0, dot(normal, lightDir));
}

void Light_Phong(in float3 normal, in float3 lightDir, in float3 viewDir, in float specularPower, out float intensity)
{
	intensity = pow(max(0, dot(viewDir, reflect(lightDir, normal))), specularPower);
}

void Light_OrenNayar()
{
}

void Light_Ashikhmin()
{
}

// Attentuation types
// ------------------

#define LIGHT_ATTENUATION_NONE 0
#define LIGHT_ATTENUATION_LINEAR 1
#define LIGHT_ATTENUATION_QUADRATIC 2

float Attenuation_Linear(float3 lightPos, float2 lightNearFarRadius, float3 surfacePos)
{
	float dist = distance(lightPos, surfacePos);
	return 1 - clamp((dist - lightNearFarRadius.x) / (lightNearFarRadius.y - lightNearFarRadius.x), 0, 1);
}

float Attenuation_Quadratic(float3 lightPos, float lightNearRadius, float lightFarRadius, float3 surfacePos)
{
	float dist = distance(lightPos, surfacePos);
	float att = clamp((dist - lightNearFarRadius.x) / (lightNearFarRadius.y - lightNearFarRadius.x), 0, 1);
	return 1 - att * att;
}

// Misc. utils
// -----------

float CalcLuminance(float3 rgb)
{
	return dot(rgb, float3(0.30, 0.59, 0.11));
}

float4x4 GetSkinningTransform(float4 boneWeights, float4 boneIndices)
{
	float4x4 skinningTransform;
	
	skinningTransform =  BoneTransforms[(int) boneIndices.x] * boneWeights.x;
	skinningTransform += BoneTransforms[(int) boneIndices.y] * boneWeights.y;
	skinningTransform += BoneTransforms[(int) boneIndices.z] * boneWeights.z;
	skinningTransform += BoneTransforms[(int) boneIndices.w] * boneWeights.w;

	return skinningTransform;
}