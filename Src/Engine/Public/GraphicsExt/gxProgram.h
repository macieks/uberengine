#ifndef GX_PROGRAM_H
#define GX_PROGRAM_H

#include "Graphics/glLib.h"
#include "GraphicsExt/gxShader.h"

/**
 *	@addtogroup gx
 *	@{
 */

//! Shader program encapsulation
class gxProgram
{
public:
	gxProgram();
	//! Auto-destroys program (if not destroyed before)
	~gxProgram();
	//! Creates program from vertex, fragment and geometry shaders
	ueBool Create(const char* vsName, const char* fsName, const char* gsName = NULL);
	//! Destroys program
	void Destroy();
	//! Gets the low level program object
	glProgram* GetProgram() const;

private:
	glProgram* m_program; //!< Low level program object
	ueResourceHandle<gxShader> m_shaders[glShaderType_MAX]; //!< Handles to shaders
};

UE_INLINE gxProgram::gxProgram() : m_program(NULL) {}
UE_INLINE gxProgram::~gxProgram() { Destroy(); }

UE_INLINE ueBool gxProgram::Create(const char* vsName, const char* fsName, const char* gsName)
{
	Destroy();

	if (!m_shaders[glShaderType_Vertex].SetByName(vsName))
		return UE_FALSE;

	if (!m_shaders[glShaderType_Fragment].SetByName(fsName))
	{
		Destroy();
		return UE_FALSE;
	}

	if (gsName)
	{
		if (!m_shaders[glShaderType_Geometry].SetByName(gsName))
		{
			Destroy();
			return UE_FALSE;
		}
	}

	m_program = glProgram_Create(
		gxShader_GetShaderObject(*m_shaders[glShaderType_Vertex]),
		gxShader_GetShaderObject(*m_shaders[glShaderType_Fragment]),
		m_shaders[glShaderType_Geometry].IsValid() ? gxShader_GetShaderObject(*m_shaders[glShaderType_Geometry]) : NULL);
	if (!m_program)
	{
		Destroy();
		return UE_FALSE;
	}

	return UE_TRUE;
}

UE_INLINE void gxProgram::Destroy()
{
	if (!m_program) return;

	glProgram_Destroy(m_program);
	m_program = NULL;

	for (u32 i = 0; i < glShaderType_MAX; i++)
		m_shaders[i] = NULL;
}

UE_INLINE glProgram* gxProgram::GetProgram() const
{
	return m_program;
}

// @}

#endif