#pragma once

#include "Base/ueMath.h"
#include "Base/Containers/ueList.h"

#include "Point.h"
#include "Entity.h"

struct gxModelInstance;
struct glCtx;
class gxCamera;
struct Path;
struct Field;

class Soldier : public Entity, public ueList<Soldier>::Node
{
public:
	Soldier();
	~Soldier();

	UE_INLINE ueBool IsSelected() const { return m_isSelected; }
	UE_INLINE void SetSelected(ueBool isSelected) { m_isSelected = isSelected; }
	UE_INLINE void SetColor(ueColor32 color) { m_color = color; }
	UE_INLINE f32 GetRotationY() const { return m_rotationY; }

	void GetWorldPos(ueVec3& out) const;

	void MoveTo(Field* target);
	void WalkTo(Field* target);
	void WalkToRandomTarget();

	void Update(f32 dt);
	void Draw(glCtx* ctx);

private:

	ueBool RecalcPath();
	void SetNextWalkField(Field* field);

	Field* m_field;
	Field* m_target;

	ueColor32 m_color;

	f32 m_targetRotationY;
	f32 m_rotationY;

	ueBool m_isSelected;

	struct Walk
	{
		Field* m_prevField;

		f32 m_speed;

		u32 m_pointIndex;
		f32 m_lineProgress;
		f32 m_lineLength;
		Path* m_path;

		f32 m_blockedTime;
	} m_walk;

	gxModelInstance* m_modelInstance;

	friend class World;
};