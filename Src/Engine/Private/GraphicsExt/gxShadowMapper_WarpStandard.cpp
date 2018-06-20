#include "GraphicsExt/gxShadowMapper.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCommonConstants.h"

ueBool gxShadowMapper::CalculateDrawPassesInfo_WarpStandard()
{
	switch (m_light.m_lightProjection)
	{
		case LightProjection_Directional:
		{
			// Determine splitting distances
			CalculateSplittingPlanes();

			// Calculate necessary transformations for each split
			const f32 frustumScale = 1.1f;
			ueVec3 frustumCorners[8];
			for (u32 i = 0; i < m_methodDesc.m_numShadowSplits; i++)
			{
				CalculateFrustumCorners(
					m_methodDesc.m_splitDistances[i], m_methodDesc.m_splitDistances[i + 1],
					frustumScale,
					frustumCorners);
				CalculateLightTransformation(
					i,
					frustumCorners,
					m_shadowMapPassInfos[i].m_lightView,
					m_shadowMapPassInfos[i].m_lightProj,
					m_shadowMaskPassInfos[i].m_lightTransformation);

				m_shadowMapPassInfos[i].m_frustum.Build(m_shadowMapPassInfos[i].m_lightView, m_shadowMapPassInfos[i].m_lightProj);

				m_shadowMaskPassInfos[i].m_cameraView = m_camera.m_view;
				m_shadowMaskPassInfos[i].m_cameraProj.SetPerspectiveFov(
					m_camera.m_FOV, m_camera.m_aspect,
					m_methodDesc.m_splitDistances[i], m_methodDesc.m_splitDistances[i + 1],
					UE_TRUE);
				m_shadowMaskPassInfos[i].m_frustum.Build(m_shadowMaskPassInfos[i].m_cameraView, m_shadowMaskPassInfos[i].m_cameraProj);
			}

			// Set up viewports
			switch (m_methodDesc.m_drawOrder)
			{
				case DrawOrder_AllShadowMapsFirst_SingleShadowMaskPass:
				{
					glViewportDesc& viewport = m_shadowMaskPassInfos[0].m_shadowMaskViewport;
					viewport.m_minZ = 0;
					viewport.m_maxZ = 1;
					viewport.m_left = 0;
					viewport.m_top = 0;
					viewport.m_width = m_methodDesc.m_shadowMaskWidth;
					viewport.m_height = m_methodDesc.m_shadowMaskHeight;
					break;
				}

				case DrawOrder_AllShadowMapsFirst_ShadowMaskPassPerSplit:
				case DrawOrder_FullPassPerSplit:
				{
					for (u32 i = 0; i < m_methodDesc.m_numShadowSplits; i++)
					{
						glViewportDesc& viewport = m_shadowMaskPassInfos[i].m_shadowMaskViewport;
						viewport.m_minZ = i / (f32) m_methodDesc.m_numShadowSplits;
						viewport.m_maxZ = (i + 1) / (f32) m_methodDesc.m_numShadowSplits;
						viewport.m_left = 0;
						viewport.m_top = 0;
						viewport.m_width = m_methodDesc.m_shadowMaskWidth;
						viewport.m_height = m_methodDesc.m_shadowMaskHeight;
					}
					break;
				}
			}

			return UE_TRUE;
		}

		default:
			return UE_FALSE;
	}
}

void gxShadowMapper::CalculateSplittingPlanes()
{
	if (m_methodDesc.m_enableAutomaticSplitting)
	{
		// Automatic split scheme:
		//
		// CLi = n*(f/n)^(i/numsplits)
		// CUi = n + (f-n)*(i/numsplits)
		// Ci = CLi*(lambda) + CUi*(1-lambda)
		//
		// Lambda scales between logarithmic and uniform
		//

		for (u32 i = 0; i < m_methodDesc.m_numShadowSplits; i++)
		{
			const f32 idm = i / (f32) m_methodDesc.m_numShadowSplits;
			const f32 logf = m_camera.m_nearPlane * uePow(m_camera.m_farPlane / m_camera.m_nearPlane, idm );
			const f32 uniform = m_camera.m_nearPlane + (m_camera.m_farPlane - m_camera.m_nearPlane) * idm;
			m_methodDesc.m_splitDistances[i] = logf * m_methodDesc.m_automaticSplitLambda + uniform * (1.0f - m_methodDesc.m_automaticSplitLambda);
		}		
	}

	// Clamp border distances
	m_methodDesc.m_splitDistances[0] = m_camera.m_nearPlane;
	m_methodDesc.m_splitDistances[m_methodDesc.m_numShadowSplits] = m_camera.m_farPlane;
}

