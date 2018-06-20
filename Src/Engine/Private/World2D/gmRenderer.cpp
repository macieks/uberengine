#include "GameObjectSystem/gmRenderer.h"
#include "Input/inSys.h"
#include "GX/gxSprite.h"
#include "GX/gxEffect.h"
#include "GX/gxFont.h"
#include "GX/gxTextRenderer.h"
#include "GL/glLib.h"

enum gmRendererState
{
	gmRendererState_None = 0,
	gmRendererState_Sprites,
	gmRendererState_Effects,
	gmRendererState_Fonts,

	gmRendererState_MAX
};

static gmRendererState s_state = gmRendererState_None;
static glCtx* s_ctx = NULL;
static ueMat44 s_view;
static ueRect s_projRect;

void gmRenderer_Begin(glCtx* ctx, const ueMat44* view, const ueRect& projRect)
{
	s_state = gmRendererState_None;
	s_ctx = ctx;
	s_view = *view;
	s_projRect = projRect;
}

void gmRenderer_EndState()
{
	switch (s_state)
	{
	case gmRendererState_Sprites:
		gxSprite_EndDraw();
		break;
	case gmRendererState_Effects:
		gxEffectTypeMgr_EndDraw();
		break;
	case gmRendererState_Fonts:
		break;
	}
}

void gmRenderer_End()
{
	gmRenderer_EndState();
	s_state = gmRendererState_None;
	s_ctx = NULL;
}

void gmRenderer_SwitchToSprites()
{
	if (s_state != gmRendererState_Sprites)
	{
		gmRenderer_EndState();

		s_state = gmRendererState_Sprites;
		gxSpriteBeginDrawParams beginDrawParams;
		beginDrawParams.m_ctx = s_ctx;
		beginDrawParams.m_globalTransform = &s_view;
		beginDrawParams.m_rect = s_projRect;
		gxSprite_BeginDraw(&beginDrawParams);
	}
}

void gmRenderer_Draw(gxSprite* sprite, gxSpriteDrawParams* params)
{
	gmRenderer_SwitchToSprites();
	gxSprite_Draw(sprite, params);
}

void gmRenderer_Draw(gxSprite* sprite, f32 x, f32 y, f32 rotation)
{
	gxSpriteDrawParams params;
	params.m_left = x;
	params.m_top = y;
	params.m_rotation = rotation;
	gmRenderer_Draw(sprite, &params);
}

void gmRenderer_SwitchToEffects()
{
	if (s_state != gmRendererState_Effects)
	{
		gmRenderer_EndState();

		s_state = gmRendererState_Effects;
		gxEffectTypeMgr_BeginDraw(s_ctx, s_view, s_projRect);
	}
}

void gmRenderer_SwitchToFonts()
{
	if (s_state != gmRendererState_Fonts)
	{
		gmRenderer_EndState();

		s_state = gmRendererState_Fonts;
#if 0 // FIXME: Fonts still need transform; here we're just *hoping* the same set up done for sprites is still valid - extremely dirty!!
		gxSpriteBeginDrawParams beginDrawParams;
		beginDrawParams.m_ctx = s_ctx;
		beginDrawParams.m_globalTransform = &s_view;
		beginDrawParams.m_rect = s_projRect;
		gxSprite_BeginDraw(&beginDrawParams);
#endif
	}
}

void gmRenderer_Draw(gxEffectHandle* effect)
{
	gmRenderer_SwitchToEffects();
	effect->Draw(s_ctx);
}

void gmRenderer_Print(gxText* text)
{
	gmRenderer_SwitchToFonts();
	gxTextRenderer_Draw(s_ctx, text);
}

void gmRenderer_Flush()
{
	if (s_state == gmRendererState_Sprites)
		gxSprite_Flush();
}

void gmRenderer_GetVirtualGameDimensions(s32& srcWidth, s32& srcHeight)
{
	srcWidth = 960;
	srcHeight = 640;
}

void gmRenderer_GetDrawTransforms(ueMat44& globalTransform, ueRect& drawRect, ueRect& unrotatedRect)
{
	ueRect worldRect;
	s32 width, height;
	gmRenderer_GetVirtualGameDimensions(width, height);
	worldRect.Set(0, 0, (f32) width, (f32) height);

	const f32 worldWidth = worldRect.GetWidth();
	const f32 worldHeight = worldRect.GetHeight();

	const ueBool isiOS =
#ifdef UE_MARMALADE
		s3eDeviceGetInt(S3E_DEVICE_OS) == S3E_OS_ID_IPHONE;
#else
		UE_FALSE;
#endif

	// Rotate level appropriately (both transform and rectangle)

#ifdef UE_MARMALADE
	if (isiOS)
	{
		ueMat44_SetTranslation(globalTransform, -worldWidth * 0.5f, -worldHeight * 0.5f, 0);
		ueMat44_Rotate(globalTransform, 0, 0, 1, UE_PI * 0.5f * ((inAccelerometer_GetFixedPosition() == inDevicePosition_LandscapeLeft) ? -1.0f : 1.0f));
		ueMat44_Translate(globalTransform, worldHeight * 0.5f, worldWidth * 0.5f, 0);
	}
	else
#endif
		globalTransform.SetIdentity();

	// Make the level fit centered on screen with preserved aspect ratio

#ifdef UE_MARMALADE
	const uint32 screenWidth = IwGxGetDisplayWidth();
	const uint32 screenHeight = IwGxGetDisplayHeight();
#else
	const glDeviceStartupParams* params = glDevice_GetStartupParams();

	const u32 screenWidth = params->m_width;
	const u32 screenHeight = params->m_height;
#endif

	 // In Marmalade emulator we're testing with device in portait mode

	f32 screenAspectRatio;
	if (isiOS)
		screenAspectRatio = (f32) screenHeight / (f32) screenWidth;
	else
		screenAspectRatio = (f32) screenWidth / (f32) screenHeight;

	const f32 levelAspectRatio = (f32) worldWidth / (f32) worldHeight;

	unrotatedRect = worldRect;

	if (screenAspectRatio > levelAspectRatio)
	{
		while (1)
		{
			if ((f32) (unrotatedRect.GetWidth() + 1) / (f32) unrotatedRect.GetHeight() > screenAspectRatio)
				break;
			unrotatedRect.m_left--;

			if ((f32) (unrotatedRect.GetWidth() + 1) / (f32) unrotatedRect.GetHeight() > screenAspectRatio)
				break;
			unrotatedRect.m_right++;
		}
	}
	else if (screenAspectRatio < levelAspectRatio)
	{
		while (1)
		{
			if ((f32) unrotatedRect.GetWidth() / (f32) (unrotatedRect.GetHeight() + 1) < screenAspectRatio)
				break;
			unrotatedRect.m_top--;

			if ((f32) unrotatedRect.GetWidth() / (f32) (unrotatedRect.GetHeight() + 1) < screenAspectRatio)
				break;
			unrotatedRect.m_bottom++;
		}
	}

	// Rotate draw rectangle by 90 degrees

	if (isiOS)
	{
		drawRect.m_left = unrotatedRect.m_top;
		drawRect.m_right = unrotatedRect.m_bottom;
		drawRect.m_top = unrotatedRect.m_left;
		drawRect.m_bottom = unrotatedRect.m_right;
	}
	else
		drawRect = unrotatedRect;
}