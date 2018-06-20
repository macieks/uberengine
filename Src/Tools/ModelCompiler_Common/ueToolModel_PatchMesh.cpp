#include "ModelCompiler_Common/ueToolModel.h"

typedef ueVec3 vec3_t;

// Quake3 vector stuff wrapper

UE_INLINE void VectorSubtract(const vec3_t& a, const vec3_t& b, vec3_t& out) { out = a - b; }
UE_INLINE void VectorAdd(const vec3_t& a, const vec3_t& b, vec3_t& out) { out = a + b; }
UE_INLINE void VectorNormalize(vec3_t& v) { v.Normalize(); }
UE_INLINE float VectorNormalize2(const vec3_t& v, vec3_t& out) { out = v; out.Normalize(); return 1.0f; }

UE_INLINE float DotProduct(const vec3_t& a, const vec3_t& b) { return ueVec3::Dot(a, b); }
UE_INLINE void VectorScale(const vec3_t& v, float scale, vec3_t& out) { out = v * scale; }
UE_INLINE float VectorLengthSquared(const vec3_t& v) { return v.LenSq(); }

UE_INLINE void VectorClear(vec3_t& v) { v.Zeroe(); }
UE_INLINE void VectorCopy(const vec3_t& v, vec3_t& out) { out = v; }
UE_INLINE void CrossProduct(const vec3_t& a, const vec3_t& b, vec3_t& out) { ueVec3::Cross(out, a, b); }

// Source: Quake 3 source code
// Copyright (C) 1999-2000 Id Software, Inc.

/*
============
LerpDrawVert
============
*/
static void LerpDrawVert(ueToolModel::Vertex *a, ueToolModel::Vertex *b, ueToolModel::Vertex *out) {
	UE_ASSERT(a->m_flags == b->m_flags);
	out->m_flags = a->m_flags;
	out->m_pos = (a->m_pos + b->m_pos) * 0.5f;

	for (int i = 0; i < ueToolModel::Vertex::MAX_TEX_STREAMS; i++)
		if (a->m_flags & (ueToolModel::Vertex::Flags_HasTex0 << i))
			out->m_tex[i] = (a->m_tex[i] + b->m_tex[i]) * 0.5f;

	out->m_color = ueColor32::Lerp(a->m_color, b->m_color, 0.5f);
}

/*
============
Transpose
============
*/
static void Transpose( int width, int height, ueToolModel::Vertex** ctrl) {
	int		i, j;
	ueToolModel::Vertex	temp;

	if ( width > height ) {
		for ( i = 0 ; i < height ; i++ ) {
			for ( j = i + 1 ; j < width ; j++ ) {
				if ( j < height ) {
					// swap the value
					temp = ctrl[j][i];
					ctrl[j][i] = ctrl[i][j];
					ctrl[i][j] = temp;
				} else {
					// just copy
					ctrl[j][i] = ctrl[i][j];
				}
			}
		}
	} else {
		for ( i = 0 ; i < width ; i++ ) {
			for ( j = i + 1 ; j < height ; j++ ) {
				if ( j < width ) {
					// swap the value
					temp = ctrl[i][j];
					ctrl[i][j] = ctrl[j][i];
					ctrl[j][i] = temp;
				} else {
					// just copy
					ctrl[i][j] = ctrl[j][i];
				}
			}
		}
	}

}

/*
=================
MakeMeshNormals

Handles all the complicated wrapping and degenerate cases
=================
*/
static void MakeMeshNormals( int width, int height, ueToolModel::Vertex** ctrl) {
	int		i, j, k, dist;
	vec3_t	normal;
	vec3_t	sum;
	int		count;
	vec3_t	base;
	vec3_t	delta;
	int		x, y;
	ueToolModel::Vertex	*dv;
	vec3_t		around[8], temp;
	bool	good[8];
	bool	wrapWidth, wrapHeight;
	float		len;
static	int	neighbors[8][2] = {
	{0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}, {-1,1}
	};

	wrapWidth = false;
	for ( i = 0 ; i < height ; i++ ) {
		VectorSubtract( ctrl[i][0].m_pos, ctrl[i][width-1].m_pos, delta );
		len = VectorLengthSquared( delta );
		if ( len > 1.0 ) {
			break;
		}
	}
	if ( i == height ) {
		wrapWidth = true;
	}

	wrapHeight = false;
	for ( i = 0 ; i < width ; i++ ) {
		VectorSubtract( ctrl[0][i].m_pos, ctrl[height-1][i].m_pos, delta );
		len = VectorLengthSquared( delta );
		if ( len > 1.0 ) {
			break;
		}
	}
	if ( i == width) {
		wrapHeight = true;
	}


	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height ; j++ ) {
			count = 0;
			dv = &ctrl[j][i];
			VectorCopy( dv->m_pos, base );
			for ( k = 0 ; k < 8 ; k++ ) {
				VectorClear( around[k] );
				good[k] = false;

				for ( dist = 1 ; dist <= 3 ; dist++ ) {
					x = i + neighbors[k][0] * dist;
					y = j + neighbors[k][1] * dist;
					if ( wrapWidth ) {
						if ( x < 0 ) {
							x = width - 1 + x;
						} else if ( x >= width ) {
							x = 1 + x - width;
						}
					}
					if ( wrapHeight ) {
						if ( y < 0 ) {
							y = height - 1 + y;
						} else if ( y >= height ) {
							y = 1 + y - height;
						}
					}

					if ( x < 0 || x >= width || y < 0 || y >= height ) {
						break;					// edge of patch
					}
					VectorSubtract( ctrl[y][x].m_pos, base, temp );
					if ( VectorNormalize2( temp, temp ) == 0 ) {
						continue;				// degenerate edge, get more dist
					} else {
						good[k] = true;
						VectorCopy( temp, around[k] );
						break;					// good edge
					}
				}
			}

			VectorClear( sum );
			for ( k = 0 ; k < 8 ; k++ ) {
				if ( !good[k] || !good[(k+1)&7] ) {
					continue;	// didn't get two points
				}
				CrossProduct( around[(k+1)&7], around[k], normal );
				if ( VectorNormalize2( normal, normal ) == 0 ) {
					continue;
				}
				VectorAdd( normal, sum, sum );
				count++;
			}
			if ( count == 0 ) {
//printf("bad normal\n");
				count = 1;
			}
			VectorNormalize2( sum, dv->m_normal );
		}
	}
}

