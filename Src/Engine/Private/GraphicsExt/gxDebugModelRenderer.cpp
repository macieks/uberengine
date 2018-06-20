#include "GraphicsExt/gxDebugModelRenderer.h"
#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxModelInstance.h"
#include "GraphicsExt/gxSkeleton.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxCommonConstants.h"

struct gxDebugModelRendererData
{
	ueBool m_isInitialized;
	gxProgram m_rigidProgram;
	gxProgram m_skinnedProgram;

	ueResourceHandle<gxTexture> m_defaultTexture;

	gxDebugModelRendererData() :
		m_isInitialized(UE_FALSE)
	{}
};

static gxDebugModelRendererData s_data;

void gxDebugModelRenderer_Startup(gxDebugModelRendererStartupParams* params)
{
	UE_ASSERT(!s_data.m_isInitialized);
	s_data.m_rigidProgram.Create(params->m_rigidVSName, params->m_FSName);
	s_data.m_skinnedProgram.Create(params->m_skinnedVSName, params->m_FSName);
	s_data.m_defaultTexture.SetByName(params->m_defaultTextureName);
	s_data.m_isInitialized = UE_TRUE;
}

void gxDebugModelRenderer_Shutdown()
{
	UE_ASSERT(s_data.m_isInitialized);
	s_data.m_rigidProgram.Destroy();
	s_data.m_skinnedProgram.Destroy();
	s_data.m_defaultTexture = NULL;
	s_data.m_isInitialized = UE_FALSE;
}

