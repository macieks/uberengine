#ifndef GX_CAMERA_H
#define GX_CAMERA_H

#include "Base/ueShapes.h"

/**
 *	@addtogroup gx
 *	@{
 */

//! Multi-purpose simple 3D camera
class gxCamera
{
public:
	//! RTS like camera parmeters
	struct RTSCameraParams
	{
		f32 m_focusHeight;	//!< Height at which camera focuses
		f32 m_minDistance;	//!< Min. camera distance from focal point
		f32 m_maxDistance;	//!< Max. camera distance from focal point
		f32 m_minPitch;		//!< Min. pitch camera rotation
		f32 m_maxPitch;		//!< Max. pitch camera rotation

		RTSCameraParams() :
			m_focusHeight(0),
			m_minDistance(5.0f),
			m_maxDistance(15.0f),
			m_minPitch(UE_PI * 0.35f),
			m_maxPitch(UE_PI * 0.45f)
		{}
	};

	gxCamera();

	//! Updates free 3D camera
	void UpdateFreeCamera(f32 dt);
	//! Updates RTS style camera
	void UpdateRTSCamera(f32 dt, const RTSCameraParams& params);
	//! Updates editor mode camera
	void UpdateEditorCamera(f32 dt, f32 focusDistance);

	//! Rotates camera around X axis
	void RotateX(f32 angle);
	//! Rotates camera around Y axis
	void RotateY(f32 angle);
	//! Rotates camera around Z axis
	void RotateZ(f32 angle);

	//! Notifies camera we'll now be drawing to texture; this is required for the projection matrix to account for Y inversion (this is needed due to difference between OpenGL and DirectX)
	void SetDrawingToTexture(ueBool enable);

	//! Sets aspect ratio
	void SetAspectRatio(f32 aspectRatio, f32 refFov = ueDegToRad(100.0f), f32 refAspectRatio = 16.0f / 9.0f);
	//! Sets up camera to look from to
	void SetLookAt(const ueVec3& eyePos, const ueVec3& atPos);
	//! Sets near Z coordinate
	void SetNearZ(f32 nearZ);
	//! Sets far Z coordinate
	void SetFarZ(f32 farZ);

	//! Gets near Z coordinate
	UE_INLINE f32 GetNearZ() const { return m_nearZ; }
	//! Gets far Z coordinate
	UE_INLINE f32 GetFarZ() const { return m_farZ; }
	//! Gets field of view
	UE_INLINE f32 GetFOV() const { return m_FOV; }
	//! Gets aspect ratio
	UE_INLINE f32 GetAspect() const { return m_aspect; }

	//! Gets viewer position
	UE_INLINE const ueVec3& GetPosition() const { return m_position; }
	//! Gets viewer facing direction
	UE_INLINE const ueVec3& GetViewDir() const { return m_viewDir; }

	//! Gets view matrix
	UE_INLINE const ueMat44& GetView() const { return m_view; }
	//! Gets inversed view matrix
	UE_INLINE const ueMat44& GetViewI() const { return m_viewI; }
	//! Gets projection matrix
	UE_INLINE const ueMat44& GetProj() const { return m_proj; }
	//! Gets inversed projection matrix
	UE_INLINE const ueMat44& GetProjI() const { return m_projI; }
	//! Gets view-projection matrix
	UE_INLINE const ueMat44& GetViewProj() const { return m_viewProj; }
	//! Gets inversed view-projection matrix
	UE_INLINE const ueMat44& GetViewProjI() const { return m_viewProjI; }
	//! Gets frustum
	UE_INLINE const ueFrustum& GetFrustum() const { return m_frustum; }

protected:
	void CalcMatrices();

	// Position and viewing angles

	ueVec3 m_position;

	f32 m_yaw;		//!< Angle around Y axis
	f32 m_pitch;	//!< Angle around X axis
	f32 m_roll;		//!< Angle around Z axis

	// Projection parameters

	f32 m_nearZ;
	f32 m_farZ;
	f32 m_aspect;
	f32 m_FOV;

	// Matrices and frustum

	ueBool m_drawingToTexture; // This tells us whether to invert Y axis in projection matrix in OpenGL (hence OpenGL vs DirectX left-top/bottom texture coordinate is different)

	ueMat44 m_view;
	ueMat44 m_viewI;
	ueMat44 m_proj;
	ueMat44 m_projI;
	ueMat44 m_viewProjI;
	ueMat44 m_viewProj;
	ueVec3 m_viewDir;

	ueFrustum m_frustum;

	// Constant camera parameters

	f32 m_movementSpeed;
	f32 m_rotationSpeed;
};

// @}

#endif // GX_CAMERA_H
