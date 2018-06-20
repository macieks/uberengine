#include "ShaderCompiler_Common/ueToolShader.h"
#include "ueShaderCompiler_Cg.h"

#define GLEW_STATIC

#include "GL/glew.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#ifdef _WIN32
	#include <GL/wglew.h>
#else
	#ifdef __APPLE__
		#include <OpenGL/OpenGL.h>
	#else
		#include <GL/glxew.h>
	#endif
#endif

#include <Cg/cgGL.h>

static bool s_isGLES = false;

bool ShaderCompiler_Cg_GetSemanticAndUsageFromString_OpenGL(glSemantic& semantic, u32& usageIndex, const char* semanticFullName)
{
#define CHECK_SEMANTIC(name, enumName) if (GetSemanticAndUsageFromString(semantic, usageIndex, semanticFullName, #name, glSemantic_##enumName)) return true;

	CHECK_SEMANTIC(POSITION, Position)
	CHECK_SEMANTIC(BLENDWEIGHT, BoneWeights)
	CHECK_SEMANTIC(BLENDINDICES, BoneIndices)
	CHECK_SEMANTIC(NORMAL, Normal)
	CHECK_SEMANTIC(POINTSIZE, PointSize)
	CHECK_SEMANTIC(TEXCOORD, TexCoord)
	CHECK_SEMANTIC(TANGENT, Tangent)
	CHECK_SEMANTIC(BINORMAL, Binormal)
	CHECK_SEMANTIC(TESSFACTOR, TessFactor)
	CHECK_SEMANTIC(COLOR, Color)
	CHECK_SEMANTIC(FOG, Fog)
	CHECK_SEMANTIC(DEPTH, Depth)
	CHECK_SEMANTIC(SAMPLE, Sample)
	CHECK_SEMANTIC(ATTR, Generic)

	/*
		Built-in GLSL vertex shader input variables:

		gl_Color
		gl_SecondaryColor
		gl_Normal
		gl_Vertex
		gl_MultiTexCoordn
		gl_FogCoord
	*/

	return false;
}

template <typename KEY_TYPE, typename VALUE_TYPE>
void multimap_insert(std::multimap<KEY_TYPE, VALUE_TYPE>& map, const std::string& key, const std::string& value)
{
	map.insert( std::pair<std::string, std::string>(key, value) );
}

const char* RemoveNonGenericAttributes(const char* input, const std::vector<std::string>& semanticNames)
{
	// Build map from GLSL built-in attributes to HLSL like semantic names

	std::multimap<std::string, std::string> GLSLToHLSLSemantic;
	multimap_insert(GLSLToHLSLSemantic, "gl_Vertex", "POSITION0");
	multimap_insert(GLSLToHLSLSemantic, "gl_Vertex", "POSITION");
	multimap_insert(GLSLToHLSLSemantic, "gl_Normal", "NORMAL0");
	multimap_insert(GLSLToHLSLSemantic, "gl_Normal", "NORMAL");
	multimap_insert(GLSLToHLSLSemantic, "gl_Color", "COLOR0");
	multimap_insert(GLSLToHLSLSemantic, "gl_Color", "COLOR");
	multimap_insert(GLSLToHLSLSemantic, "gl_SecondaryColor", "COLOR1");
	multimap_insert(GLSLToHLSLSemantic, "gl_FogCoord", "FOGCOORD0");
	for (int i = 0; i < 8; i++)
	{
		std::string indexAsString = "";
		string_format(indexAsString, "%d", i);
		multimap_insert(GLSLToHLSLSemantic, "gl_MultiTexCoord" + indexAsString, "TEXCOORD" + indexAsString);
	}

	// Replace all keys with values in the input string
	// Also add corresponding attribute declarations to GLSL source

	std::string output = input;
	for (std::multimap<std::string, std::string>::iterator i = GLSLToHLSLSemantic.begin(); i != GLSLToHLSLSemantic.end(); i++)
		if (vector_contains(semanticNames, i->second) && string_replace_all(output, i->first, i->second) > 0) // Replace
			output = "attribute vec4 " + i->second + ";\n" + output; // Declare

	return strdup(output.c_str()); // Don't care about memory leaks - this tool is going to exit soon anyway
}

const char* AddPrecisionInfo(const char* input, glShaderType type)
{
	if (!s_isGLES || type != glShaderType_Fragment)
		return input;

	const std::string output =
		std::string("precision mediump float;\n") +
		input;

	return strdup(output.c_str()); // Don't care about memory leaks - this tool is going to exit soon anyway
}

