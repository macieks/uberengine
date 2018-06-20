#ifndef GM_RENDERER_H
#define GM_RENDERER_H

#include "Base/ueShapes.h"

struct glCtx;
struct gxSprite;
struct gxSpriteDrawParams;
class gxEffectHandle;
struct gxText;

void gmRenderer_Begin(glCtx* ctx, const ueMat44* view, const ueRect& projRect);
void gmRenderer_End();

void gmRenderer_SwitchToSprites();
void gmRenderer_SwitchToEffects();

void gmRenderer_Draw(gxSprite* sprite, gxSpriteDrawParams* params);
void gmRenderer_Draw(gxSprite* sprite, f32 x, f32 y, f32 rotation = 0.0f);
void gmRenderer_Draw(gxEffectHandle* effect);
void gmRenderer_Print(gxText* text);

void gmRenderer_Flush();

void gmRenderer_GetVirtualGameDimensions(s32& srcWidth, s32& srcHeight);
void gmRenderer_GetDrawTransforms(ueMat44& globalTransform, ueRect& drawRect, ueRect& unrotatedRect);

#endif // GM_RENDERER_H
