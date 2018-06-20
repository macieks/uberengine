#ifndef GX_MODEL_INIT_DATA_H
#define GX_MODEL_INIT_DATA_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/ueBase.h"

struct glStreamFormatDesc;
struct gxCollisionGeometry_InitData;

//! Model initialization data; only used during model loading
struct gxModel_InitData
{
	struct VertexBufferDesc
	{
		u32 m_readOffset;

		u32 m_size;
	};

	struct IndexBufferDesc
	{
		u32 m_readOffset;

		u32 m_indexSize;
		u32 m_numIndices;
	};

	struct LOD
	{
		glStreamFormatDesc* m_formatDescs;

		VertexBufferDesc m_vbDesc;
		IndexBufferDesc m_ibDesc;

		gxCollisionGeometry_InitData* m_collisionGeomInitData;
	};

	ueBox m_box;		//!< Model bounding box
	u32 m_numLODs;		//!< Number of levels of detail of the model
	LOD* m_LODs;		//!< Array of per LOD init data
};

// @}

#endif // GX_MODEL_INIT_DATA_H
