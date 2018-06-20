#include <stdio.h>    /* for printf and NULL */
#include <stdlib.h>   /* for exit */
#include <math.h>     /* for sqrt, sin, and cos */
#include <assert.h>   /* for assert */

#include "TexLib.h"
#include "TexLib_Private.h"

bool TexLib_LoadTGA(TexLib_Context* ctx);
bool TexLib_LoadPNG(TexLib_Context* ctx);
bool TexLib_LoadJPG(TexLib_Context* ctx);
bool TexLib_LoadDDS(TexLib_Context* ctx);

void* TexLib_DefaultAlloc(TexLib_Context* ctx, int size)
{
	return malloc(size);
}

void TexLib_DefaultFree(TexLib_Context* ctx, void* ptr)
{
	return free(ptr);
}

bool TexLib_DefaultRead(TexLib_Context* ctx, void* data, int size)
{
	return fread(data, size, 1, (FILE*) ctx->m_privateData) == 1;
}

bool TexLib_Load(TexLib_Context* ctx)
{
	// Load from stream

	if (!ctx->m_loadFromFile)
	{
		switch (ctx->m_type)
		{
#ifdef TEXLIB_HAS_TGA
			case TexLib_Type_TGA: return TexLib_LoadTGA(ctx);
#endif
#ifdef TEXLIB_HAS_PNG
			case TexLib_Type_PNG: return TexLib_LoadPNG(ctx);
#endif
#ifdef TEXLIB_HAS_JPG
			case TexLib_Type_JPG: return TexLib_LoadJPG(ctx);
#endif
#ifdef TEXLIB_HAS_DDS
			case TexLib_Type_DDS: return TexLib_LoadDDS(ctx);
#endif
		}
		return false;
	}

	// Recognize texture type

	const char* ext = strstr(ctx->m_path, ".");
	if (!ext)
		return NULL;
	ext++;

	char extBuffer[5];
	const int extLength = strlen(ext);
	if (extLength > 4)
		return false;

	for (int i = 0; i < extLength; i++)
		extBuffer[i] = tolower(ext[i]);
	extBuffer[extLength] = 0;

	TexLib_Type type;
	if (!stricmp(extBuffer, "tga")) type = TexLib_Type_TGA;
	else if (!stricmp(extBuffer, "dds")) type = TexLib_Type_DDS;
	else if (!stricmp(extBuffer, "png")) type = TexLib_Type_PNG;
	else if (!stricmp(extBuffer, "jpg") || !stricmp(extBuffer, "jpeg")) type = TexLib_Type_JPG;
	else
		return false;

	// Open file

	FILE* file = fopen(ctx->m_path, "rb");
	if (!file)
		return false;

	TexLib_Context tempCtx = *ctx;
	tempCtx.m_loadFromFile = false;
	tempCtx.m_privateData = file;
	tempCtx.m_read = TexLib_DefaultRead;
	tempCtx.m_type = type;

	// Load!

	const bool result = TexLib_Load(&tempCtx);

	// Close the file

	fclose(file);

	return result;
}