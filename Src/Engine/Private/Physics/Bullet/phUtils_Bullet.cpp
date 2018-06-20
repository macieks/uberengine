#include "Base/ueBase.h"

#if defined(PH_BULLET)

#include "Physics/Bullet/phLib_Bullet_Private.h"

/*----------------------------- phBulletConverter ---------------------------*/

btVector3 phBulletConverter::ToBulletVec3(const ueVec3& v)
{
	return btVector3(ueVec3_GetX(v), ueVec3_GetY(v), ueVec3_GetZ(v));
}

ueVec3 phBulletConverter::FromBulletVec3(const btVector3& v)
{
	return ueVec3(v.getX(), v.getY(), v.getZ());
}

btQuaternion phBulletConverter::ToBulletQuaternion(const ueQuat& q)
{
	return btQuaternion(ueQuat_GetX(q), ueQuat_GetY(q), ueQuat_GetZ(q), ueQuat_GetW(q));
}

ueQuat phBulletConverter::FromBulletQuaternion(const btQuaternion& q)
{
	ueQuat dst;
	ueQuat_Set(dst, q.getX(), q.getY(), q.getZ(), q.getW());
	return dst;
}

void phBulletConverter::ToBulletTransform(const ueMat44& src, btTransform& dst)
{
	dst.getBasis().setValue(
		ueMat44_GetElem(src, 0, 0), ueMat44_GetElem(src, 1, 0), ueMat44_GetElem(src, 2, 0),
		ueMat44_GetElem(src, 0, 1), ueMat44_GetElem(src, 1, 1), ueMat44_GetElem(src, 2, 1),
		ueMat44_GetElem(src, 0, 2), ueMat44_GetElem(src, 1, 2), ueMat44_GetElem(src, 2, 2));

	ueVec3 translation;
	ueMat44_GetTranslation(translation, src);
	dst.setOrigin(ToBulletVec3(translation));
}

void phBulletConverter::FromBulletTransform(ueMat44& dst, const btTransform& src)
{
	const btVector3& col0 = src.getBasis().getColumn(0);
	const btVector3& col1 = src.getBasis().getColumn(1);
	const btVector3& col2 = src.getBasis().getColumn(2);
	const btVector3& translation = src.getOrigin();

	ueMat44_SetColumn(dst, 0, col0.getX(), col0.getY(), col0.getZ(), 0);
	ueMat44_SetColumn(dst, 1, col1.getX(), col1.getY(), col1.getZ(), 0);
	ueMat44_SetColumn(dst, 2, col2.getX(), col2.getY(), col2.getZ(), 0);
	ueMat44_SetColumn(dst, 3, translation.getX(), translation.getY(), translation.getZ(), 1);
}

/*----------------------------- phBulletDebugDraw ---------------------------*/

void phBulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	static const f32 oneOver255 = 1.0f / 255.0f;
	m_debugDraw->DrawLine(phBulletConverter::FromBulletVec3(from), phBulletConverter::FromBulletVec3(to), ueColor32::F(color.getX() * oneOver255, color.getY() * oneOver255, color.getZ() * oneOver255, 1.0f));
}

void phBulletDebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	// TODO
}

void phBulletDebugDraw::draw3dText(const btVector3& location, const char* textString)
{
	m_debugDraw->DrawText(phBulletConverter::FromBulletVec3(location), textString);
}

void phBulletDebugDraw::reportErrorWarning(const char* warningString) {}

void phBulletDebugDraw::setDebugMode(int debugMode) { m_debugMode = debugMode; }
int phBulletDebugDraw::getDebugMode() const { return m_debugMode; }

#else // defined(PH_BULLET)
	UE_NO_EMPTY_FILE
#endif