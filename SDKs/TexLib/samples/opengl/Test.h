#include <stdio.h>    /* for printf and NULL */
#include <stdlib.h>   /* for exit */
#include <math.h>     /* for sqrt, sin, and cos */
#include <assert.h>   /* for assert */

#ifdef WIN32
	#include <windows.h>
#endif

#include <TexLib.h>

inline bool IsDXT(TexLib_Format format)
{
	return TexLib_Format_DXT1 <= format && format <= TexLib_Format_DXT5;
}

inline bool IsDXT1(TexLib_Format format)
{
	return TexLib_Format_DXT1 <= format && format <= TexLib_Format_DXT1a;
}

inline int NumBytesPerPixel(TexLib_Format format)
{
	if (format == TexLib_Format_ARGB) return 4;
	if (format == TexLib_Format_RGB) return 3;
	assert(!"DXT formats not supported here");
	return 0;
}

struct Tex
{
	int m_width;
	int m_height;
	TexLib_Format m_format;

	int m_lockedSize;
	void* m_locked;
	int m_lockedRowPitch;
	int m_lockedSlicePitch;

	Tex() :
		m_width(0),
		m_height(0),
		m_format(TexLib_Format_Unknown),
		m_lockedSize(0),
		m_locked(NULL),
		m_lockedRowPitch(0),
		m_lockedSlicePitch(0)
	{}

	inline TexLib_Format GetFormat() const { return m_format; }

	void Create();
	void* Lock(int face, int level);
	void Unlock(int face, int level);
};