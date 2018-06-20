#ifndef GX_DEBUG_MODEL_RENDERER_H
#define GX_DEBUG_MODEL_RENDERER_H

#include "Base/ueMath.h"

struct glCtx;
struct glProgram;
struct glTextureBuffer;
struct gxModel;
struct gxModelInstance;

/**
 *	@addtogroup gx
 *	@{
 */

//! Debug model renderer startup parameters
struct gxDebugModelRendererStartupParams
{
	const char* m_rigidVSName;			//!< Vertex shader to render rigid parts
	const char* m_skinnedVSName;		//!< Vertex shader to render skinned parts
	const char* m_FSName;				//!< Fragment shader
	const char* m_defaultTextureName;	//!< Default texture

	gxDebugModelRendererStartupParams() :
		m_rigidVSName("common/model_vs"),
		m_skinnedVSName("common/model_skinned_vs"),
		m_FSName("common/model_fs"),
		m_defaultTextureName("common/default_texture")
	{}
};

//! Debug model rendering parameters
struct gxDebugModelRendererDrawParams
{
	glCtx* m_ctx;						//!< Context to be used

	u32 m_lod;							//!< Level of detail index
	ueBool m_drawModel;					//!< Indicates whether to draw the actual model (instance) geometry
	ueBool m_drawSkeleton;				//!< Indicates whether to draw the model (instance) skeleton
	ueBool m_drawNormals;				//!< Indicates whether to draw normal vectors

	ueColor32 m_skeletonColor;			//!< Skeleton color (only used when m_drawSkeleton is set to true)

	gxModel* m_model;					//!< Optional model
	gxModelInstance* m_modelInstance;	//!< Optional model instance

	const ueMat44* m_viewProj;			//!< View-proj transformation
	const ueMat44* m_world;				//!< World transformation
	glProgram* m_rigidProgram;			//!< Optional override for rigid geometry program
	glProgram* m_skinnedProgram;		//!< Optional override for skinned geometry program

	glTextureBuffer* m_texture;			//!< Optional texture to use for all model meshes

	gxDebugModelRendererDrawParams() :
		m_ctx(NULL),
		m_lod(0),
		m_drawModel(UE_TRUE),
		m_drawSkeleton(UE_FALSE),
		m_drawNormals(UE_FALSE),
		m_skeletonColor(ueColor32::Yellow),
		m_model(NULL),
		m_modelInstance(NULL),
		m_viewProj(NULL),
		m_world(NULL),
		m_rigidProgram(NULL),
		m_skinnedProgram(NULL),
		m_texture(NULL)
	{}
};

//! Starts up debug model renderer
void gxDebugModelRenderer_Startup(gxDebugModelRendererStartupParams* params);
//! Shuts down debug model renderer
void gxDebugModelRenderer_Shutdown();

//! Draw the model (instance) and / or model (instance) skeleton
void gxDebugModelRenderer_Draw(gxDebugModelRendererDrawParams* params);

// @}

#endif // GX_DEBUG_MODEL_RENDERER_H