/*
==================
PutPointsOnCurve
==================
*/
static void PutPointsOnCurve( ueToolModel::Vertex**	ctrl, int width, int height ) {
	int			i, j;
	ueToolModel::Vertex	prev, next;

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 1 ; j < height ; j += 2 ) {
			LerpDrawVert( &ctrl[j][i], &ctrl[j+1][i], &prev );
			LerpDrawVert( &ctrl[j][i], &ctrl[j-1][i], &next );
			LerpDrawVert( &prev, &next, &ctrl[j][i] );
		}
	}


	for ( j = 0 ; j < height ; j++ ) {
		for ( i = 1 ; i < width ; i += 2 ) {
			LerpDrawVert( &ctrl[j][i], &ctrl[j][i+1], &prev );
			LerpDrawVert( &ctrl[j][i], &ctrl[j][i-1], &next );
			LerpDrawVert( &prev, &next, &ctrl[j][i] );
		}
	}
}

/*
=================
R_SubdividePatchToGrid
=================
*/
void ueToolModel::SubdividePatchMesh(ueToolModel::Mesh* mesh, u32 maxDimension, f32 subdivisions)
{
	int			i, j, k, l;
	ueToolModel::Vertex		prev, next, mid;
	float		len, maxLen;
	int			dir;
	int			t;

	UE_ASSERT(mesh->m_flags & ueToolModel::Mesh::Flags_IsPatch);

	// Allocate memory for points and error table

	char* tempMemory = (char*) malloc((sizeof(ueToolModel::Vertex*) + sizeof(ueToolModel::Vertex) * maxDimension + 2 * sizeof(float)) * maxDimension);
	UE_ASSERT(tempMemory);
	char* memoryPtr = tempMemory;

	ueToolModel::Vertex** ctrl = (ueToolModel::Vertex**) memoryPtr;
	memoryPtr += sizeof(ueToolModel::Vertex*) * maxDimension;
	for (i = 0; i < (int) maxDimension; i++)
	{
		ctrl[i] = (ueToolModel::Vertex*) memoryPtr;
		memoryPtr += sizeof(ueToolModel::Vertex) * maxDimension;
	}

	float* errorTable[2];
	errorTable[0] = (float*) memoryPtr;
	memoryPtr += sizeof(float) * maxDimension;
	errorTable[1] = (float*) memoryPtr;

	int width = mesh->m_patch.m_width;
	int height = mesh->m_patch.m_height;

	// Copy control points into temp grid

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height ; j++ ) {
			ctrl[j][i] = mesh->m_patch.m_controlPoints[j*width+i];
		}
	}

	// Subdivide

	for ( dir = 0 ; dir < 2 ; dir++ ) {

		for ( j = 0 ; j < (int) maxDimension ; j++ ) {
			errorTable[dir][j] = 0;
		}

		// horizontal subdivisions
		for ( j = 0 ; j + 2 < width ; j += 2 ) {
			// check subdivided midpoints against control points

			// Should also check midpoints of adjacent patches against the control points
			// this would basically stitch all patches in the same LOD group together.

			maxLen = 0;
			for ( i = 0 ; i < height ; i++ ) {
				vec3_t		midxyz;
				vec3_t		midxyz2;
				vec3_t		dir;
				vec3_t		projected;
				float		d;

				// calculate the point on the curve
				for ( l = 0 ; l < 3 ; l++ ) {
					midxyz[l] = (ctrl[i][j].m_pos[l] + ctrl[i][j+1].m_pos[l] * 2 + ctrl[i][j+2].m_pos[l]) * 0.25f;
				}

				// see how far off the line it is
				// using dist-from-line will not account for internal
				// texture warping, but it gives a lot less polygons than
				// dist-from-midpoint
				VectorSubtract( midxyz, ctrl[i][j].m_pos, midxyz );
				VectorSubtract( ctrl[i][j+2].m_pos, ctrl[i][j].m_pos, dir );
				VectorNormalize( dir );

				d = DotProduct( midxyz, dir );
				VectorScale( dir, d, projected );
				VectorSubtract( midxyz, projected, midxyz2);
				len = VectorLengthSquared( midxyz2 );			// we will do the sqrt later
				if ( len > maxLen ) {
					maxLen = len;
				}
			}

			maxLen = sqrt(maxLen);

			// if all the points are on the lines, remove the entire columns
			if ( maxLen < 0.1f ) {
				errorTable[dir][j+1] = 999;
				continue;
			}

			// see if we want to insert subdivided columns
			if ( width + 2 > (int) maxDimension ) {
				errorTable[dir][j+1] = 1.0f/maxLen;
				continue;	// can't subdivide any more
			}

			if ( maxLen <= subdivisions ) {
				errorTable[dir][j+1] = 1.0f/maxLen;
				continue;	// didn't need subdivision
			}

			errorTable[dir][j+2] = 1.0f/maxLen;

			// insert two columns and replace the peak
			width += 2;
			for ( i = 0 ; i < height ; i++ ) {
				LerpDrawVert( &ctrl[i][j], &ctrl[i][j+1], &prev );
				LerpDrawVert( &ctrl[i][j+1], &ctrl[i][j+2], &next );
				LerpDrawVert( &prev, &next, &mid );

				for ( k = width - 1 ; k > j + 3 ; k-- ) {
					ctrl[i][k] = ctrl[i][k-2];
				}
				ctrl[i][j + 1] = prev;
				ctrl[i][j + 2] = mid;
				ctrl[i][j + 3] = next;
			}

			// back up and recheck this set again, it may need more subdivision
			j -= 2;

		}

		Transpose( width, height, ctrl );
		t = width;
		width = height;
		height = t;
	}


	// put all the aproximating points on the curve
	PutPointsOnCurve( ctrl, width, height );

	// cull out any rows or columns that are colinear
	for ( i = 1 ; i < width-1 ; i++ ) {
		if ( errorTable[0][i] != 999 ) {
			continue;
		}
		for ( j = i+1 ; j < width ; j++ ) {
			for ( k = 0 ; k < height ; k++ ) {
				ctrl[k][j-1] = ctrl[k][j];
			}
			errorTable[0][j-1] = errorTable[0][j];
		}
		width--;
	}

	for ( i = 1 ; i < height-1 ; i++ ) {
		if ( errorTable[1][i] != 999 ) {
			continue;
		}
		for ( j = i+1 ; j < height ; j++ ) {
			for ( k = 0 ; k < width ; k++ ) {
				ctrl[j-1][k] = ctrl[j][k];
			}
			errorTable[1][j-1] = errorTable[1][j];
		}
		height--;
	}