void gxShadowMapper::CalculateFrustumCorners(f32 zNear, f32 zFar, f32 scale, ueVec3* corners)
{
	// Calculate projection matrix
	ueMat44 splitProjMatrix;
	splitProjMatrix.SetPerspectiveFov(m_camera.m_FOV, m_camera.m_aspect, zNear, zFar, UE_TRUE);

	// Calculate view projection matrix
	ueMat44 cameraSplitViewProj;
	ueMat44::Mul(cameraSplitViewProj, m_camera.m_view, splitProjMatrix);

	// Calculate inverted VP matrix
	ueMat44 invMatrix;
	ueMat44::Invert(invMatrix, cameraSplitViewProj);

	// Frustum corners in post projective space
	ueVec3 srcPoints[8];
	srcPoints[0].Set(-1.0f,  1.0f,  1.0f);
	srcPoints[1].Set(-1.0f, -1.0f,  1.0f);
	srcPoints[2].Set(1.0f, -1.0f,  1.0f);
	srcPoints[3].Set(1.0f,  1.0f,  1.0f);
	srcPoints[4].Set(-1.0f,  1.0f, -1.0f);
	srcPoints[5].Set(-1.0f, -1.0f, -1.0f);
	srcPoints[6].Set(1.0f, -1.0f, -1.0f);
	srcPoints[7].Set(1.0f,  1.0f, -1.0f);

	// Transform frustum corners back from post projective space to world space
	ueVec3 center = ueVec3::Zero;
	for (u32 i = 0; i < 8; i++)
	{
		invMatrix.TransformCoord(corners[i], srcPoints[i]);
		center += corners[i];
	}

	// Scale frustum
	if (scale != 1.0f)
	{
		// Get frustum center
		center *= 1.0f / 8.0f;

		// Extrude
		for (u32 i = 0; i < 8; i++)
			corners[i] += (corners[i] - center) * (scale - 1.0f);
	}
}

