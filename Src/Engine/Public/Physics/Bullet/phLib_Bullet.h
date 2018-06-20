#ifndef PH_LIB_BULLET_H
#define PH_LIB_BULLET_H

/**
 *	@addtogroup ph
 *	@{
 */

//! Startup parameters
struct phStartupParams : phStartupParams_Base
{
};

//! Scene description
struct phSceneDesc : phSceneInit_Base
{
	ueBool m_enableMultiThreadedSimulation;	//!< Enables multi-threaded simulation
	u32 m_numCoresToUse;					//!< Number of cores to use

	ueBool m_discreteWorld;					//!< Indicates whether to use discrete world
	ueBool m_supportSoftBodies;				//!< Indicates whether to support soft bodies

	phSceneDesc() :
		m_enableMultiThreadedSimulation(UE_TRUE),
		m_numCoresToUse(2),
		m_discreteWorld(UE_FALSE),
		m_supportSoftBodies(UE_FALSE)
	{}
};

// @}

#endif // PH_LIB_BULLET_H