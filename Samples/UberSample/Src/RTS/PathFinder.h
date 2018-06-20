#pragma once

#include "Field.h"

#include "Base/Containers/ueHeap.h"

class World;
struct Path;

class PathFinder
{
public:
	void Init(ueAllocator* allocator);
	void Deinit();

	Path* AStar(Field* start, Field* goal);
	void DestroyPath(Path* path);

private:

	void PrintPathTo(Field* end);
	Path* BuildPathTo(Field* end);

	struct CmpFieldByF
	{
		s32 operator() (const Field* a, const Field* b) const
		{
			return a->f < b->f ? 1 : (a->f > b->f ? -1 : 0);
		}
	};

	ueAllocator* m_allocator;
	ueHeap<Field*, CmpFieldByF> m_openset;
};