void gxShadowMapper::CalculateLightTransformation(u32 splitIndex, const ueVec3* corners,
	ueMat44& lightViewMatrix, ueMat44& lightProjMatrix, ueMat44& lightViewProjMatrix)
{
	// Light's orthogonal projection's width and height
	const f32 lightSize = 10000.0f; // Any number; it will be trimmed anyway

	// Compute light projection matrix
	lightProjMatrix.SetOrtho(lightSize, lightSize, 0, m_light.m_radius, UE_TRUE);

	// Compute light view matrix
	ueVec3 lightRefPosition = m_cameraPosition + m_light.m_direction * m_light.m_radius * -0.5f;
	lightViewMatrix.SetLookAt(lightRefPosition, m_cameraPosition, ueVec3_UnitY);

	// Rotate light view to align shadow map to view direction
	if (!m_methodDesc.m_hintStable)
	{
		ueVec3 cameraViewDirVS;
		lightViewMatrix.TransformNormal(cameraViewDirVS, m_cameraViewDir);
		cameraViewDirVS[2] = 0;
		cameraViewDirVS.Normalize();
		const f32 viewRotation = ueATan2(cameraViewDirVS[0], cameraViewDirVS[1]);
		lightViewMatrix.Rotate(0, 0, 1, viewRotation);
	}

	// Compute initial light view-proj matrix
	ueMat44::Mul(lightViewProjMatrix, lightViewMatrix, lightProjMatrix);

	// Find the min and max values of the current frustum split in lights post-projection space
	f32 maxX = F32_MIN;
	f32 maxY = F32_MIN;
	f32 minX = F32_MAX;
	f32 minY = F32_MAX;

	// Project each frustum corner and determine min / max in projection space
	for (u32 i = 0; i < 8; i++)
	{
		// Transform point
		ueVec3 projPos;
		lightViewProjMatrix.TransformCoord(projPos, corners[i]);

		// Find min and max values
		if (ueVec3_GetX(projPos) > maxX) maxX = ueVec3_GetX(projPos);
		if (ueVec3_GetY(projPos) > maxY) maxY = ueVec3_GetY(projPos);
		if (ueVec3_GetX(projPos) < minX) minX = ueVec3_GetX(projPos);
		if (ueVec3_GetY(projPos) < minY) minY = ueVec3_GetY(projPos);
	}	

	// Clamp values to valid range (post-projection)
	maxX = ueClamp(maxX, -1.0f, 1.0f);
	maxY = ueClamp(maxY, -1.0f, 1.0f);
	minX = ueClamp(minX, -1.0f, 1.0f);
	minY = ueClamp(minY, -1.0f, 1.0f);

	if (m_methodDesc.m_hintStable)
	{
		// FIXME: Testing stable shadow mapping
		struct SplitDimension
		{
			f32 m_width;
			f32 m_height;
			SplitDimension() : m_width(0), m_height(0) {}
			void Update(f32 width, f32 height)
			{
				m_width = ueMax(m_width, width);
				m_height = ueMax(m_height, height);
			}
		};
		static SplitDimension dims[10];

		SplitDimension& dim = dims[splitIndex];
		dim.Update(maxX - minX, maxY - minY);

		// Build a special matrix for cropping the lights view
		// to only contain points of the current frustum split
		const f32 scaleX = 2.0f / dim.m_width;
		const f32 scaleY = 2.0f / dim.m_height;
		const f32 offsetX = -0.5f * (maxX + minX) * scaleX;
		const f32 offsetY = -0.5f * (maxY + minY) * scaleY;

		ueMat44 cropMatrix;
		ueMat44_Set44(cropMatrix,
			scaleX,   0.0f,		0.0f,   0.0f,
			0.0f,     scaleY,   0.0f,   0.0f,
			0.0f,     0.0f,     1.0f,   0.0f,
			offsetX,  offsetY,  0.0f,   1.0f);

		ueMat44_Mul(lightProjMatrix, cropMatrix);

		ueMat44_Mul(lightViewProjMatrix, lightViewMatrix, lightProjMatrix);

		// Round to nearest shadow map texel in post-projection space
		ueVec3 origin;
		ueMat44_TransformCoord(origin, lightViewProjMatrix, ueVec3::Zero);

		const f32 shadowMapTexScale = m_methodDesc.m_shadowMapSize * 0.5f;
		const f32 xRound = (ueVec3_GetY(origin) * shadowMapTexScale - (u32) (ueVec3_GetX(origin) * shadowMapTexScale)) / shadowMapTexScale;
		const f32 yRound = (ueVec3_GetY(origin) * shadowMapTexScale - (u32) (ueVec3_GetY(origin) * shadowMapTexScale)) / shadowMapTexScale;

		ueMat44 roundMatrix;
		ueMat44_Set44(roundMatrix,
			1.0f,    0.0f,     0.0f,   0.0f,
			0.0f,    1.0f,     0.0f,   0.0f,
			0.0f,    0.0f,     1.0f,   0.0f,
			-xRound, -yRound,  0.0f,   1.0f);

		ueMat44_Mul(lightProjMatrix, roundMatrix);
	}
	else
	{
		// Build a special matrix for cropping the lights view to only contain points of the current frustum
		const f32 scaleX = 2.0f / ( maxX - minX );
		const f32 scaleY = 2.0f / ( maxY - minY );
		const f32 offsetX = -0.5f * (maxX + minX) * scaleX;
		const f32 offsetY = -0.5f * (maxY + minY) * scaleY;

		ueMat44 cropMatrix;
		ueMat44_Set44(cropMatrix,
			scaleX,	0.0f,		0.0f,	0.0f,
			0.0f,	scaleY,		0.0f,	0.0f,
			0.0f,	0.0f,		1.0f,	0.0f,
			offsetX,offsetY,	0.0f,	1.0f);

		ueMat44_Mul(lightProjMatrix, cropMatrix);
	}

	// Determine depth bias to be used
	const f32 depthBias = m_methodDesc.m_depthBias[ m_methodDesc.m_useSeparateDepthBiasPerSplit ? splitIndex : 0 ];

	// Account for depth bias and half-pixel offset with some APIs
	ueMat44 transMatrix;
	ueMat44_Set44(transMatrix,
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f + (0.5f / (f32) m_methodDesc.m_shadowMapSize), 0.5f + (0.5f / (f32) m_methodDesc.m_shadowMapSize), -depthBias, 1.0f);

	// Determine final light view projection matrix
	ueMat44_Mul(lightViewProjMatrix, lightViewMatrix, lightProjMatrix);
	ueMat44_Mul(lightViewProjMatrix, transMatrix);
}