const char* RemoveNonGenericVaryings(const char* input, glShaderType type)
{
	const char* fsVaryingsToReplace[] =
	{
		"gl_Color", "VAR_Color0",
		"gl_SecondaryColor", "VAR_Color1",
		"gl_TexCoord[0]", "VAR_TexCoord0",
		"gl_TexCoord[1]", "VAR_TexCoord1",
		"gl_TexCoord[2]", "VAR_TexCoord2",
		"gl_TexCoord[3]", "VAR_TexCoord3",
		"gl_TexCoord[4]", "VAR_TexCoord4",
		"gl_TexCoord[5]", "VAR_TexCoord5",
		"gl_TexCoord[6]", "VAR_TexCoord6",
		"gl_TexCoord[7]", "VAR_TexCoord7",
		NULL
	};

	const char* vsVaryingsToReplace[] =
	{
		"gl_FrontColor", "VAR_Color0",
		"gl_BackColor", "VAR_BackColor0",
		"gl_FrontSecondaryColor", "VAR_Color1",
		"gl_BackSecondaryColor", "VAR_BackColor1",
		"gl_TexCoord[0]", "VAR_TexCoord0",
		"gl_TexCoord[1]", "VAR_TexCoord1",
		"gl_TexCoord[2]", "VAR_TexCoord2",
		"gl_TexCoord[3]", "VAR_TexCoord3",
		"gl_TexCoord[4]", "VAR_TexCoord4",
		"gl_TexCoord[5]", "VAR_TexCoord5",
		"gl_TexCoord[6]", "VAR_TexCoord6",
		"gl_TexCoord[7]", "VAR_TexCoord7",
		NULL
	};

	const char** varyingsToReplace = NULL;
	switch (type)
	{
		case glShaderType_Geometry: return input;
		case glShaderType_Vertex: varyingsToReplace = vsVaryingsToReplace; break;
		case glShaderType_Fragment: varyingsToReplace = fsVaryingsToReplace; break;
	}

	std::string output = input;
	for (u32 i = 0; varyingsToReplace[i]; i += 2)
		if (string_replace_all(output, varyingsToReplace[i], varyingsToReplace[i + 1]) > 0)		// Replace
			output = std::string("varying vec4 ") + varyingsToReplace[i + 1] + ";\n" + output;	// Declare

	return strdup(output.c_str()); // Don't care about memory leaks - this tool is going to exit soon anyway
}

void ShaderCompiler_Cg_Init_OpenGL(bool isGLES)
{
	s_isGLES = isGLES;

	// Minimal OpenGL startup

	glutInitWindowSize(100, 100);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutCreateWindow("UberEngine Cg Shader Compiler");
}

void ShaderCompiler_Cg_PostInit_OpenGL()
{
	cgGLSetDebugMode(CG_TRUE);
}

CGprofile ShaderCompiler_Cg_SelectProfile_OpenGL(glShaderType shaderType)
{
	CGprofile profile;
	switch (shaderType)
	{
		case glShaderType_Vertex: profile = CG_PROFILE_GLSLV; break; /* cgGLGetLatestProfile(CG_GL_VERTEX); */
		case glShaderType_Fragment: profile = CG_PROFILE_GLSLF; break; /* cgGLGetLatestProfile(CG_GL_FRAGMENT); */
		case glShaderType_Geometry: profile = CG_PROFILE_GLSLG; break; /* cgGLGetLatestProfile(CG_GL_GEOMETRY); */
		UE_INVALID_CASE(shaderType);
	}

	cgGLSetOptimalOptions(profile);
	const char** optimalOptions = cgGLGetOptimalOptions(profile);

	return profile;
}

const char* ShaderCompiler_Cg_PreprocessProgram_OpenGL(glShaderType shaderType, const std::vector<std::string>& semanticNames, const char* programString)
{
	// Replace built-in (vertex shader) attributes with generic attributes (so, we can avoid fixed pipeline limitations)

	programString =
		(shaderType == glShaderType_Vertex ?
			RemoveNonGenericAttributes(programString, semanticNames) :
			programString);

	// Replace built-in varyings with generic varyings (so, we can avoid e.g. color clamping to 0..1 range)

	programString = RemoveNonGenericVaryings(programString, shaderType);

	// Add precision information if GLES

	programString = AddPrecisionInfo(programString, shaderType);

	return programString;
}