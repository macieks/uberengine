#include "Gameplay/gxRenderer.h"

void gxRenderer::RenderScene(gaScene* scene, gaCamera* camera, gxRenderResult& result, gxRenderResult& rightEyeResult)
{
	m_scene = scene;
	m_frameIndex++;

	switch (m_stereo.m_mode)
	{
		case StereoMode_None:
			RenderScene(camera->GetView(), camera->GetProj(), result, 0);
			break;

		case StereoMode_SeparateColors:
		{
			const ueVec3& rightVec = camera->GetView().GetRightVec();
			ueMat44 view = m_camera.GetView();

			// Draw left eye

			ueVec3 offset = rightVec * -m_stereo.m_eyeOffset;
			view.Translate(offset);

			glCtx_SetColorWrite(ctx, 0, m_stereo.m_leftEyeColorMask);

			RenderScene(view, camera->GetProj(), result, 0);

			// Draw right eye

			offset = rightVec * 2.0f * m_stereo.m_eyeOffset;
			view.Translate(offset);

			glCtx_SetColorWrite(ctx, 0, m_stereo.m_rightEyeColorMask);

			if (m_stereo.m_useReprojection)
				ReprojectToRightEye(view, camera->GetProj(), rightEyeResult);
			else
			{
				glCtx_Clear(ctx, glClearFlag_Depth | glClearFlag_Stencil);
				RenderScene(view, camera->GetProj(), rightEyeResult, 0);
			}

			glCtx_SetColorWrite(ctx, 0, glColorMask_All);
			break;
		}

		UE_INVALID_CASE(m_stereo.m_mode);
	}
}

void gxRenderer::RenderScene(const ueMat44& view, const ueMat44& proj, gxRenderResult& result, u32 level)
{
	if (m_forward.m_enableSoftwareOcclusionCulling)
		DoSoftwareOcclusionCulling(view, proj);

	// Initialize frame

	gxFrame frame;

	frame.InitializeMemory();

	frame.m_view = view;
	frame.m_proj = proj;
	ueMat44::SetMul(frame.m_viewProj, view, proj);
	frame.m_frustum.Build(frame.m_viewProj);

	// Find occluders

	m_scene->FindOccluders(&frame);

	// Find visible (against frustum & occluders) portals and lights

	m_scene->FindPortals(&frame);
	m_scene->FindLights(&frame);

	// Render

	switch (m_mode)
	{
		case Mode_Forward:
			RenderScene_Forward(&frame, result, level);
			break;
		case Mode_Deferred:
			RenderScene_Deferred(&frame, result, level);
			break;
		UE_INVALID_CASE(m_mode);
	}
}

void gxRenderer::RenderScene_Forward(gxFrame* frame, gxRenderResult& result, u32 level)
{
	// Render shadow maps

	for (u32 i = 0; i < frame.m_lights.Size(); i++)
		RenderShadowMap(&frame, frame.m_lights[i]);

#if 0
	// Render portals / mirrors with texture

	for (u32 i = 0; i < frame.m_portals.Size(); i++)
		RenderReflectionTexture(&frame, frame.m_portal[i]);
#endif

	// Set up passes required

	frame.m_passes[gxPass_Opaque].m_isUsed = UE_TRUE;
	frame.m_passes[gxPass_Transparent].m_isUsed = UE_TRUE;
	if (result.m_isRequired[gxRenderResult_Velocity])
		frame.m_passes[gxPass_Velocity].m_isUsed = UE_TRUE;
	if (result.m_isRequired[gxRenderResult_Depth] || m_forward.m_enableDepthPrepass)
		frame.m_passes[gxPass_Depth].m_isUsed = UE_TRUE;
	if (result.m_isRequired[gxRenderResult_Normals])
		frame.m_passes[gxPass_Normals].m_isUsed = UE_TRUE;

	// Collect render packets and sort them

	m_scene->SubmitRenderPackets(&frame);
	frame.SortPackets();

	// Optional software occlusion culling

	if (m_forward.m_enableSoftwareOcclusionCulling)
		PerformSoftwareOcclusionCulling(&frame);

	// Depth prepass

	if (frame.m_passes[gxPass_Depth].m_isUsed)
	{
		UE_NOT_IMPLEMENTED();
		DrawPass(&frame, gxPass_Depth);
	}

	// Velocity

	if (frame.m_passes[gxPass_Velocity].m_isUsed)
	{
		DrawPass(&frame, gxPass_Velocity);
	}

	// Opaque

	DrawPass(&frame, gxPass_Opaque);

	// Transparent

	DrawPass(&frame, gxPass_Transparent);

#if 0
	// Render portals / mirrors

	for (u32 i = 0; i < portals.Size(); i++)
		RenderPortalPost(&frame, frame.m_portal[i]);
#endif

	// Render lights

	for (u32 i = 0; i < frame.m_lights.Size(); i++)
		RenderLight(&frame, frame.m_lights[i]);
}

void gxRenderer::PerformSoftwareOcclusionCulling(const ueMat44& view, const ueMat44& proj)
{
}

void gxRenderer::DrawPass(rdDrawFrame* frame, rdDrawPass pass)
{
	rdDrawPacketContext ctx;
	ctx.m_frame = frame;
	ctx.m_pass = pass;

	gxPacket* packet = frame->m_passes[pass].m_packets;
	while (packet)
	{
		packet->m_drawFunc(packet, &ctx);
		packet = packet->m_next;
	}
}

void gxFrame::SortPackets()
{
	const int numFreeSortSlots = (m_memorySize - m_memoryPointer) / sizeof(gxPacket*);
	gxPacket** sortArray = (gxPacket**) m_memory;

	for (u32 i = 0; i < gxPass_MAX; i++)
	{
		gxPass& pass = m_passes[i];
		if (!pass.m_packets)
			continue;

		u32 numPackets = 0;
		gxPacket* p = pass.m_packets;
		while (p)
		{
			UE_ASSERT(numFreeSortSlots < numPackets);
			sortArray[numPackets++] = p;
			p = p->m_next;
		}

		ueSort(sortArray, numPackets, gxPacket::CmpPtrs);
	}
}