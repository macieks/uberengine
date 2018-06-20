#include "GraphicsExt/gxCommonConstants.h"

glConstantHandle gxCommonConstants::World;
glConstantHandle gxCommonConstants::View;
glConstantHandle gxCommonConstants::Proj;
glConstantHandle gxCommonConstants::ViewProj;
glConstantHandle gxCommonConstants::WorldView;
glConstantHandle gxCommonConstants::WorldViewProj;
glConstantHandle gxCommonConstants::BoneTransforms;

glConstantHandle gxCommonConstants::RightVec;
glConstantHandle gxCommonConstants::UpVec;
glConstantHandle gxCommonConstants::FrontVec;

glConstantHandle gxCommonConstants::ColorMap;
glConstantHandle gxCommonConstants::ColorMap2;
glConstantHandle gxCommonConstants::ColorMapSize;
glConstantHandle gxCommonConstants::DepthMap;
glConstantHandle gxCommonConstants::NormalMap;
glConstantHandle gxCommonConstants::SpecularMap;
glConstantHandle gxCommonConstants::DetailMap;
glConstantHandle gxCommonConstants::EnvMap;
glConstantHandle gxCommonConstants::BlurMap;

glConstantHandle gxCommonConstants::LightDir;

glConstantHandle gxCommonConstants::ShadowMap;
glConstantHandle gxCommonConstants::ShadowMapSize;
glConstantHandle gxCommonConstants::ShadowMask;
glConstantHandle gxCommonConstants::LightMatrix;

glConstantHandle gxCommonConstants::RenderTargetSize;
glConstantHandle gxCommonConstants::PixelOffset;
glConstantHandle gxCommonConstants::Color;
glConstantHandle gxCommonConstants::Time;

ueBool gxCommonConstants::Startup()
{
	UE_ASSERT_FUNC( World.Init("World", glConstantType_Float4x4) );
	UE_ASSERT_FUNC( View.Init("View", glConstantType_Float4x4) );
	UE_ASSERT_FUNC( Proj.Init("Proj", glConstantType_Float4x4) );
	UE_ASSERT_FUNC( ViewProj.Init("ViewProj", glConstantType_Float4x4) );
	UE_ASSERT_FUNC( WorldView.Init("WorldView", glConstantType_Float4x4) );
	UE_ASSERT_FUNC( WorldViewProj.Init("WorldViewProj", glConstantType_Float4x4) );
	UE_ASSERT_FUNC( BoneTransforms.Init("BoneTransforms", glConstantType_Float4x4, 48) );

	UE_ASSERT_FUNC( RightVec.Init("RightVec", glConstantType_Float3) );
	UE_ASSERT_FUNC( UpVec.Init("UpVec", glConstantType_Float3) );
	UE_ASSERT_FUNC( FrontVec.Init("FrontVec", glConstantType_Float3) );

	UE_ASSERT_FUNC( ColorMap.Init("ColorMap", glConstantType_Sampler2D) );
	UE_ASSERT_FUNC( ColorMap2.Init("ColorMap2", glConstantType_Sampler2D) );
	UE_ASSERT_FUNC( ColorMapSize.Init("ColorMapSize", glConstantType_Float4) );

	UE_ASSERT_FUNC( DepthMap.Init("DepthMap", glConstantType_Sampler2D) );
	UE_ASSERT_FUNC( NormalMap.Init("NormalMap", glConstantType_Sampler2D) );
	UE_ASSERT_FUNC( SpecularMap.Init("SpecularMap", glConstantType_Sampler2D) );
	UE_ASSERT_FUNC( DetailMap.Init("DetailMap", glConstantType_Sampler2D) );
	UE_ASSERT_FUNC( EnvMap.Init("EnvMap", glConstantType_SamplerCube) );
	UE_ASSERT_FUNC( BlurMap.Init("BlurMap", glConstantType_Sampler2D) );

	UE_ASSERT_FUNC( LightDir.Init("LightDir", glConstantType_Float3) );

	UE_ASSERT_FUNC( ShadowMap.Init("ShadowMap", glConstantType_Sampler2D) );
	UE_ASSERT_FUNC( ShadowMapSize.Init("ShadowMapSize", glConstantType_Float4) );
	UE_ASSERT_FUNC( ShadowMask.Init("ShadowMask", glConstantType_Sampler2D) );
	UE_ASSERT_FUNC( LightMatrix.Init("LightMatrix", glConstantType_Float4x4) );

	UE_ASSERT_FUNC( RenderTargetSize.Init("RenderTargetSize", glConstantType_Float4) );
	UE_ASSERT_FUNC( PixelOffset.Init("PixelOffset", glConstantType_Float2) );
	UE_ASSERT_FUNC( Color.Init("Color", glConstantType_Float4) );

	UE_ASSERT_FUNC( Time.Init("Time", glConstantType_Float) );

	return UE_TRUE;
}