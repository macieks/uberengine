#include "Input/inMouse.h"
#include "Base/ueWindow.h"
#include "Graphics/glLib.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxAnimationInstance.h"
#include "GraphicsExt/gxModelInstance.h"
#include "GraphicsExt/gxCamera.h"
#include "World.h"
#include "Terrain.h"
#include "Sample_RTS.h"
#include "SampleApp.h"
#include "Cursor.h"

void World::Init(ueAllocator* allocator)
{
	UE_ASSERT(ueIsPow2(g_terrain->GetDim()));

	// Allocate and initialize fields

	m_allocator = allocator;
	m_dim = g_terrain->GetDim() - 1;
	m_cellSize = g_terrain->GetCellSize();
	m_fields = (Field*) m_allocator->Alloc(sizeof(Field) * m_dim * m_dim);
	UE_ASSERT(m_fields);

	ueMemSet(m_fields, 0, sizeof(Field) * m_dim * m_dim);
	for (u32 x = 0; x < m_dim; x++)
		for (u32 z = 0; z < m_dim; z++)
		{
			Field* field = &m_fields[x * m_dim + z];
			field->m_flags = g_terrain->GetHeight(Point(x, z)) < 0.5f ? Field::Flags_Passable : 0;
			field->m_entity = NULL;
			field->m_pos = Point(x, z);
		}

	// Load soldier model

	m_soldierModel.SetByName("rts_sample/soldier");
	m_soldierAnimation.SetByName("rts_sample/walk_anim");

	m_soldierTexture.SetByName("rts_sample/soldiermap0");

	m_soldierProgram.Create("common/model_vs", "common/model_fs");
	m_soldierSkinnedProgram.Create("common/model_skinned_vs", "common/model_fs");

	m_isSelecting = UE_FALSE;
}

void World::Deinit()
{
	Soldier* soldier = m_soldiers.Front();
	while (soldier)
	{
		Soldier* next = soldier->Next();
		m_soldiers.Remove(soldier);
		ueDelete(soldier, m_allocator);
		soldier = next;
	}

	m_soldierModel = NULL;
	m_soldierAnimation = NULL;

	m_allocator->Free(m_fields);
	m_fields = NULL;
	m_allocator = NULL;
}

void World::ResetForPathfinding()
{
	for (u32 i = 0; i < m_dim * m_dim; i++)
		m_fields[i].ResetPathFinding();
}

Field* World::FindNearestEmptyField(Point p)
{
	s32 xp, zp;

	s32 dist = 0;
	while (1)
	{
		xp = p.x + dist;
		zp = p.y + dist;

		if (IsEmpty(Point(xp, zp))) break;
		
		xp = p.x;
		while (xp < p.x + dist)
			if (IsEmpty(Point(xp, zp))) break;
			else xp++;
		if (xp < p.x + dist) break;

		zp = p.y;
		while (zp < p.y + dist)
			if (IsEmpty(Point(xp, zp))) break;
			else zp++;
		if (zp < p.y + dist) break;

		dist++;
		UE_ASSERT(dist < (s32) m_dim);
	}

	return GetFieldAt(Point(xp, zp));
}

void World::AddUnit(u32 soldierCount, Point pos, ueColor32 color)
{
	for (u32 i = 0; i < soldierCount; i++)
	{
		Field* spawnField = FindNearestEmptyField(pos);

		Soldier* soldier = new(m_allocator) Soldier();
		soldier->SetColor(color);
		soldier->MoveTo(spawnField);

		soldier->m_modelInstance = gxModel_CreateInstance(*m_soldierModel);
		UE_ASSERT(soldier->m_modelInstance);

		gxModelInstance_PlayAnimation(soldier->m_modelInstance, *m_soldierAnimation);

		m_soldiers.PushFront(soldier);

#if 0
		Field* targetField = GetFieldAt(Point(40, 14));
		soldier->WalkTo(targetField);
#endif
	}
}

void World::Update(f32 dt)
{
	UE_PROF_SCOPE("World::Update");

	// Select units

	if (inMouse_WasPressed(inMouseButton_Left))
	{
		m_isSelecting = UE_TRUE;
		Cursor::GetClipPos(m_selectionStart);
		m_selectionEnd = m_selectionStart;
	}
	else if (m_isSelecting)
	{
		Cursor::GetClipPos(m_selectionEnd);

		if (inMouse_WasReleased(inMouseButton_Left))
		{
			ueVec2 min = m_selectionStart;
			ueVec2 max = m_selectionEnd;
			if (min[0] > max[0])
			{
				const f32 temp = min[0];
				min[0] = max[0];
				max[0] = temp;
			}
			if (min[1] > max[1])
			{
				const f32 temp = min[1];
				min[1] = max[1];
				max[1] = temp;
			}

			Soldier* soldier = m_soldiers.Front();
			while (soldier)
			{
				ueVec3 posProj;
				soldier->GetWorldPos(posProj);
				g_camera->GetViewProj().TransformCoord(posProj);

				soldier->SetSelected(
					min[0] <= posProj[0] && posProj[0] <= max[0] &&
					min[1] <= -posProj[1] && -posProj[1] <= max[1]);

				soldier = soldier->Next();
			}

			m_isSelecting = UE_FALSE;
		}
	}

	// Order selected units to move

	Point overPoint;
	if (inMouse_WasPressed(inMouseButton_Right) && g_terrain->GetOverField(overPoint))
	{
		Field* f = GetFieldAt(overPoint);
		if (f)
		{
			Soldier* soldier = m_soldiers.Front();
			while (soldier)
			{
				if (soldier->IsSelected())
					soldier->WalkTo(f);
				soldier = soldier->Next();
			}
		}
	}

	// Update units

	Soldier* soldier = m_soldiers.Front();
	while (soldier)
	{
		soldier->Update(dt);
		soldier = soldier->Next();
	}
}