void gxDebugModelRenderer_Draw(gxDebugModelRendererDrawParams* params)
{
	UE_ASSERT(s_data.m_isInitialized);
	UE_ASSERT(params->m_model || params->m_modelInstance);
	UE_ASSERT(params->m_viewProj);
	UE_ASSERT(params->m_ctx);

	glCtx* ctx = params->m_ctx;
	gxModel* model = params->m_model ? params->m_model : gxModelInstance_GetModel(params->m_modelInstance);

	// Draw skeleton

	const gxSkeleton* skeleton = gxModel_GetLOD(model, params->m_lod)->m_skeleton;
	if (params->m_drawSkeleton && skeleton)
	{
		UE_PROF_SCOPE("Debug Draw Skeleton");

		gxShapeDrawParams shapeDrawParams;
		shapeDrawParams.m_viewProj = params->m_viewProj;
		gxShapeDraw_SetDrawParams(&shapeDrawParams);
		gxShapeDraw_Begin(ctx);

		const ueMat44* nodeTransforms = params->m_modelInstance ? gxModelInstance_GetNodeTransforms(params->m_modelInstance, params->m_lod) : NULL;
		for (u32 i = 0; i < skeleton->m_numNodes; i++)
		{
			const u32 parentIndex = skeleton->m_nodes[i].m_parentNodeIndex;
			if (parentIndex == U32_MAX)
				continue;

			gxShape_Line line;
			line.m_color = ueColor32::Green;
			if (nodeTransforms)
			{
				line.m_a = nodeTransforms[i].GetTranslation();
				line.m_b = nodeTransforms[parentIndex].GetTranslation();
			}
			else
			{
				UE_NOT_IMPLEMENTED();
			}
			gxShapeDraw_DrawLine(line);
		}

		gxShapeDraw_End();
	}

	// Draw model

	if (params->m_drawModel)
	{
		UE_PROF_SCOPE("Debug Draw Model");

		const ueMat44* worldTransform = params->m_world ? params->m_world : &ueMat44::Identity;

		gxModelLOD* lod = gxModel_GetLOD(model, params->m_lod);

		glCtx_SetFillMode(ctx, glFillMode_Solid);
		glCtx_SetBlending(ctx, UE_FALSE);
		glCtx_SetCullMode(ctx, glCullMode_CCW);
		glCtx_SetDepthWrite(ctx, UE_TRUE);
		glCtx_SetDepthTest(ctx, UE_TRUE);

		glCtx_SetIndices(ctx, lod->m_IB);

		glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::ViewProj, params->m_viewProj);
		glCtx_SetSamplerConstant(ctx, gxCommonConstants::ColorMap, params->m_texture ? params->m_texture : gxTexture_GetBuffer(*s_data.m_defaultTexture));

		for (u32 i = 0; i < lod->m_numMeshes; i++)
		{
			gxModelMesh* mesh = &lod->m_meshes[i];
			const ueBool isSkinned = mesh->m_numNodes >= 2;

			glCtx_SetStream(ctx, 0, lod->m_VB, lod->m_vertexFormats[mesh->m_vertexFormatIndex], mesh->m_vertexBufferOffset);

			glProgram* program = (isSkinned && params->m_modelInstance) ?
				(params->m_skinnedProgram ? params->m_skinnedProgram : s_data.m_skinnedProgram.GetProgram()) :
				(params->m_rigidProgram ? params->m_rigidProgram : s_data.m_rigidProgram.GetProgram());
			glCtx_SetProgram(ctx, program);

			if (params->m_modelInstance)
			{
				if (!isSkinned)
				{
					const ueMat44* transform =
						(mesh->m_numNodes == 1) ? // Rigid skinned?
						&gxModelInstance_GetNodeTransforms(params->m_modelInstance, params->m_lod)[mesh->m_boneIndices[0]] :
						gxModelInstance_GetBaseTransform(params->m_modelInstance);

					glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::World, transform);
				}
				else
				{
					ueMat44 nodeTransforms[64];
					UE_ASSERT(mesh->m_numNodes <= UE_ARRAY_SIZE(nodeTransforms));
					gxModelInstance_GetSkinningNodeTransforms(params->m_modelInstance, params->m_lod, mesh->m_numNodes, mesh->m_boneIndices, nodeTransforms);
					glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::BoneTransforms, nodeTransforms, mesh->m_numNodes);
				}
			}
			else
				glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::World, worldTransform);

			glCtx_DrawIndexed(ctx, (glPrimitive) mesh->m_primType, mesh->m_baseVertex, mesh->m_firstVertex, mesh->m_numVerts, mesh->m_firstIndex, mesh->m_numIndices);
		}
	}

	// Draw normals

	if (params->m_drawNormals)
	{
		UE_PROF_SCOPE("Debug Draw Normals");

		const ueMat44* worldTransform = params->m_world ? params->m_world : &ueMat44::Identity;

		gxModelLOD* lod = gxModel_GetLOD(model, params->m_lod);

		gxShapeDrawParams drawParams;
		drawParams.m_viewProj = params->m_viewProj;
		gxShapeDraw_SetDrawParams(&drawParams);
		gxShapeDraw_Begin(ctx);

		if (const void* vertexData = glVertexBuffer_Lock(lod->m_VB, glBufferLockFlags_Read))
		{
			for (u32 i = 0; i < lod->m_numMeshes; i++)
			{
				gxModelMesh* mesh = &lod->m_meshes[i];
				glStreamFormat* format = lod->m_vertexFormats[mesh->m_vertexFormatIndex];

				const u32 stride = glStreamFormat_GetStride(format);

				const glVertexElement* posInfo = glStreamFormat_GetElement(format, glSemantic_Position);
				const glVertexElement* normalInfo = glStreamFormat_GetElement(format, glSemantic_Normal);
				if (!posInfo || !normalInfo)
					continue;

				const u8* vertex = (const u8*) vertexData + mesh->m_vertexBufferOffset;
				for (u32 j = 0; j < mesh->m_numVerts; j++)
				{
					ueVec3 pos, normal;
					glVertexElement_GetDataAsF32(normalInfo, (f32*) &normal, 3, vertex);
					glVertexElement_GetDataAsF32(posInfo, (f32*) &pos, 3, vertex);

					gxShape_Line line;
					line.m_a = pos;
					line.m_b = pos + normal;
					line.m_transform = worldTransform;
					gxShapeDraw_DrawLine(line);

					vertex += stride;
				}
			}

			glVertexBuffer_Unlock(lod->m_VB);
		}

		gxShapeDraw_End();
	}
}