#ifndef GX_SPRITE_H
#define GX_SPRITE_H

#include "Base/ueShapes.h"

struct glCtx;
struct glTextureBuffer;
struct gxTexture;

#ifdef UE_MARMALADE
	class CIwMaterial;
#endif

/**
 *	@struct gxSprite
 *	@brief Animated 2D sprite object with support for animation blending
 */
struct gxSprite;

/**
 *	@struct gxSpriteTemplate
 *	@brief Animated sprite template used to create sprites; each sprite template corresponds with single .sprite.xml file
 */
struct gxSpriteTemplate;

//! Event callback function
typedef void (*gxSpriteEventCallback)(const char* name, void* userData);

//! Sprite animation play mode
enum gxSpriteAnimationPlayMode
{
	gxSpriteAnimationPlayMode_Loop = 0,
	gxSpriteAnimationPlayMode_Once,
	gxSpriteAnimationPlayMode_OnceAndFreeze,
	gxSpriteAnimationPlayMode_LoopWhenDone,		//!< Start looping when current animation finishes
	gxSpriteAnimationPlayMode_OnceWhenDone,		//!< Start playing when current animation finishes

	gxSpriteAnimationPlayMode_MAX
};

//! Parameters for when starting sprite rendering
struct gxSpriteBeginDrawParams
{
	glCtx* m_ctx;	//!< Context to be used for rendering
	ueRect m_rect;	//!< Rectangle to be used for viewport
	const ueMat44* m_globalTransform; //!< Optional global transformation

	gxSpriteBeginDrawParams() :
		m_ctx(NULL),
		m_rect(0, 0, 480, 320),
		m_globalTransform(NULL)
	{}
};

//! Sprite draw parameters
struct gxSpriteDrawParams
{
	//! Polygon description
	struct Poly
	{
		u32 m_numVerts;		//!< Number of polygon verts
		const void* m_pos;  //!< Pointer to float32 x 2 position data
		u32 m_posStride;	//!< Stride of the position data; 0 indicates default stride (i.e. sizeof(f32) * 2 = 8)
		const void* m_tex;  //!< Pointer to float32 x 2 texture coordinate data
		u32 m_texStride;	//!< Stride of the texture coordinate data; 0 indicates default stride (i.e. sizeof(f32) * 2 = 8)
		const u16* m_indices; //!< Optional indices

		UE_INLINE Poly() :
			m_numVerts(0),
			m_pos(NULL),
			m_posStride(0),
			m_tex(NULL),
			m_texStride(0),
			m_indices(NULL)
		{}
	};

	ueColor32 m_color;	//!< Color

	f32 m_rotation;		//!< Rotation; only used for rectangle
	f32 m_origin[2];	//!< Rotation center within [0..width-1, 0..height-1]

	// One of the 3 has to be specified: left & top coordinates, rectangle or polygon

	f32 m_left;			//!< Left coordinate (width used will be taken from sprite; optional)
	f32 m_top;			//!< Top coordinate (height used will be taken from sprite; optional)
	ueRect* m_rect;		//!< Rectangle (optional)
	ueRect* m_texRect;	//!< Texture coordinate rectangle (optional)
	Poly* m_poly;		//!< Polygon (optional)

	f32 m_layer;		//!< Layer "index"; lower index gets rendered first

	ueBool m_additiveBlending; //!< Indicates whether additive blending shall be used; otherwise default alpha-blending will be used

#ifdef UE_MARMALADE
	CIwMaterial* m_material;
#endif

	UE_INLINE gxSpriteDrawParams() :
		m_rotation(0.0f),
		m_left(0),
		m_top(0),
		m_rect(NULL),
		m_texRect(NULL),
		m_poly(NULL),
		m_layer(0.0f),
		m_additiveBlending(UE_FALSE)
#ifdef UE_MARMALADE
		,m_material(NULL)
#endif
	{
		m_origin[0] = m_origin[1] = 0.0f;
	}
};

//! Sprite subsystem startup parameters
struct gxSpriteStartupParams
{
	ueAllocator* m_allocator;	//!< Allocator
	u32 m_maxSpriteTemplates;	//!< Max. number of sprites
	u32 m_maxSprites;			//!< Max. number of sprites
	u32 m_maxAnimInstances;		//!< Max. animation instances among all sprites
	u32 m_maxDrawVerts;			//!< Max. draw vertices (for a single draw batch)
	u32 m_maxDrawSprites;		//!< Max. draw sprites (for a single draw batch)

#if !defined(GL_FIXED_PIPELINE)
	const char* m_VSName;		//!< Default sprite vertex shader
	const char* m_FSName;		//!< Default sprite fragment shader
	const char* m_animatedFSName;//!< Animated (blending between 2 frames) sprite fragment shader
#endif

	gxSpriteStartupParams() :
		m_allocator(NULL),
		m_maxSpriteTemplates(100),
		m_maxSprites(1000),
		m_maxAnimInstances(2000),
		m_maxDrawVerts(4096),
		m_maxDrawSprites(1024)
#if !defined(GL_FIXED_PIPELINE)
		,m_VSName("common/sprite_vs"),
		m_FSName("common/sprite_fs"),
		m_animatedFSName("common/animated_sprite_fs")
#endif
	{}
};

//! Starts up sprite subsystem
void		gxSprite_Startup(gxSpriteStartupParams* params);
//! Shuts down sprite subsystem
void		gxSprite_Shutdown();

