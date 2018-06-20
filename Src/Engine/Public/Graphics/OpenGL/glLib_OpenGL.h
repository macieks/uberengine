#ifndef GL_LIB_OPENGL_H
#define GL_LIB_OPENGL_H

#include "Graphics/OpenGL/glStructs_OpenGL.h"

/**
 *	@addtogroup gl
 *	@{
 */

//! Checks whether given render group (e.g. FBO in OpenGL) is supported
ueBool glRenderGroup_CheckSupport(glRenderGroup* group);

// @}

#endif // GL_LIB_OPENGL_H