#if 0 // Disabled as we're not using tri-strips

	// flip for longest tristrips as an optimization
	// the results should be visually identical with or
	// without this step
	if ( height > width ) {
		Transpose( width, height, ctrl );
		InvertErrorTable( errorTable, width, height );
		t = width;
		width = height;
		height = t;
		InvertCtrl( width, height, ctrl );
	}

#endif

	// calculate normals
	MakeMeshNormals( width, height, ctrl );

	// Build Mesh verts and indices

	mesh->m_verts.clear();
	mesh->m_verts.reserve(width * height);
	for (i = 0; i < width; i++)
		for (j = 0; j < height; j++)
			mesh->m_verts.push_back(ctrl[j][i]);

	mesh->m_indices.clear();
	mesh->m_indices.reserve((width - 1) * (height - 1) * 3 * 2);
	for (i = 0; i < width - 1; i++)
		for (j = 0; j < height - 1; j++)
		{
			const int indices[4] =
			{
				j + i * height,
				j + (i + 1) * height,
				j + 1 + (i + 1) * height,
				j + 1 + i * height
			};

			mesh->m_indices.push_back(indices[0]);
			mesh->m_indices.push_back(indices[1]);
			mesh->m_indices.push_back(indices[2]);

			mesh->m_indices.push_back(indices[0]);
			mesh->m_indices.push_back(indices[2]);
			mesh->m_indices.push_back(indices[3]);
		}

	// Release temp memory

	free(tempMemory);
}

void ueToolModel::ConvertPatchesToTrimeshes()
{
	ueLogI("Converting patches to tri-meshes...");

	FOR_EACH_MESH(mesh)
	{
		if (!mesh->IsPatch())
			continue;

		// Generate tri-mesh data from patch

		SubdividePatchMesh(mesh, 128, 16);

		// Remove patch information

		mesh->m_flags &= ~Mesh::Flags_IsPatch;
		mesh->m_patch.m_width = 0;
		mesh->m_patch.m_height = 0;
		mesh->m_patch.m_controlPoints.clear();
	}
}