//! Loads sprite template from xml
gxSpriteTemplate* gxSpriteTemplate_Get(const char* templateName);
//! Unloads sprite template
void		gxSpriteTemplate_Release(gxSpriteTemplate* t);
//! Gets name of the sprite template
const char*	gxSpriteTemplate_GetName(gxSpriteTemplate* t);
//! Gets dimensions of the sprite template
void		gxSpriteTemplate_GetSize(gxSpriteTemplate* t, u32& width, u32& height);

//! Creates sprite from given template
gxSprite*	gxSprite_Create(gxSpriteTemplate* templ);
//! Creates sprite from template of given name
gxSprite*	gxSprite_Create(const char* templateName);
//! Creates sprite from a single texture; such sprite will use no template and will have no animations
gxSprite*	gxSprite_CreateFromTexture(const char* textureName);
//! Creates sprite from a single texture; such sprite will use no template and will have no animations
gxSprite*	gxSprite_CreateFromTexture(gxTexture* texture);
//! Creates sprite from a single texture buffer; such sprite will use no template and will have no animations
gxSprite*	gxSprite_CreateFromTextureBuffer(glTextureBuffer* tb);
//! Destroys sprite
void		gxSprite_Destroy(gxSprite* sprite);
//! Gets dimensions of the sprite
void		gxSprite_GetSize(gxSprite* sprite, u32& width, u32& height);
//! Gets sprite name
const char* gxSprite_GetName(gxSprite* sprite);
//! Sets event callback for sprite
void		gxSprite_SetEventCallback(gxSprite* sprite, gxSpriteEventCallback callback, void* userData);

//! Starts animation on a sprite
void		gxSprite_PlayAnimation(gxSprite* sprite, const char* animationName, gxSpriteAnimationPlayMode mode = gxSpriteAnimationPlayMode_Loop, f32 transitionTime = 0.0f);
//! Starts default animation on a sprite
void		gxSprite_PlayDefaultAnimation(gxSprite* sprite, gxSpriteAnimationPlayMode mode = gxSpriteAnimationPlayMode_Loop, f32 transitionTime = 0.0f);
//! Gets animation length
f32			gxSprite_GetAnimationLength(gxSprite* sprite, const char* animationName);
//! Updates animation on sprite
void		gxSprite_Update(gxSprite* sprite, f32 dt);

//! Begins rendering sprites
void		gxSprite_BeginDraw(gxSpriteBeginDrawParams* params);
//! Ends rendering sprites; also flushes all pending render commands
void		gxSprite_EndDraw();
//! Draws sprite; note: the actual rendering migth be deferred until later stage (performs batching and state sorting)
void		gxSprite_Draw(gxSprite* sprite, gxSpriteDrawParams* drawParams);
//! Flushes all rendering commands
void		gxSprite_Flush();

//! Handy sprite pointer class
class gxSpritePtr
{
public:
	UE_INLINE gxSpritePtr() : m_sprite(NULL) {}
	UE_INLINE ~gxSpritePtr() { Destroy(); }
	UE_INLINE ueBool IsValid() const { return m_sprite != NULL; }
	UE_INLINE void operator = (gxSprite* other) { Destroy(); m_sprite = other; }
	UE_INLINE gxSprite* operator * () const { return m_sprite; }
	UE_INLINE gxSprite* operator -> () const { UE_ASSERT(m_sprite); return m_sprite; }
	UE_INLINE void Create(const char* templateName, const char* animationName = NULL)
	{
		Destroy();
		m_sprite = gxSprite_Create(templateName);
		UE_ASSERT(m_sprite);
		if (animationName)
			gxSprite_PlayAnimation(m_sprite, animationName);
	}
	UE_INLINE void Create(gxSpriteTemplate* t) { Destroy(); m_sprite = gxSprite_Create(t); UE_ASSERT(m_sprite); }
	UE_INLINE void CreateFromTexture(const char* textureName) { Destroy(); m_sprite = gxSprite_CreateFromTexture(textureName); UE_ASSERT(m_sprite); }
	UE_INLINE void CreateFromTexture(gxTexture* texture) { Destroy(); m_sprite = gxSprite_CreateFromTexture(texture); UE_ASSERT(m_sprite); }
	UE_INLINE void CreateFromTextureBuffer(glTextureBuffer* tb) { Destroy(); m_sprite = gxSprite_CreateFromTextureBuffer(tb); UE_ASSERT(m_sprite); }
	UE_INLINE void Destroy() { if (m_sprite) { gxSprite_Destroy(m_sprite); m_sprite = NULL; } }
	UE_INLINE void Update(f32 dt) { if (m_sprite) gxSprite_Update(m_sprite, dt); }
/*	UE_INLINE void Draw(gxSpriteDrawParams* drawParams) { if (m_sprite) gxSprite_Draw(m_sprite, drawParams); }
	UE_INLINE void Draw(f32 x, f32 y, f32 rot = 0.0f)
	{
		if (m_sprite)
		{
			gxSpriteDrawParams drawParams;
			drawParams.m_left = x;
			drawParams.m_top = y;
			drawParams.m_rotation = rot;
			gxSprite_Draw(m_sprite, &drawParams);
		}
	}*/
private:
	gxSprite* m_sprite;
};

#endif // GX_SPRITE_H