#ifndef UE_LIGHTMAPPER_H
#define UE_LIGHTMAPPER_H

#include "Math/ueMath.h"

struct ueToolLight
{
	enum Type
	{
		Type_Dir = 0,
		Type_Spot,
		Type_Omni,

		Type_MAX
	};

	enum Shape
	{
		Shape_Sphere = 0,
		Shape_Mesh,

		Shape_MAX
	};

	Type m_type;
	Shape m_shape;

	f32 m_radius;
	ueVec3 m_direction;

	f32 m_spotWidth;
	f32 m_spotHeight;

	ueVec3 m_color;
};

struct ueLightmapperSetttings
{
	f32 m_texelDensity;
	u32 m_maxTextureDimension;

	ueLightmapperSetttings() :
		m_texelDensity(1.0f),
		m_maxTextureDimension(1024)
	{}
};

struct ueLightmapperImportanceVolume
{
};

void ueLightmapper_Startup();
void ueLightmapper_Shutdown();

void ueLightmapper_Reset(ueLightmapperSetttings* settings);
void ueLightmapper_AddModel(ueToolModel* model, const ueMat44* transform = &ueMat44_Identity);
void ueLightmapper_AddLight(ueToolLight* light);
void ueLightmapper_AddImportanceVolume(ueLightmapperImportanceVolume* volume);

void ueLightmapper_Run(volatile f32* progress, volatile bool* stop);

void ueLightmapper_GetResults();

#endif // UE_LIGHTMAPPER_H