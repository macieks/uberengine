#ifndef PH_LIB_H
#define PH_LIB_H

/**
 *	@defgroup ph Physics
 *	@brief Physics library with support for 2D and 3D physics. Includes binary deserialization of exported collision data.
 */

#include "Base/ueMath.h"

class ueBuffer;

/**
 *	@addtogroup ph
 *	@{
 */

#include "Physics/phStructs.h"

#if defined(PH_BULLET)
	#include "Physics/Bullet/phLib_Bullet.h"
#elif defined(PH_PHYSX)
	#include "Physics/PhysX/phLib_PhysX.h"
#endif

//! Collision callback
typedef void (*phCollisionCallback)(phCollisionEventDesc* event);

//! Debug drawing callback
class phDebugDraw
{
public:
	virtual ~phDebugDraw() {}
	//! Draws the line in 3D
	virtual void DrawLine(const ueVec3& from, const ueVec3& to, ueColor32 color) {}
	//! Draws the text at 3D location
	virtual void DrawText(const ueVec3& location, const char* textString) {}
};

//! Starts up physics library
void		phLib_Startup(phStartupParams* params);
//! Shuts down physics library
void		phLib_Shutdown();
//! Gets underlying physics library specific 32-bit symbol (to distinguish between e.g. Havok or PhysX)
u32			phLib_GetSymbol();

// Player controller

//! Creates player controller and adds it to the scene
phPlayerController*	phPlayerController_Create(phPlayerControllerDesc* desc);
//! Destroys player controller
void				phPlayerController_Destroy(phPlayerController* pc);
//! Moves player controller in desired direction
void				phPlayerController_Move(phPlayerController* pc, const ueVec3& dir, f32 dt);
//! Sets position (teleports) of the player controller
void				phPlayerController_SetPosition(phPlayerController* pc, const ueVec3& pos);
//! Gets position of the player controller
void				phPlayerController_GetPosition(phPlayerController* pc, ueVec3& pos);
//! Starts jump of the player controller (will jump only if on the ground)
void				phPlayerController_Jump(phPlayerController* pc, f32 v);
//! Updates all player controllers
void				phPlayerController_UpdateAll();

// Shape

//! Creates (reusable) shape
phShape*	phShape_Create(phShapeDesc* desc);
//! Creates (reusable) shape
phShape*	phShape_CreateInPlace(void* data, ueBuffer* tempBuffer);
//! Destroys shape
void		phShape_Destroy(phShape* shape);

// Soft body

//! Creates soft body
phSoftBody*	phSoftBody_Create(phScene* scene, phSoftBodyDesc* desc);
//! Destroys soft body
void		phSoftBody_Destroy(phSoftBody* body);
//! Applies an impulse to soft body at given positions
void		phSoftBody_ApplyImpulseAt(phSoftBody* body, const ueVec3& pos);

//! Sets user data for a body
void		phSoftBody_SetUserData(phSoftBody* body, void* userData);
//! Gets user data for a body
void*		phSoftBody_GetUserData(phSoftBody* body);

// Body

//! Creates body in the scene
phBody*	phBody_Create(phScene* scene, phBodyDesc* desc);
//! Destroys body and removes it from the scene
void	phBody_Destroy(phBody* body);

//! Tells whether body is dynamic
ueBool	phBody_IsDynamic(phBody* body);

//! Gets body transformation
void	phBody_GetTransform(phBody* body, ueMat44& transform);
//! Sets body transformation
void	phBody_SetTransform(phBody* body, const ueMat44& transform);

//! Gets body linear velocity
void	phBody_GetLinearVelocity(phBody* body, ueVec3& velocity);
//! Sets body linear velocity
void	phBody_SetLinearVelocity(phBody* body, const ueVec3& velocity);
//! Gets body speed
f32		phBody_GetSpeed(phBody* body);

//! Gets body mass
f32		phBody_GetMass(phBody* body);
//! Sets body mass
void	phBody_SetMass(phBody* body, f32 mass);

//! Gets body collision flags
u32		phBody_GetCollisionFlags(phBody* body);
//! Sets body collision flags
void	phBody_SetCollisionFlags(phBody* body, u32 flags);

//! Applies torque to body
void	phBody_ApplyTorque(phBody* body, const ueVec3& torque);
//! Applies impulse to body
void	phBody_ApplyImpulse(phBody* body, const ueVec3& impulse);

//! Sets user data for a body
void	phBody_SetUserData(phBody* body, void* userData);
//! Gets user data for a body
void*	phBody_GetUserData(phBody* body);

// Scene

//! Create physics scene
phScene*phScene_Create(const phSceneDesc* desc);
//! Destroy physics scene
void	phScene_Destroy(phScene* scene);

//! Begins update (by given delta time)
void	phScene_BeginUpdate(phScene* scene, f32 dt);
//! Ends update (waits if needed)
void	phScene_EndUpdate(phScene* scene);
//! Processes all collision
void	phScene_ProcessCollisions(phScene* scene, phCollisionCallback callback);

//! Intersects scene against shape
ueBool	phScene_IntersectShape(phScene* scene, const phShapeIntersectionTestDesc* intersectionTestDesc, phShapeIntersectionResultSet* result = NULL);
//! Intersects scene against ray
ueBool	phScene_IntersectRay(phScene* scene, const phRayIntersectionTestDesc* intersectionTestDesc, phShapeIntersectionResultSet* result = NULL);

//! Sets scene gravity
void	phScene_SetGravity(phScene* scene, const ueVec3& gravity);
//! Gets scene gravity
void	phScene_GetGravity(ueVec3& out, phScene* scene);

//! Sets debug drawing interface for a scene
void	phScene_DebugDraw(phScene* scene, phDebugDraw* debugDraw = NULL);

// @}

#endif // PH_LIB_H