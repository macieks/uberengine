#ifndef PH_LIB_PHYSX_H
#define PH_LIB_PHYSX_H

/**
 *	@addtogroup ph
 *	@{
 */

//! Startup parameters
struct phStartupParams : phStartupParams_Base
{
	u32 m_cookingBufferSize;				//!< Size of the buffer used for geometry cooking
	ueBool m_supportDebugVisualization;		//!< Indicates whether to support debug visualization

	phStartupParams() :
		m_cookingBufferSize(0),
		m_supportDebugVisualization(UE_TRUE)
	{}
};

//! Scene description
struct phSceneDesc : phSceneInit_Base
{
};

// @}

#endif // PH_LIB_PHYSX_H