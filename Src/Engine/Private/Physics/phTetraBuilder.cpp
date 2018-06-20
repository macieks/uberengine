#include "Physics/phTetraBuilder.h"
#include "Base/ueMath.h"
#include "Base/Containers/ueHashSet.h"
#include "Base/Allocators/ueMallocAllocator.h"

void phTetraBuilder_ComputeBaryCoords(ueVec3& baryCoords, const ueVec3& vertex, const ueVec3& p0, const ueVec3& p1, const ueVec3& p2, const ueVec3& p3)
{
	ueVec3 q, q0, q1, q2;
	ueVec3::Sub(q, vertex, p3);
	ueVec3::Sub(q0, p0, p3);
	ueVec3::Sub(q1, p1, p3);
	ueVec3::Sub(q2, p2, p3);

	ueMat44 m;
	m.Set33(q0, q1, q2);

	const f32 det = m.Determinant();

	m.SetColumn(0, q);
	baryCoords[0] = m.Determinant();

	m.SetColumn(0, q0);
	m.SetColumn(1, q);
	baryCoords[1] = m.Determinant();

	m.SetColumn(1, q1);
	m.SetColumn(2, q);
	baryCoords[2] = m.Determinant();

	if (det != 0.0f)
		baryCoords /= det;
}

void phTetraBuilder_BuildTetraVertexLinks(phTetraVertexLink* links, const phTetraMesh& tetraMesh, const phTetraMesh& triMesh)
{
	const u32 numTetra = tetraMesh.m_numIndices / 4;

	for (u32 i = 0; i < triMesh.m_numVerts; i++)
	{
		const ueVec3 triVert(&triMesh.m_pos[i * 3]);

		f32 minDist = 0.0f;
		phTetraVertexLink tmpLink;

		for (u32 j = 0; j < numTetra; j++)
		{
			const u32* ix = &tetraMesh.m_indices[j * 4];
			const ueVec3 p0(&tetraMesh.m_pos[ ix[0] * 3 ]);
			const ueVec3 p1(&tetraMesh.m_pos[ ix[1] * 3 ]);
			const ueVec3 p2(&tetraMesh.m_pos[ ix[2] * 3 ]);
			const ueVec3 p3(&tetraMesh.m_pos[ ix[3] * 3 ]);

			ueVec3 b;
			phTetraBuilder_ComputeBaryCoords(b, triVert, p0, p1, p2, p3);

			// Is the vertex inside the tetrahedron? If yes we take it

			if (b[0] >= 0.0f && b[1] >= 0.0f && b[2] >= 0.0f &&
				(b[0] + b[1] + b[2]) <= 1.0f)
			{
				b.Store3(tmpLink.m_barycentricCoords);
				tmpLink.m_tetraIndex = j;
				break;
			}

			// Otherwise, if we are not in any tetrahedron we take the closest one

			f32 dist = 0.0f;

			if (b[0] + b[1] + b[2] > 1.0f) dist = b[0] + b[1] + b[2] - 1.0f;

			if (b[0] < 0.0f) dist = (-b[0] < dist) ? dist : -b[0];
			if (b[1] < 0.0f) dist = (-b[1] < dist) ? dist : -b[1];
			if (b[2] < 0.0f) dist = (-b[2] < dist) ? dist : -b[2];

			if (i == 0 || dist < minDist)
			{
				minDist = dist;
				b.Store3(tmpLink.m_barycentricCoords);
				tmpLink.m_tetraIndex = j;
			}
		}

		links[i] = tmpLink;
	}
}

void phTetraBuilder_BuildTetraLinks(phTetraMesh& m)
{
	UE_ASSERT(m.m_isTetra);

	const u32 maxLinks = m.m_numIndices / 4 * 6;

	ueMallocAllocator allocator;
	ueHashSet<phTetraLink> links;
	UE_ASSERT_FUNC(links.Init(&allocator, maxLinks));

	// Collect unique edges from all tetrahedrons

	for (u32 i = 0; i < m.m_numIndices; i += 4)
	{
		const u32* ids = m.m_indices + i;

		// For each tetrahedron's edge

		for (u32 a = 0; a < 3; a++)
			for (u32 b = a + 1; b < 4; b++)
			{
				phTetraLink link;
				link.a = ueMin(ids[a], ids[b]);
				link.b = ueMax(ids[b], ids[a]);

				links.Insert(link);
			}
	}

	// Copy to array

	UE_ASSERT(m.m_numVerts < (1 << 16));

	m.m_links = (phTetraLink*) malloc(sizeof(phTetraLink) * links.Size()); // malloc is okay - it's mostly meant for tool side
	UE_ASSERT(m.m_links);
	m.m_numLinks = 0;

	ueHashSet<phTetraLink>::Iterator iter(links);
	while (phTetraLink* link = iter.Next())
		m.m_links[m.m_numLinks++] = *link;
}

struct phTetraFace
{
	u16 m_ids[3];
	u16 m_counter;

	UE_INLINE phTetraFace() : m_counter(1) {}
	UE_INLINE void SortIds()
	{
		for (u32 i = 0; i < 2; i++)
			for (u32 j = i + 1; j < 2; j++)
				if (m_ids[i] > m_ids[j])
				{
					const u16 temp = m_ids[i];
					m_ids[i] = m_ids[j];
					m_ids[j] = temp;
				}
	}

	struct HashPred
	{
		UE_INLINE u32 operator () (const phTetraFace& face) const
		{
			return face.m_ids[0] + (face.m_ids[1] << 16) + (face.m_ids[2] << 8);
		}
	};
};

void phTetraBuilder_BuildTetraOuterFaces(phTetraMesh& m)
{
	UE_ASSERT(m.m_isTetra);
	UE_ASSERT(m.m_numVerts < (1 << 16));

	const u32 maxFaces = m.m_numIndices;

	ueMallocAllocator allocator;
	ueHashSet<phTetraFace, phTetraFace::HashPred> faces;
	UE_ASSERT_FUNC(faces.Init(&allocator, maxFaces));

	// Collect unique faces from all tetrahedrons

	for (u32 i = 0; i < m.m_numIndices; i += 4)
	{
		const u32* ids = m.m_indices + i;

		// For each tetrahedron's face

		for (u32 a = 0; a < 2; a++)
			for (u32 b = a + 1; b < 3; b++)
				for (u32 c = b + 1; c < 4; c++)
				{
					phTetraFace face;
					face.m_ids[0] = ids[a];
					face.m_ids[1] = ids[b];
					face.m_ids[2] = ids[c];
					face.SortIds();

					phTetraFace* existingFace = faces.Find(face);
					if (existingFace)
						existingFace->m_counter++;
					else
						faces.Insert(face);
				}
	}

	// Copy to array

	m.m_outerFaces = (u16*) malloc(sizeof(u16) * 3 * faces.Size()); // malloc is okay - it's mostly meant for tool side
	UE_ASSERT(m.m_outerFaces);
	m.m_numOuterFaces = 0;

	ueHashSet<phTetraFace, phTetraFace::HashPred>::Iterator iter(faces);
	while (phTetraFace* face = iter.Next())
	{
		if (face->m_counter > 1)
			continue;
		for (u32 i = 0; i < 3; i++)
			m.m_outerFaces[m.m_numOuterFaces * 3 + i] = face->m_ids[i];
		m.m_numOuterFaces++;
	}
}