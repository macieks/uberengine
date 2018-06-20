#ifndef GX_ANIMATION_PRIVATE_H
#define GX_ANIMATION_PRIVATE_H

#include "Base/ueResource_Private.h"

struct ioPackage;

/**
 *	Skeletal key-frame based animation.
 *
 *	An animation can be applied to any skeleton.
 *	Bone matching (between animation bones and skeleton bones) is done automatically.
 */
struct gxAnimation : ueResource
{
	// Animated node
	struct Node
	{
		const char* m_name;

		s32 m_numRotationKeys;
		f32* m_rotationTimes;
		ueQuat* m_rotationKeys;
		f32 m_rotationFrequencyInv;

		s32 m_numTranslationKeys;
		f32* m_translationTimes;
		ueVec3* m_translationKeys;
		f32 m_translationFrequencyInv;

		s32 m_numScaleKeys;
		f32* m_scaleTimes;
		ueVec3* m_scaleKeys;
		f32 m_scaleFrequencyInv;
	};

	struct Data
	{
		f32 m_length;
		u32 m_numNodes;
		Node* m_nodes;
	};

	Data* m_data;

	// Loading only data

	ioPackage* m_package;
};

#endif // GX_ANIMATION_PRIVATE_H