#pragma once

#include "Base/ueToolsBase.h"

//! General purpose multi-channel texture rectangle packer (can pack different rectangles into different texture channels)
class RectPacker
{
public:
	struct Rect
	{
		unsigned int x;
		unsigned int y;
		unsigned int w;
		unsigned int h;

		unsigned int layer;

		void* userData;

		struct Cmp
		{
			UE_INLINE bool operator () (const Rect& a, const Rect& b) const
			{
				return b.w < a.w || (b.w == a.w && b.h < a.h);
			}
		};
	};

	//! Packs set of rectangles into possibly smallest rectangle
	static void Solve(unsigned int spaceBetweenRects, unsigned int numLayers, bool pow2Dim, std::vector<Rect>& rects, unsigned int& width, unsigned int& height);
	//! Packs set of rectangles into fixed size rectangle
	static bool SolveFixedRect(unsigned int spaceBetweenRects, unsigned int numLayers, std::vector<Rect>& rects, unsigned int width, unsigned int height);
};