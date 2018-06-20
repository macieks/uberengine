#include "Physics/phTetraBuilder.h"

#include "NxTetra.h"
typedef NxTetraInterface* (__cdecl *NX_GetTetraInterfaceFunc)();

static HMODULE s_tetraDLL = 0;
static NxTetraInterface* s_tetraInterface = NULL;

NxTetraInterface* NxGetTetraInterface()
{
	s_tetraDLL = LoadLibraryA("NxTetra.dll");
	if (!s_tetraDLL)
	{
		ueLogE("Failed to get NXTetraInterface, reason: NxTetra.dll not found.");
		return NULL;
	}

	NX_GetTetraInterfaceFunc getTetraInterfaceFunc = (NX_GetTetraInterfaceFunc) GetProcAddress(s_tetraDLL, "getTetraInterface");
	return getTetraInterfaceFunc ? getTetraInterfaceFunc() : NULL;
}

void phTetraBuilder_Startup()
{
	UE_ASSERT(!s_tetraInterface);
	s_tetraInterface = NxGetTetraInterface();
	UE_ASSERT(s_tetraInterface);
}

void phTetraBuilder_Shutdown()
{
	UE_ASSERT(s_tetraInterface);
	FreeLibrary(s_tetraDLL);
	s_tetraDLL = 0;
	s_tetraInterface = NULL;
}

ueBool phTetraBuilder_BuildTetraMesh(phTetraMesh& tetraMesh, const phTetraMesh& triMesh, u32 subdivisionLevel)
{
	UE_ASSERT(s_tetraInterface);

	s_tetraInterface->setSubdivisionLevel(subdivisionLevel);

	UE_ASSERT(!triMesh.m_isTetra);
	NxTetraMesh in;
	in.mIsTetra = false;
	in.mIndices = triMesh.m_indices;
	in.mTcount = triMesh.m_numIndices / 3;
	in.mVcount = triMesh.m_numVerts;
	in.mVertices = triMesh.m_pos;

	// Note: This function seems not to be deterministic - may produce different tetra mesh each time

	NxTetraMesh out;
	if (!s_tetraInterface->createTetraMesh(in, out))
		return UE_FALSE;

	UE_ASSERT(out.mIsTetra);
	tetraMesh.m_isTetra = UE_TRUE;
	tetraMesh.m_indices = out.mIndices;
	tetraMesh.m_numIndices = out.mTcount * 4;
	tetraMesh.m_pos = out.mVertices;
	tetraMesh.m_numVerts = out.mVcount;

	phTetraBuilder_BuildTetraLinks(tetraMesh);
	phTetraBuilder_BuildTetraOuterFaces(tetraMesh);

	return UE_TRUE;
}

void phTetraBuilder_ReleaseMesh(phTetraMesh& mesh)
{
	NxTetraMesh nxMesh;
	nxMesh.mIsTetra = mesh.m_isTetra ? true : false;
	nxMesh.mIndices = mesh.m_indices;
	nxMesh.mTcount = mesh.m_numIndices / (mesh.m_isTetra ? 4 : 3);
	nxMesh.mVcount = mesh.m_numVerts;
	nxMesh.mVertices = mesh.m_pos;

	if (mesh.m_links)
		free(mesh.m_links);
	if (mesh.m_outerFaces)
		free(mesh.m_outerFaces);

	s_tetraInterface->releaseTetraMesh(nxMesh);
}