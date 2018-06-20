#include "SampleApp.h"
#include "Base/ueMath.h"
#include "Base/ueRand.h"
#include "Base/Containers/ueABTree.h"
#include "Input/inSys.h"
#include "GraphicsExt/gxCamera.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxTextRenderer.h"

/**
 *	Demonstrates frustum visibility culling using ABT tree.
 */
class ueSample_VisibilityCulling : public ueSample
{
public:

	#define NUM_BOXES (1 << 13)

	ueBool Init()
	{
		m_visCullMethod = VisCullMethod_ABTree;

		// Create boxes

		const f32 minCoord = -20.0f;
		const f32 maxCoord = 20.0f;
		const f32 dimension = 0.4f;
		for (u32 i = 0; i < NUM_BOXES; i++)
		{
			TestBox* box = &m_boxes[i];

			box->m_originalBox.m_min.Set(
				ueRandG_F32(minCoord, maxCoord),
				ueRandG_F32(minCoord, maxCoord),
				ueRandG_F32(minCoord, maxCoord));
			box->m_originalBox.m_max = box->m_originalBox.m_min + dimension;

			box->m_box = box->m_originalBox;

			box->m_color = ueColor32(
				ueRandG_U32(0, 255),
				ueRandG_U32(0, 255),
				ueRandG_U32(0, 255),
				255);
		}

		// Insert boxes to ABTree

		const f32 minABTreeCoord = minCoord - 6; // Account for box animation offset
		const f32 maxABTreeCoord = maxCoord + 6;

		ueABTreeDesc treeDesc;
		treeDesc.m_stackAllocator = g_app.GetStackAllocator();
		treeDesc.m_spaceBox = ueBox(minABTreeCoord, minABTreeCoord, minABTreeCoord, maxABTreeCoord, maxABTreeCoord, maxABTreeCoord);
		treeDesc.m_maxTreeDepth = 16;
		treeDesc.m_minObjectsInNode = 8;
		treeDesc.m_maxObjectsInNode = treeDesc.m_minObjectsInNode * 2;
		treeDesc.m_maxObjects = NUM_BOXES;
		treeDesc.m_maxNodes = NUM_BOXES;

		m_abTree = ueABTree_Create(&treeDesc);
		UE_ASSERT(m_abTree);

		for (u32 i = 0; i < NUM_BOXES; i++)
			m_boxes[i].m_abTreeObjectId = ueABTree_Insert(m_abTree, m_boxes[i].m_box, &m_boxes[i]);

		m_angle = 0;

		// Create camera

		m_camera.SetLookAt(ueVec3(1.5f, 0.5f, 0), ueVec3::Zero);

		// Input

		m_inputConsumerId = inSys_RegisterConsumer("Visibility Culling sample", 0.0f);
		m_inputEvents.m_nextCullMethod = inSys_RegisterEvent(m_inputConsumerId, "Next visibility culling mode", TempPtr(inBinding(inDev_Keyboard, inKey_M)));

		return UE_TRUE;
	}

	void Deinit()
	{
		ueABTree_Destroy(m_abTree);
		inSys_UnregisterConsumer(m_inputConsumerId);
	}

	void DoFrame(f32 dt)
	{
		Update(dt);
		Render();
	}

	void Update(f32 dt)
	{
		m_stats.Reset();

		// Update camera

		if (inSys_IsConsumerActive(m_inputConsumerId))
			m_camera.UpdateFreeCamera(dt);

		if (inSys_WasPressed(m_inputEvents.m_nextCullMethod))
			m_visCullMethod = (VisCullMethod) (((u32) m_visCullMethod + 1) % VisCullMethod_Count);

		ueTimer timer;

		// Animate boxes
		{
			UE_PROF_SCOPE("Animate Boxes");

			timer.Restart();

			ueVec3 offsets[32];
			m_angle += dt;
			if (m_angle > UE_PI * 2)
				m_angle -= UE_PI * 2;
			f32 angleOffset = 0;
			for (u32 i = 0; i < UE_ARRAY_SIZE(offsets); i++)
			{
				angleOffset += 1.0f;
				const f32 sine = ueSin(m_angle + angleOffset);
				const f32 cosine = ueCos(m_angle + angleOffset);

				offsets[i].Set(sine, cosine, -sine);
				offsets[i] *= 3.0f;
			}

			for (u32 i = 0; i < NUM_BOXES; i++)
			{
				const ueVec3 offset = offsets[i & (UE_ARRAY_SIZE(offsets) - 1)];

				m_boxes[i].m_box.m_min = m_boxes[i].m_originalBox.m_min + offset;
				m_boxes[i].m_box.m_max = m_boxes[i].m_originalBox.m_max + offset;

				ueABTree_Update(m_abTree, m_boxes[i].m_abTreeObjectId, m_boxes[i].m_box);
			}
			m_stats.m_abTreeUpdateTime = timer.GetSecs();
		}

		// Determine visible boxes
		{
			m_numVisibleBoxes = 0;
			timer.Restart();

			switch (m_visCullMethod)
			{
				case VisCullMethod_None:
				{
					UE_PROF_SCOPE("Vis Culling - None");
					m_numVisibleBoxes = NUM_BOXES;
					for (u32 i = 0; i < m_numVisibleBoxes; i++)
						m_visibleBoxes[i] = &m_boxes[i];
					break;
				}
				case VisCullMethod_FrustumAll:
				{
					UE_PROF_SCOPE("Vis Culling - Brute Force Frustum");
					const ueFrustum& frustum = m_camera.GetFrustum();
					for (u32 i = 0; i < NUM_BOXES; i++)
						if (frustum.Intersect(m_boxes[i].m_box))
							m_visibleBoxes[m_numVisibleBoxes++] = &m_boxes[i];
					break;
				}

				case VisCullMethod_ABTree:
				{
					UE_PROF_SCOPE("Vis Culling - AB Tree");
					ueABTree_Cull(m_abTree, m_camera.GetFrustum(), (void**) m_visibleBoxes, NUM_BOXES, m_numVisibleBoxes);
					break;
				}
			}

			m_stats.m_cullTime = timer.GetSecs();
		}

		m_stats.m_numAll = NUM_BOXES;
		m_stats.m_numCulled = m_numVisibleBoxes;

		ueABTreeStats stats;
		ueABTree_GetStats(m_abTree, &stats);
		m_stats.m_numABTreeNodes = stats.m_numNodes;
	}