void World::Draw(glCtx* ctx)
{
	UE_PROF_SCOPE("World::Draw");

	gxShapeDrawParams drawParams;
	drawParams.m_viewProj = &g_camera->GetViewProj();
	gxShapeDraw_SetDrawParams(&drawParams);
	gxShapeDraw_Begin(ctx);

	// Soldiers

	Soldier* soldier = m_soldiers.Front();
	while (soldier)
	{
		soldier->Draw(ctx);
		soldier = soldier->Next();
	}

	// Over-field

	Point overPoint;
	if (g_terrain->GetOverField(overPoint))
	{
		gxShape_Line line;
		line.m_color = (GetFieldAt(overPoint)->m_flags & Field::Flags_Passable) ? ueColor32::Green : ueColor32::Red;

		const ueVec3 offset(0, 0.1f, 0);

		g_terrain->ToWorldPos(line.m_a, overPoint);
		line.m_a += offset;
		g_terrain->ToWorldPos(line.m_b, overPoint + Point(1, 0));
		line.m_b += offset;
		gxShapeDraw_DrawLine(line);

		line.m_a = line.m_b;
		g_terrain->ToWorldPos(line.m_b, overPoint + Point(1, 1));
		line.m_b += offset;
		gxShapeDraw_DrawLine(line);

		line.m_a = line.m_b;
		g_terrain->ToWorldPos(line.m_b, overPoint + Point(0, 1));
		line.m_b += offset;
		gxShapeDraw_DrawLine(line);

		line.m_a = line.m_b;
		g_terrain->ToWorldPos(line.m_b, overPoint);
		line.m_b += offset;
		gxShapeDraw_DrawLine(line);
	}

	// Selection

	if (m_isSelecting)
	{
		ueVec2 start, end;
		Cursor::ConvertClipTo01Pos(start, m_selectionStart);
		Cursor::ConvertClipTo01Pos(end, m_selectionEnd);

		gxShape_Rect rect;
		rect.m_rect.Set(start, end);
		rect.m_color = ueColor32(0, 0, 255, 40);
		rect.m_wireFrame = UE_FALSE;
		gxShapeDraw_DrawRect(rect);

		rect.m_color = ueColor32(0, 0, 255, 200);
		rect.m_wireFrame = UE_TRUE;
		gxShapeDraw_DrawRect(rect);
	}

	// Cursor
	{
		gxShape_Triangle2D triangle;
		Cursor::Get01Pos(triangle.m_a);

		triangle.m_b = ueVec2(0.05f, 0.01f) + triangle.m_a;
		triangle.m_c = ueVec2(0.01f, 0.05f) + triangle.m_a;

		triangle.m_color = ueColor32(0, 255, 0, 250);
		triangle.m_wireFrame = UE_TRUE;
		gxShapeDraw_DrawTriangle(triangle);
	}

	gxShapeDraw_End();
}

void World::DrawModelInstance(glCtx* ctx, gxModelInstance* modelInstance)
{
	const u32 lodIndex = 0;
	gxModel* model = gxModelInstance_GetModel(modelInstance);
	gxModelLOD* lod = gxModel_GetLOD(model, lodIndex);

	glCtx_SetFillMode(ctx, glFillMode_Solid);
	glCtx_SetBlending(ctx, UE_FALSE);
	glCtx_SetCullMode(ctx, glCullMode_CCW);
	glCtx_SetDepthWrite(ctx, UE_TRUE);
	glCtx_SetDepthTest(ctx, UE_TRUE);
	glCtx_SetIndices(ctx, lod->m_IB);

	glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::ViewProj, &g_camera->GetViewProj());
	glCtx_SetSamplerConstant(ctx, gxCommonConstants::ColorMap, gxTexture_GetBuffer(*m_soldierTexture));

	for (u32 i = 0; i < lod->m_numMeshes; i++)
	{
		gxModelMesh* mesh = &lod->m_meshes[i];
		const ueBool isSkinned = mesh->m_numNodes >= 2;

		glCtx_SetStream(ctx, 0, lod->m_VB, lod->m_vertexFormats[mesh->m_vertexFormatIndex], mesh->m_vertexBufferOffset);
		glCtx_SetProgram(ctx, (isSkinned ? m_soldierSkinnedProgram : m_soldierProgram).GetProgram());

		if (!isSkinned)
		{
			const ueMat44* transform =
				(mesh->m_numNodes == 1) ? // Rigid skinned?
				&gxModelInstance_GetNodeTransforms(modelInstance, lodIndex)[mesh->m_boneIndices[0]] :
				gxModelInstance_GetBaseTransform(modelInstance);

			glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::World, transform);
		}
		else
		{
			ueMat44 nodeTransforms[64];
			UE_ASSERT(mesh->m_numNodes <= UE_ARRAY_SIZE(nodeTransforms));
			gxModelInstance_GetSkinningNodeTransforms(modelInstance, lodIndex, mesh->m_numNodes, mesh->m_boneIndices, nodeTransforms);
			glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::BoneTransforms, nodeTransforms, mesh->m_numNodes);
		}

		glCtx_DrawIndexed(ctx, (glPrimitive) mesh->m_primType, mesh->m_baseVertex, mesh->m_firstVertex, mesh->m_numVerts, mesh->m_firstIndex, mesh->m_numIndices);
	}
}
