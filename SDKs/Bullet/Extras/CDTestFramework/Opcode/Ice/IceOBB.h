/*
 *	ICE / OPCODE - Optimized Collision Detection
 * http://www.codercorner.com/Opcode.htm
 * 
 * Copyright (c) 2001-2008 Pierre Terdiman,  pierre@codercorner.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the uethors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains OBB-related code. (oriented bounding box)
 *	\file		IceOBB.h
 *	\uethor		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEOBB_H__
#define __ICEOBB_H__

	// Forward declarations
	class LSS;

	class ICEMATHS_API OBB
	{
		public:
		//! Constructor
		inline_						OBB()		{}
		//! Constructor
		inline_						OBB(const Point& center, const Point& extents, const Matrix3x3& rot) : mCenter(center), mExtents(extents), mRot(rot)	{}
		//! Destructor
		inline_						~OBB()		{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups an empty OBB.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void				SetEmpty()
									{
										mCenter.Zero();
										mExtents.Set(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);
										mRot.Identity();
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Tests if a point is contained within the OBB.
		 *	\param		p	[in] the world point to test
		 *	\return		true if inside the OBB
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				ContainsPoint(const Point& p)	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Builds an OBB from an AABB and a world transform.
		 *	\param		aabb	[in] the aabb
		 *	\param		mat		[in] the world transform
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void				Create(const AABB& aabb, const Matrix4x4& mat);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Recomputes the OBB after an arbitrary transform by a 4x4 matrix.
		 *	\param		mtx		[in] the transform matrix
		 *	\param		obb		[out] the transformed OBB
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	void				Rotate(const Matrix4x4& mtx, OBB& obb)	const
									{
										// The extents remain constant
										obb.mExtents = mExtents;
										// The center gets x-formed
										obb.mCenter = mCenter * mtx;
										// Combine rotations
										obb.mRot = mRot * Matrix3x3(mtx);
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Checks the OBB is valid.
		 *	\return		true if the box is valid
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	BOOL				IsValid()	const
									{
										// Consistency condition for (Center, Extents) boxes: Extents >= 0.0f
										if(mExtents.x < 0.0f)	return FALSE;
										if(mExtents.y < 0.0f)	return FALSE;
										if(mExtents.z < 0.0f)	return FALSE;
										return TRUE;
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the obb planes.
		 *	\param		planes	[out] 6 box planes
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				ComputePlanes(Plane* planes)	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the obb points.
		 *	\param		pts	[out] 8 box points
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				ComputePoints(Point* pts)	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes vertex normals.
		 *	\param		pts	[out] 8 box points
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				ComputeVertexNormals(Point* pts)	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Returns edges.
		 *	\return		24 indices (12 edges) indexing the list returned by ComputePoints()
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				const udword*		GetEdges()	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Returns local edge normals.
		 *	\return		edge normals in local space
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				const Point*		GetLocalEdgeNormals()	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Returns world edge normal
		 *	\param		edge_index		[in] 0 <= edge index < 12
		 *	\param		world_normal	[out] edge normal in world space
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void				ComputeWorldEdgeNormal(udword edge_index, Point& world_normal)	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes an LSS surrounding the OBB.
		 *	\param		lss		[out] the LSS
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void				ComputeLSS(LSS& lss)	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Checks the OBB is inside another OBB.
		 *	\param		box		[in] the other OBB
		 *	\return		TRUE if we're inside the other box
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				BOOL				IsInside(const OBB& box)	const;

		inline_	const Point&		GetCenter()		const	{ return mCenter;	}
		inline_	const Point&		GetExtents()	const	{ return mExtents;	}
		inline_	const Matrix3x3&	GetRot()		const	{ return mRot;		}

		inline_	void				GetRotatedExtents(Matrix3x3& extents)	const
									{
										extents = mRot;
										extents.Scale(mExtents);
									}

				Point				mCenter;		//!< B for Box
				Point				mExtents;		//!< B for Bounding
				Matrix3x3			mRot;			//!< O for Oriented

				// Orientation is stored in row-major format,
				// i.e. rows = eigen vectors of the covariance matrix
	};

#endif	// __ICEOBB_H__