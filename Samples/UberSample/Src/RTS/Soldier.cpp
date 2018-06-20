#include "Base/ueRand.h"
#include "Graphics/glLib.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxModelInstance.h"
#include "GraphicsExt/gxCamera.h"
#include "Soldier.h"
#include "Field.h"
#include "World.h"
#include "PathFinder.h"
#include "Path.h"
#include "Sample_RTS.h"

Soldier::Soldier() :
	Entity(Type_Soldier),
	m_field(NULL),
	m_target(NULL),
	m_isSelected(UE_TRUE),
	m_targetRotationY(0.0f),
	m_rotationY(0.0f)
{
	m_walk.m_path = NULL;
	m_walk.m_prevField = NULL;
	m_walk.m_blockedTime = 0.0f;
	m_walk.m_speed = 7.0f;
}

Soldier::~Soldier()
{
	MoveTo(NULL);
	gxModelInstance_Destroy(m_modelInstance);
}

void Soldier::MoveTo(Field* field)
{
	m_walk.m_prevField = NULL;
	
	if (m_field)
		m_field->m_entity = NULL;
	m_field = field;
	if (m_field)
	{
		UE_ASSERT(!m_field->m_entity);
		m_field->m_entity = this;
	}

	WalkTo(NULL);
}

void Soldier::WalkTo(Field* target)
{
	if (target == m_target)
		return;

	if (m_walk.m_path)
	{
		g_pathFinder->DestroyPath(m_walk.m_path);
		m_walk.m_path = NULL;
	}

	m_target = target;
	if (m_target)
		RecalcPath();
}

ueBool Soldier::RecalcPath()
{
	UE_ASSERT(m_target);

	if (m_walk.m_path)
		g_pathFinder->DestroyPath(m_walk.m_path);

	m_walk.m_path = g_pathFinder->AStar(m_field, m_target);

	if (m_walk.m_path)
	{
		m_walk.m_pointIndex = 0;
		m_walk.m_lineProgress = 1.0f;
		m_walk.m_lineLength = 1.0f;
		return UE_TRUE;
	}

	return UE_FALSE;
}

void Soldier::SetNextWalkField(Field* field)
{
	m_walk.m_prevField = m_field;

	if (m_field)
		m_field->m_entity = NULL;
	m_field = field;
	if (m_field)
	{
		UE_ASSERT(!m_field->m_entity);
		m_field->m_entity = this;
	}
}

void Soldier::WalkToRandomTarget()
{
	ueRand* rand = ueRand_GetGlobal();
	while (1)
	{
		Point targetPos = Point(ueRand_U32(rand, g_world->GetDim() - 1), ueRand_U32(rand, g_world->GetDim() - 1));
		Field* target = g_world->GetEmptyFieldAt(targetPos);
		if (target)
		{
			WalkTo(target);
			if (m_walk.m_path)
				break;
		}
	}
}

void Soldier::Update(f32 dt)
{
	UE_PROF_SCOPE("Soldier::Update");

	gxModelInstance_Update(m_modelInstance, dt);

	f32 rotDist = ueMod(m_targetRotationY - m_rotationY, UE_2PI);
	if (rotDist > UE_PI)
		rotDist = -rotDist;
	m_rotationY += rotDist * 0.1f;

	if (m_target)
	{
		if (!m_walk.m_path)
		{
			m_walk.m_blockedTime += dt;

			if (m_walk.m_blockedTime < 2.0f /* or field which was blocked is now unblocked */)
				return;

			if (!RecalcPath())
			{
				m_walk.m_blockedTime = 0.0f;
				return;
			}
		}

		const f32 newLineProgress = m_walk.m_lineProgress + dt * m_walk.m_speed;

		// Just progress on current path segment
		if (newLineProgress < m_walk.m_lineLength)
		{
			m_walk.m_lineProgress = newLineProgress;
			m_walk.m_blockedTime = 0.0f;
		}

		// Next path segment
		else
		{
			// Finished path?
			if (m_walk.m_pointIndex + 1 == m_walk.m_path->m_numPoints)
			{
				// Path wasn't leading to target?
				if (m_target != m_field)
				{
					RecalcPath();
					return;
				}

				// Arrived at destination

				g_pathFinder->DestroyPath(m_walk.m_path);
				m_walk.m_path = NULL;
				m_target = NULL;

				return;
			}

			Field* newField = g_world->GetFieldAt(m_walk.m_path->m_points[m_walk.m_pointIndex + 1]);

			// Can enter next field?
			if (newField->IsEmpty())
			{
				SetNextWalkField(newField);
				m_walk.m_lineProgress = newLineProgress - m_walk.m_lineLength;
				m_walk.m_lineLength = Point::Dist(m_field->m_pos, m_walk.m_prevField->m_pos);
				m_walk.m_pointIndex++;
				m_walk.m_blockedTime = 0.0f;

				ueVec2 dir;
				(m_field->m_pos - m_walk.m_prevField->m_pos).AsVec2(dir);
				m_targetRotationY = ueATan2(dir[0], dir[1]);
			}
			else
			{
				m_walk.m_blockedTime += dt;
				RecalcPath();
			}
		}
	}
}

void Soldier::GetWorldPos(ueVec3& pos) const
{
	g_terrain->ToWorldPos(pos, m_field->m_pos);

	if (m_walk.m_path && m_walk.m_prevField)
	{
		ueVec3 prevPos;
		g_terrain->ToWorldPos(prevPos, m_walk.m_prevField->m_pos);
		const f32 lerpScale = m_walk.m_lineProgress / m_walk.m_lineLength;
		ueVec3::Lerp(pos, prevPos, pos, lerpScale);
	}
}

void Soldier::Draw(glCtx* ctx)
{
	UE_PROF_SCOPE("Soldier::Draw");

	ueVec3 pos;
	GetWorldPos(pos);

	if (m_walk.m_path)
		m_walk.m_path->Draw(5);

	ueMat44 t;
	t.SetScale(0.5f);
	t.Rotate(ueVec3(0, 1, 0), m_rotationY + UE_PI);
	t.Translate(pos + ueVec3(0, 1, 0));

	gxModelInstance_SetBaseTransform(m_modelInstance, t);

	g_world->DrawModelInstance(ctx, m_modelInstance);

	if (m_isSelected)
	{
		gxShape_Line line;
		line.m_a = pos;
		line.m_b = pos + ueVec3(0, 3, 0);
		line.m_color = m_color;
		gxShapeDraw_DrawLine(line);
	}
}