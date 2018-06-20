#ifndef GX_COLLISION_H
#define GX_COLLISION_H

/**
 *	@addtogroup gx
 *	@{
 */

struct phBody;
struct phShape;
struct phShapeDesc;
struct phScene;
struct gxCollisionGeometryInstance;

struct gxCollisionGeometry_InitData
{
	u32 m_symbol;		//!< Physics library symbol

	u32 m_bufferSize;	//!< Physics library specific buffer size
	void* m_buffer;		//!< Physics library specific buffer
};

//! Collision geometry (used by model @see gxModel)
struct gxCollisionGeometry
{
	//! Collision geometry part
	struct Part
	{
		u32 m_nodeIndex;			//!< Node index within gxModel's LOD; U32_MAX indicates this part is not linked to any node
		ueMat44 m_localTransform;	//!< Transform relative to node or model instance's transform
		phShape* m_shape;			//!< Part's shape
	};

	u32 m_numParts;					//!< Number of parts
	Part* m_parts;					//!< An array of parts

	void Init(gxCollisionGeometry_InitData* initData);
	void Deinit();

	u32 GetInstanceSize() const;
	gxCollisionGeometryInstance* CreateInstanceInPlace(void* buffer, u32 bufferSize);
	void DestroyInstanceInPlace(gxCollisionGeometryInstance* inst);
};

//! Collision geometry instance (used by model instance @see gxModelInstance)
struct gxCollisionGeometryInstance
{
	phScene* m_scene;					//!< Scene where geometry instance was instanciated
	phBody** m_bodies;					//!< Bodies created for this geometry instance
	gxCollisionGeometry* m_geometry;	//!< Referenced geometry

	void AddToScene(phScene* scene);
	void RemoveFromScene();
};

// @}

#endif // GX_COLLISION_H
