#include "PathFinder.h"
#include "World.h"
#include "Path.h"
#include "Sample_RTS.h"

const Point neighboursCoords[] =
{
	Point(1, -1),
	Point(1, 0),
	Point(1, 1),
	Point(0, 1),
	Point(-1, 1),
	Point(-1, 0),
	Point(-1, -1),
	Point(0, -1)
};

const f32 diagonalDist = ueSqrt(2);

const f32 neighboursDists[] =
{
	diagonalDist,
	1,
	diagonalDist,
	1,
	diagonalDist,
	1,
	diagonalDist,
	1
};

f32 heuristic_estimate_of_distance(Field* start, Field* goal)
{
	const s16 xDist = ueAbs(start->m_pos.x - goal->m_pos.x);
	const s16 yDist = ueAbs(start->m_pos.y - goal->m_pos.y);
	const s16 distDiff = ueAbs(xDist - yDist);
	return (f32) (xDist + yDist - 2 * distDiff) + diagonalDist * distDiff;
}

void PathFinder::Init(ueAllocator* allocator)
{
	m_allocator = allocator;
	UE_ASSERT_FUNC(m_openset.Init(m_allocator, g_world->GetDim() * g_world->GetDim()));
}

void PathFinder::Deinit()
{
	m_openset.Deinit();
}

Path* PathFinder::AStar(Field* start, Field* goal)
{
	g_world->ResetForPathfinding();

	m_openset.Clear();

	start->g = 0;                        // Distance from start along optimal path.
	start->f = heuristic_estimate_of_distance(start, goal); // Estimated total distance from start to goal through y.
	start->m_flags |= Field::Flags_IsOpen;

	m_openset.Push(start);

	Field* x = NULL;
	Field* y = NULL;
	while (m_openset.Size() > 0)
	{
		x = m_openset.PopMin();
		if (x == goal)
		{
			//PrintPathTo(goal);
			return BuildPathTo(goal);
		}

		x->m_flags |= Field::Flags_IsClosed;
		for (int i = 0; i < ARRAYSIZE(neighboursCoords); i++)
		{
			y = g_world->GetEmptyFieldAt(x->m_pos + neighboursCoords[i]);

			if (!y || (y->m_flags & Field::Flags_IsClosed))
			   continue;

			const ueBool isNotOpen = !(y->m_flags & Field::Flags_IsOpen);
	
			const f32 tentative_g_score = x->g + neighboursDists[i];
			const ueBool tentative_better = isNotOpen || tentative_g_score < y->g;

			if (tentative_better)
			{
				y->m_parent = x;
				y->g = tentative_g_score;
				y->f = y->g + heuristic_estimate_of_distance(y, goal);
			}

			if (isNotOpen)
			{
				y->m_flags |= Field::Flags_IsOpen;
				m_openset.Push(y);
			}
		}
	}

	return NULL;
}

void PathFinder::PrintPathTo(Field* f)
{
	ueLog("path:\n");
	while (f)
	{
		ueLog("  %d %d\n", (s32) f->m_pos.x, (s32) f->m_pos.y);
		f = f->m_parent;
	}
}

Path* PathFinder::BuildPathTo(Field* end)
{
	u32 numPoints = 0;

	Field* f = end;
	while (f)
	{
		numPoints++;
		f = f->m_parent;
	}

	Path* path = (Path*) m_allocator->Alloc(sizeof(Path) + sizeof(Point) * numPoints);
	path->m_numPoints = numPoints;
	path->m_points = (Point*) (path + 1);

	f = end;
	while (f)
	{
		path->m_points[--numPoints] = f->m_pos;
		f = f->m_parent;
	}
	path->m_points[0].AsVec2(path->m_curveStartPoint);
	path->m_points[numPoints - 1].AsVec2(path->m_curveEndPoint);

	return path;
}

void PathFinder::DestroyPath(Path* path)
{
	m_allocator->Free(path);
}