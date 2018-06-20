#ifndef GX_MODEL_PRIVATE_H
#define GX_MODEL_PRIVATE_H

#include "Base/ueResource_Private.h"

struct ioPackage;

//! 3D skeletal model resource
struct gxModel : ueResource
{
	gxModel();
	~gxModel();

	ueBox m_box;		//!< Bounding box

	u32 m_numLODs;		//!< Number of levels of detail
	gxModelLOD* m_LODs;	//!< Levels of detail

	u32 m_instanceSize; //!< Size (in bytes) of the gxModelInstance for this model

	// Used when loading

	ioPackage* m_package;
};

#endif // GX_MODEL_PRIVATE_H