	void Render()
	{
		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;

		// Draw the boxes

		DrawBoxes(ctx);

		// Draw stats
		{
			UE_PROF_SCOPE("Render Vis Stats");

			const char* visCullMethodName = NULL;
			switch (m_visCullMethod)
			{
				case VisCullMethod_None: visCullMethodName = "none"; break;
				case VisCullMethod_FrustumAll: visCullMethodName = "frustum all"; break;
				case VisCullMethod_ABTree: visCullMethodName = "AB tree"; break;
			}

#if defined(UE_AB_TREE_GATHER_QUERY_STATS)
			const ueABTreeQueryStats* queryStats = ueABTree_GetLastQueryStats(m_abTree);
#endif

			char statsTextBuffer[512];
			ueStrFormat(statsTextBuffer, UE_ARRAY_SIZE(statsTextBuffer),
				"FPS %.3f\n"
				"Culled boxes %.1f %% (%d / %d)\n"
				"Cull method: %s [M]\n"
				"Cull time: %.4f ms\n"
				"AB-Tree only:\n"
				"  update time: %.4f ms\n"
				"  nodes: %d\n"
				"  nodes visited: %d\n"
				"  frustum tests: %d\n"
				"  frustum triv tests: %d",
				1.0f / g_app.GetGameTimer().DT(),
				((f32) m_stats.m_numCulled / m_stats.m_numAll) * 100.0f, m_stats.m_numCulled, m_stats.m_numAll,
				visCullMethodName,
				m_stats.m_cullTime * 1000.0f,
				m_stats.m_abTreeUpdateTime * 1000.0f,
				m_stats.m_numABTreeNodes,
#if defined(UE_AB_TREE_GATHER_QUERY_STATS)
				queryStats->m_numNodesVisited,
				queryStats->m_numFrustumTests,
				queryStats->m_numTriviallyAcceptedFrustumTests);
#else
				-1, -1, -1);
#endif

			gxText text;
			text.m_x = 150;
			text.m_y = 100;
			text.m_utf8Buffer = statsTextBuffer;
			gxTextRenderer_Draw(ctx, &text);
		}

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	void DrawBoxes(glCtx* ctx)
	{
		UE_PROF_SCOPE("Render Boxes");

		gxShapeDrawParams drawParams;
		drawParams.m_viewProj = &m_camera.GetViewProj();
		gxShapeDraw_SetDrawParams(&drawParams);
		gxShapeDraw_Begin(ctx);

		gxShape_Box drawBox;
		for (u32 i = 0; i < m_numVisibleBoxes; i++)
		{
			drawBox.m_color = m_visibleBoxes[i]->m_color;
			drawBox.m_box = m_visibleBoxes[i]->m_box;
			gxShapeDraw_DrawBox(drawBox);
		}

		// Draw ab-tree bbox

		const ueABTreeDesc* treeDesc = ueABTree_GetDesc(m_abTree);

		drawBox.m_color = ueColor32::White;
		drawBox.m_box = treeDesc->m_spaceBox;
		gxShapeDraw_DrawBox(drawBox);

		drawBox.m_wireFrame = UE_FALSE;
		drawBox.m_color = ueColor32(0, 200, 0, 30);
		drawBox.m_box = treeDesc->m_spaceBox;
		gxShapeDraw_DrawBox(drawBox);

		gxShapeDraw_End();
	}

private:

	enum VisCullMethod
	{
		VisCullMethod_None = 0,
		VisCullMethod_FrustumAll,
		VisCullMethod_ABTree,

		VisCullMethod_Count
	};

	VisCullMethod m_visCullMethod;

	struct TestBox
	{
		ueBox m_box;
		ueColor32 m_color;
		u32 m_abTreeObjectId;

		ueBox m_originalBox;
	};

	TestBox m_boxes[NUM_BOXES];
	f32 m_angle;

	u32 m_numVisibleBoxes;
	TestBox* m_visibleBoxes[NUM_BOXES];

	ueABTree* m_abTree;

	gxCamera m_camera;

	inConsumerId m_inputConsumerId;
	struct InputEvents
	{
		inEventId m_nextCullMethod;
	} m_inputEvents;

	struct Stats
	{
		u32 m_numAll;
		u32 m_numCulled;
		f32 m_cullTime;
		u32 m_numABTreeNodes;
		f32 m_abTreeUpdateTime;

		void Reset()
		{
			m_numAll = 0;
			m_numCulled = 0;
			m_cullTime = 0.0f;
			m_numABTreeNodes = 0;
			m_abTreeUpdateTime = 0.0f;
		}
	};

	Stats m_stats;

};

UE_DECLARE_SAMPLE(ueSample_VisibilityCulling, "VisibilityCulling")
