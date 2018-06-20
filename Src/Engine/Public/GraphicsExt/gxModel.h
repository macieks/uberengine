#ifndef GX_MODEL_H
#define GX_MODEL_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/ueShapes.h"
#include "GraphicsExt/gxTexture.h"

struct glStreamFormat;
struct glVertexBuffer;
struct glIndexBuffer;
struct gxSkeleton;
struct gxCollisionGeometry;
struct gxModelInstance;

enum gxModelMeshFlags
{
	gxModelMeshFlags_Color		= 0 << 1,	//!< For use by color rendering
	gxModelMeshFlags_Geometry	= 1 << 1,	//!< For use by depth/shadow/velocity rendering
};

struct gxModelMesh
{
	const char* m_name;		//!< Mesh name
	u32 m_meshFlags;		//!< Mesh type; @see gxModelMeshFlags

	ueBox m_box;			//!< Mesh bounding box

	u32 m_primType;			//!< Primitive type; @see glPrimitive

	u32 m_vertexFormatIndex;//!< Vertex format index (within lod)
	u32 m_materialIndex;	//!< Material index (within lod)

	u32 m_vertexBufferOffset;//!< Offset within vertex buffer to start of vertex data (within lod's vertex buffer)
	u32 m_baseVertex;		//!< Base vertex index
	u32 m_firstVertex;		//!< First vertex index
	u32 m_numVerts;			//!< Number of verts

	u32 m_firstIndex;		//!< First index (within lod's index buffer)
	u32 m_numIndices;		//!< Number of indices

	u32 m_numNodes;			//!< Number of bones used by the primitive
	u32* m_boneIndices;		//!< Bone indices
};

struct gxModelMaterialInfo
{
	ueResourceHandle<gxTexture> m_colorMap; // TEMP HACK: to be replaced with material handle once material system is finalized
	f32 m_mipLevelsUsage; //!< Texture mip-levels usage metric for use by texture streaming algorithms; the value is maximal ratio of UV difference to local space position difference along all edges of all triangles
};

//! Level of detail
struct gxModelLOD
{
	f32 m_quality;			//!< Quality of this level of detail; value within 0..1 range

	u32 m_numMeshes;		//!< Number of meshes
	gxModelMesh* m_meshes;	//!< Meshes

	gxCollisionGeometry* m_collisionGeometry;	//!< Optional collision geometry

	u32 m_numMaterials;		//!< Number of materials
	gxModelMaterialInfo* m_materials;//!< Materials used by the model LOD (referenced by individual mesh primitives)

	gxSkeleton* m_skeleton;	//!< Optional skeleton

	u32 m_numVertexFormats;	//!< Number of vertex formats
	glStreamFormat** m_vertexFormats; //!< An array of vertex formats

	glVertexBuffer* m_VB;	//!< Vertex buffer containing vertex data for all meshes
	glIndexBuffer* m_IB;	//!< Index buffer containing index data for all meshes
};

UE_DEFINE_RESOURCE_TYPE(gxModel, UE_BE_4BYTE_SYMBOL('m','o','d','l'));

//! Creates model instance
gxModelInstance*	gxModel_CreateInstance(gxModel* m);
//! Gets number of LODs in a model
u32					gxModel_GetNumLODs(gxModel* m);
//! Gets specific LOD in a model
gxModelLOD*			gxModel_GetLOD(gxModel* m, u32 index);
//! Gets level of detail statistics
void				gxModelLOD_GetStats(gxModelLOD* lod, u32& numVerts, u32& numIndices, u32& numTris);

void gxModelMgr_Startup(ueAllocator* allocator, const char* config);
void gxModelMgr_Shutdown();
ueAllocator* gxModelMgr_GetAllocator();

// @}

#endif // GX_MODEL_H
