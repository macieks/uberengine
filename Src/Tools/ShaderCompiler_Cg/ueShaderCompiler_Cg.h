#include "ShaderCompiler_Common/ueToolShader.h"

#include <Cg/cg.h>    // Can't include this?  Is Cg Toolkit installed?

enum CgType
{
	CgType_OpenGL = 0,
	CgType_D3D11,

	CgType_MAX
};

extern CgType s_cgType;

bool GetSemanticAndUsageFromString(glSemantic& semantic, u32& usageIndex, const char* semanticFullName, const char* semanticName, glSemantic candidateSemantic);

// OpenGL

void ShaderCompiler_Cg_Init_OpenGL(bool isGLES);
void ShaderCompiler_Cg_PostInit_OpenGL();
CGprofile ShaderCompiler_Cg_SelectProfile_OpenGL(glShaderType shaderType);
bool ShaderCompiler_Cg_GetSemanticAndUsageFromString_OpenGL(glSemantic& semantic, u32& usageIndex, const char* semanticFullName);
const char* ShaderCompiler_Cg_PreprocessProgram_OpenGL(glShaderType shaderType, const std::vector<std::string>& semanticNames, const char* programString);

// D3D11 (stub implementation)

UE_INLINE void ShaderCompiler_Cg_Init_D3D11() {}
UE_INLINE void ShaderCompiler_Cg_PostInit_D3D11() {}
UE_INLINE CGprofile ShaderCompiler_Cg_SelectProfile_D3D11(glShaderType shaderType) { return CG_PROFILE_UNKNOWN; }
UE_INLINE bool ShaderCompiler_Cg_GetSemanticAndUsageFromString_D3D11(glSemantic& semantic, u32& usageIndex, const char* semanticFullName) { return false; }
UE_INLINE const char* ShaderCompiler_Cg_PreprocessProgram_D3D11(glShaderType shaderType, const std::vector<std::string>& semanticNames, const char* programString) { return NULL; }