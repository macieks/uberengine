#include "GraphicsExt/gxSprite.h"
#include "GraphicsExt/gxTexture.h"
#include "Graphics/glLib.h"
#if !defined(GL_FIXED_PIPELINE)
	#include "GraphicsExt/gxProgram.h"
	#include "Graphics/glVertexBufferFactory.h"
	#include "Graphics/glIndexBufferFactory.h"
	#include "GraphicsExt/gxCommonConstants.h"
#endif
#include "Base/Containers/ueList.h"
#include "Base/Containers/ueGenericPool.h"
#include "Base/Containers/ueHashSet.h"
#include "IO/ioXml.h"
#include "Base/ueCmp.h"
#include "Base/ueSorting.h"

struct gxAnimation2DFrame
{
	ueResourceHandle<gxTexture> m_texture;
};

struct gxAnimation2DEvent
{
	f32 m_time;
	const char* m_name;
};

struct gxAnimation2D
{
	const char* m_name;

	ueBool m_isDefault;

	f32 m_frameTime;
	u32 m_numFrames;

	f32 m_totalTime;

	ueBool m_blendFrames;

	gxAnimation2DFrame* m_frames;

	u32 m_numEvents;
	gxAnimation2DEvent* m_events;

	gxAnimation2D() :
		m_isDefault(UE_FALSE),
		m_frameTime(0.3f),
		m_totalTime(0.0f),
		m_blendFrames(UE_FALSE),
		m_numEvents(0),
		m_events(NULL)
	{}
};

struct gxSpriteTemplate
{
	u32 m_refCount;

	const char* m_name;

	u32 m_numAnims;
	gxAnimation2D** m_anims;
	gxAnimation2D* m_defaultAnim;

	gxSpriteTemplate() :
		m_refCount(0),
		m_name(NULL),
		m_defaultAnim(NULL)
	{}

	struct HashPred
	{
		UE_INLINE u32 operator () (const gxSpriteTemplate* t) const
		{
			return ueCalcHash(t->m_name);
		}
	};

	struct CmpPred
	{
		s32 operator () (const gxSpriteTemplate* a, const gxSpriteTemplate* b) const
		{
			return ueStrCmp(a->m_name, b->m_name);
		}
	};
};

struct gxAnimation2DInstance : ueList<gxAnimation2DInstance>::Node
{
	gxAnimation2D* m_animation;
	f32 m_time;
	f32 m_weight;
	f32 m_weightChangeSpeed;
	gxSpriteAnimationPlayMode m_mode;
};

struct gxSprite
{
	gxSpriteTemplate* m_template;

	ueResourceHandle<gxTexture> m_defaultTexture; // Texture used when template is NULL
	glTextureBuffer* m_defaultTextureBuffer;

	ueList<gxAnimation2DInstance> m_animInstances;

	gxSpriteEventCallback m_eventCallback;
	void* m_userData;

#if defined(GL_MARMALADE)
	CIwMaterial* m_materials[2];
#endif

	gxSprite() :
		m_template(NULL),
		m_defaultTextureBuffer(NULL),
		m_eventCallback(NULL),
		m_userData(NULL)
	{
#if defined(GL_MARMALADE)
		m_materials[0] = m_materials[1] = NULL;
#endif
	}
};

#if !defined(GL_MARMALADE)

struct gxSpriteVertex
{
	f32 pos[2];
	f32 lerp;
	f32 uv0[2];
	f32 uv1[2];
	u32 color;
};

#endif

struct gxSpriteDrawCommand
{
	u32 m_firstVertex;
	u32 m_numVerts;
	u32 m_firstIndex;
	u32 m_numIndices;

	ueColor32 m_color;

	f32 m_lerpScale;

	ueBool m_blendFrames;
	s16 m_commandIndex;

	ueBool m_additiveBlending;

	gxTexture* m_textures[2];
	glTextureBuffer* m_textureBuffers[2];
	f32 m_layer;

	gxSprite* m_sprite;

#if defined(GL_MARMALADE)

	CIwMaterial* m_material;

	struct BatchCmp
	{
		UE_INLINE s32 operator () (const gxSpriteDrawCommand& a, const gxSpriteDrawCommand& b) const
		{
			const s32 cmp = ueCmpPointers(a.m_textures[0], b.m_textures[0]);
			if (cmp != 0) return cmp;
			return (s32) a.m_additiveBlending - (s32) b.m_additiveBlending;
		}
	};

	struct SortCmp
	{
		UE_INLINE s32 operator () (const gxSpriteDrawCommand& a, const gxSpriteDrawCommand& b) const
		{
			if (a.m_layer < b.m_layer) return -1;
			if (a.m_layer > b.m_layer) return 1;
			s32 cmp = ueCmpPointers(a.m_sprite, b.m_sprite);
			if (cmp) return cmp;
			cmp = (s32) a.m_additiveBlending - (s32) b.m_additiveBlending;
			return cmp ? cmp : (a.m_commandIndex - b.m_commandIndex); // Make the sort stable to avoid flickering
		}
	};

#else // defined(GL_MARMALADE)

	struct BatchCmp
	{
		UE_INLINE s32 operator () (const gxSpriteDrawCommand& a, const gxSpriteDrawCommand& b) const
		{
			return ueCmpMemberRange(a.m_textures, a.m_textures, b.m_textures, b.m_textures);
		}
	};

	struct SortCmp
	{
		UE_INLINE s32 operator () (const gxSpriteDrawCommand& a, const gxSpriteDrawCommand& b) const
		{
			if (a.m_layer < b.m_layer) return -1;
			if (a.m_layer > b.m_layer) return 1;
			const s32 cmp = ueCmpPointers(a.m_sprite, b.m_sprite);
			return cmp ? cmp : (a.m_commandIndex - b.m_commandIndex); // Make the sort stable to avoid flickering
		}
	};

#endif // defined(GL_MARMALADE)
};

typedef ueHashSet<gxSpriteTemplate*, gxSpriteTemplate::HashPred, gxSpriteTemplate::CmpPred> TemplateSetType;

struct gxSpriteData
{
	ueAllocator* m_allocator;

	TemplateSetType m_templates;

	// Pools for frequently created objects

	ueGenericPool m_spritesPool;
	ueGenericPool m_animInstancesPool;

	// Drawing

	ueBool m_isDrawing;
	glCtx* m_ctx;

	u32 m_numDrawBatches;

	u32 m_numVerts;
	u32 m_maxVerts;
#if defined(GL_MARMALADE)
	CIwSVec3* m_pos;
	CIwSVec2* m_uv;
	CIwColour* m_color;
#else
	gxSpriteVertex* m_drawVerts;
#endif

	u32 m_numIndices;
	u32 m_maxIndices;
	u16* m_drawIndices;

	u32 m_numDrawCommands;
	u32 m_maxDrawCommands;
	gxSpriteDrawCommand* m_drawCommands;

#if !defined(GL_MARMALADE)

	glVertexBufferFactory m_VB;
	glIndexBufferFactory m_IB;

	glStreamFormat* m_SF;

	gxProgram m_program;
	gxProgram m_animatedProgram;

#endif // !defined(GL_MARMALADE)
};

static gxSpriteData* s_data = NULL;

void gxSprite_Startup(gxSpriteStartupParams* params)
{
	UE_ASSERT(!s_data);

	const ueSize spritesMemorySize = ueGenericPool::CalcMemReq(sizeof(gxSprite), params->m_maxSprites);
	const ueSize animInstancesMemorySize = ueGenericPool::CalcMemReq(sizeof(gxAnimation2DInstance), params->m_maxAnimInstances);
	const ueSize templateSetSize = TemplateSetType::CalcMemReq(params->m_maxSpriteTemplates);
	const ueSize drawMemorySize =
		sizeof(u16) * (params->m_maxDrawVerts - 2) * 3 +
#if defined(GL_MARMALADE)
		(sizeof(CIwSVec3) + sizeof(CIwSVec2) + sizeof(CIwColour)) * params->m_maxDrawVerts +
#else
		sizeof(gxSpriteVertex) * params->m_maxDrawVerts +
#endif
		sizeof(gxSpriteDrawCommand) * params->m_maxDrawSprites;
	const ueSize alignedSpriteDataSize = ueAlignPow2((u32) sizeof(gxSpriteData), (u32) UE_DEFAULT_ALIGNMENT);
	const ueSize memorySize = alignedSpriteDataSize + spritesMemorySize + animInstancesMemorySize + drawMemorySize + templateSetSize;

	u8* memory = (u8*) params->m_allocator->Alloc(memorySize);
	UE_ASSERT(memory);

	s_data = new(memory) gxSpriteData();
	memory += alignedSpriteDataSize;
	s_data->m_allocator = params->m_allocator;

	s_data->m_spritesPool.InitMem(memory, spritesMemorySize, sizeof(gxSprite), params->m_maxSprites);
	memory += spritesMemorySize;

	s_data->m_animInstancesPool.InitMem(memory, animInstancesMemorySize, sizeof(gxAnimation2DInstance), params->m_maxAnimInstances);
	memory += animInstancesMemorySize;

	s_data->m_templates.InitMem(memory, templateSetSize, params->m_maxSpriteTemplates);
	memory += templateSetSize;

	s_data->m_numVerts = 0;
	s_data->m_maxVerts = params->m_maxDrawVerts;
#if defined(GL_MARMALADE)
	s_data->m_pos = (CIwSVec3*) memory;
	memory += sizeof(CIwSVec3) * params->m_maxDrawVerts;
	s_data->m_uv = (CIwSVec2*) memory;
	memory += sizeof(CIwSVec2) * params->m_maxDrawVerts;
	s_data->m_color = (CIwColour*) memory;
	memory += sizeof(CIwColour) * params->m_maxDrawVerts;
#else
	s_data->m_drawVerts = (gxSpriteVertex*) memory;
	memory += sizeof(gxSpriteVertex) * params->m_maxDrawVerts;
#endif

	s_data->m_numIndices = 0;
	s_data->m_maxIndices = (params->m_maxDrawVerts - 2) * 3;
	s_data->m_drawIndices = (u16*) memory;
	memory += sizeof(u16) * (params->m_maxDrawVerts - 2) * 3;

	s_data->m_numDrawCommands = 0;
	s_data->m_maxDrawCommands = params->m_maxDrawSprites;
	s_data->m_drawCommands = (gxSpriteDrawCommand*) memory;
	memory += sizeof(gxSpriteDrawCommand) * params->m_maxDrawSprites;

	s_data->m_numDrawBatches = 0;

#if !defined(GL_MARMALADE)

	s_data->m_VB.Init(sizeof(f32) * 2 * params->m_maxDrawVerts);
	s_data->m_IB.Init(params->m_maxDrawVerts, sizeof(u16));

	const glVertexElement sfElems[] = 
	{
		{glSemantic_Position, 0, ueNumType_F32, 3, UE_FALSE, UE_OFFSET_OF(gxSpriteVertex, pos)},
		{glSemantic_TexCoord, 0, ueNumType_F32, 4, UE_FALSE, UE_OFFSET_OF(gxSpriteVertex, uv0)},
		{glSemantic_Color, 0, ueNumType_U8, 4, UE_TRUE, UE_OFFSET_OF(gxSpriteVertex, color)},
	};
 
	glStreamFormatDesc sfDesc;
	sfDesc.m_stride = sizeof(gxSpriteVertex);
	sfDesc.m_numElements = UE_ARRAY_SIZE(sfElems);
	sfDesc.m_elements = sfElems;
	s_data->m_SF = glStreamFormat_Create(&sfDesc);
	UE_ASSERT(s_data->m_SF);

	s_data->m_program.Create(params->m_VSName, params->m_FSName);
	s_data->m_animatedProgram.Create(params->m_VSName, params->m_animatedFSName);

#endif // !defined(GL_MARMALADE)

	s_data->m_isDrawing = UE_FALSE;
}

void gxSprite_Shutdown()
{
	UE_ASSERT(s_data);

#ifdef UE_ENABLE_ASSERTION
	TemplateSetType::Iterator iter(s_data->m_templates);
	while (gxSpriteTemplate** t = iter.Next())
		ueLogE("Unreleased sprite template (name = '%s')",(*t)->m_name);
	UE_ASSERT(!s_data->m_templates.Size());
#endif
	s_data->m_templates.Deinit();

#if !defined(GL_FIXED_PIPELINE)
	s_data->m_program.Destroy();
	s_data->m_animatedProgram.Destroy();
	s_data->m_VB.Deinit();
	s_data->m_IB.Deinit();
	glStreamFormat_Destroy(s_data->m_SF);
#endif

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

gxSpriteTemplate* gxSpriteTemplate_Get(const char* spriteName)
{
	UE_ASSERT(s_data);

	// Find existing

	gxSpriteTemplate temp;
	temp.m_name = spriteName;
	gxSpriteTemplate** templatePtr = s_data->m_templates.Find(&temp);
	if (templatePtr)
	{
		(*templatePtr)->m_refCount++;
		return *templatePtr;
	}

	// Parse sprite Xml

	uePath spriteFilePath;
	ueStrFormatS(spriteFilePath, "%s.sprite.xml", spriteName);
	ioXmlDoc* doc = ioXmlDoc_Load(s_data->m_allocator, spriteFilePath);
	UE_ASSERT(doc);
	ioXmlDocScopedDestructor docDestructor(doc);

	ioXmlNode* spriteNode = ioXmlDoc_GetFirstNode(doc, "sprite");
	UE_ASSERT(spriteNode);

	// Create sprite template

	const u32 numAnims = ioXmlNode_CalcNumNodes(spriteNode, "animation");
	UE_ASSERT(numAnims > 0);

	const u32 nameLength = ueStrLen(spriteName);

	const ueSize templateMemorySize = sizeof(gxSpriteTemplate) + numAnims * sizeof(gxAnimation2D*) + nameLength + 1;
	u8* templateMemory = (u8*) s_data->m_allocator->Alloc(templateMemorySize);
	UE_ASSERT(templateMemory);

	gxSpriteTemplate* templ = new(templateMemory) gxSpriteTemplate;
	templateMemory += sizeof(gxSpriteTemplate);
	templ->m_refCount = 1;

	templ->m_numAnims = numAnims;
	templ->m_anims = (gxAnimation2D**) templateMemory;
	templateMemory += numAnims * sizeof(gxAnimation2D*);

	ueMemCpy(templateMemory, spriteName, nameLength + 1);
	templ->m_name = (const char*) templateMemory;
	templateMemory += nameLength + 1;

	// Load animations

	u32 animIndex = 0;
	for (ioXmlNode* animNode = ioXmlNode_GetFirstNode(spriteNode, "animation"); animNode; animNode = ioXmlNode_GetNext(animNode, "animation"))
	{
		const char* name = ioXmlNode_GetAttrValue(animNode, "name");

		const u32 numFrames = ioXmlNode_CalcNumNodes(animNode, "frame");

		u32 numEvents = 0;
		u32 eventsSize = 0;
		for (ioXmlNode* eventNode = ioXmlNode_GetFirstNode(animNode, "event"); eventNode; eventNode = ioXmlNode_GetNext(eventNode, "event"))
		{
			const char* name = ioXmlNode_GetAttrValue(eventNode, "name");
			eventsSize += sizeof(gxAnimation2DEvent) + ueStrLen(name) + 1;
			numEvents++;
		}

		// Allocate animation memory

		const u32 nameLength = ueStrLen(name);
		u8* animMemory = (u8*) s_data->m_allocator->Alloc(
			sizeof(gxAnimation2D) +
			numFrames * sizeof(gxAnimation2DFrame) +
			eventsSize +
			nameLength + 1);
		UE_ASSERT(animMemory);

		// Create animation

		gxAnimation2D* anim = new(animMemory) gxAnimation2D();
		animMemory += sizeof(gxAnimation2D);

		anim->m_numFrames = numFrames;
		anim->m_frames = TTypeInfoComplex<gxAnimation2DFrame>::ConstructArray(animMemory, numFrames);
		animMemory += numFrames * sizeof(gxAnimation2DFrame);

		anim->m_numEvents = numEvents;
		if (numEvents)
		{
			anim->m_events = (gxAnimation2DEvent*) animMemory;
			animMemory += sizeof(gxAnimation2DEvent) * numEvents;
		}

		ueMemCpy(animMemory, name, nameLength + 1);
		anim->m_name = (const char*) animMemory;
		animMemory += nameLength + 1;

		// Get frame time

		ioXmlNode_GetAttrValueF32(animNode, "frameTime", anim->m_frameTime);
		anim->m_totalTime = (f32) numFrames * anim->m_frameTime;

		// Check blend mode

		ioXmlNode_GetAttrValueBool(animNode, "blend", anim->m_blendFrames);

		// Check if default

		ueBool isDefault;
		if (!animIndex || (ioXmlNode_GetAttrValueBool(animNode, "isDefault", isDefault) && isDefault))
		{
			templ->m_defaultAnim = anim;
			anim->m_isDefault = UE_TRUE;
		}

		// Load all frames and events

		u32 frameIndex = 0;
		u32 eventIndex = 0;
		f32 time = 0.0f;
		for (ioXmlNode* elemNode = ioXmlNode_GetFirstNode(animNode); elemNode; elemNode = ioXmlNode_GetNext(elemNode))
		{
			const char* elemName = ioXmlNode_GetName(elemNode);

			if (!ueStrCmp(elemName, "frame"))
			{
				gxAnimation2DFrame& frame = anim->m_frames[frameIndex++];

				const char* textureName = ioXmlNode_GetAttrValue(elemNode, "texture");
				UE_ASSERT(textureName);
				frame.m_texture.SetByName(textureName);

				time += anim->m_frameTime;
			}
			else if (!ueStrCmp(elemName, "event"))
			{
				gxAnimation2DEvent& ev = anim->m_events[eventIndex++];
				ev.m_time = time;
				ev.m_name = (char*) animMemory;

				const char* name = ioXmlNode_GetAttrValue(elemNode, "name");
				const u32 nameSize = ueStrLen(name) + 1;
				ueMemCpy(animMemory, name, nameSize);
				animMemory += nameSize;
			}
		}

		templ->m_anims[animIndex++] = anim;
	}

	UE_ASSERT(templ->m_defaultAnim);

	// Add template to set

	s_data->m_templates.Insert(templ);

	return templ;
}

void gxSprite_SetEventCallback(gxSprite* sprite, gxSpriteEventCallback callback, void* userData)
{
	sprite->m_eventCallback = callback;
	sprite->m_userData = userData;
}

void gxSpriteTemplate_Release(gxSpriteTemplate* t)
{
	UE_ASSERT(s_data);

	if (--t->m_refCount)
		return;

	s_data->m_templates.Remove(t);

	for (u32 i = 0; i < t->m_numAnims; i++)
	{
		gxAnimation2D* anim = t->m_anims[i];
		for (u32 j = 0; j < anim->m_numFrames; j++)
		{
			gxAnimation2DFrame& frame = anim->m_frames[j];
			frame.m_texture.Release();
		}
		ueDelete(anim, s_data->m_allocator);
	}
	ueDelete(t, s_data->m_allocator);
}

void gxSpriteTemplate_GetSize(gxSpriteTemplate* t, u32& width, u32& height)
{
	gxTexture* texture = *t->m_defaultAnim->m_frames->m_texture;
	width = gxTexture_GetWidth(texture);
	height = gxTexture_GetHeight(texture);
}

const char* gxSpriteTemplate_GetName(gxSpriteTemplate* t)
{
	return t->m_name;
}

void gxSprite_GetSize(gxSprite* t, u32& width, u32& height)
{
	if (t->m_template)
		gxSpriteTemplate_GetSize(t->m_template, width, height);
	else
	{
		gxTexture* texture = *t->m_defaultTexture;
		width = gxTexture_GetWidth(texture);
		height = gxTexture_GetHeight(texture);
	}
}

const char* gxSprite_GetName(gxSprite* sprite)
{
	return sprite->m_template ? sprite->m_template->m_name : ueResource_GetName(sprite->m_defaultTexture.AsResource());
}

#if defined(GL_MARMALADE)

void gxSprite_CreateMaterials(gxSprite* sprite)
{
	// Create first material

	sprite->m_materials[0] = new(s_data->m_allocator) CIwMaterial();
	UE_ASSERT(sprite->m_materials[0]);

	glTextureBuffer* tex =
		sprite->m_template ?
		sprite->m_template->m_defaultAnim->m_frames[0].m_texture->GetBuffer() :
		sprite->m_defaultTextureBuffer;
	CIwTexture* ciwTex = glTextureBuffer_GetIwTexture(tex);
	sprite->m_materials[0]->SetTexture(ciwTex);

	sprite->m_materials[0]->SetAlphaMode(ciwTex->HasAlpha() ? CIwMaterial::ALPHA_BLEND : CIwMaterial::ALPHA_NONE);
//	sprite->m_materials[0]->SetModulateMode(CIwMaterial::MODULATE_NONE);

	sprite->m_materials[0]->SetClamping(true);

	sprite->m_materials[0]->SetDepthWriteMode(CIwMaterial::DEPTH_WRITE_DISABLED);
	sprite->m_materials[0]->SetCullMode(CIwMaterial::CULL_NONE);

	// Create second material for animated objects

	if (sprite->m_template)
	{
		sprite->m_materials[1] = new(s_data->m_allocator) CIwMaterial();
		UE_ASSERT(sprite->m_materials[1]);

		sprite->m_materials[1]->SetDepthWriteMode(CIwMaterial::DEPTH_WRITE_DISABLED);
		sprite->m_materials[1]->SetCullMode(CIwMaterial::CULL_NONE);

		sprite->m_materials[1]->SetClamping(true);

		sprite->m_materials[1]->SetAlphaMode(ciwTex->HasAlpha() ? CIwMaterial::ALPHA_BLEND : CIwMaterial::ALPHA_NONE);

		sprite->m_materials[1]->SetDepthWriteMode(CIwMaterial::DEPTH_WRITE_DISABLED);
		sprite->m_materials[1]->SetCullMode(CIwMaterial::CULL_NONE);
	}
}

#endif // defined(GL_MARMALADE)

gxSprite* gxSprite_Create(const char* templateName)
{
	gxSpriteTemplate* templ = gxSpriteTemplate_Get(templateName);
	if (!templ)
		return NULL;

	gxSprite* sprite = gxSprite_Create(templ);
	if (!sprite)
		return NULL;

	templ->m_refCount--; // We've increased ref count when we called gxSpriteTemplate_Get
	return sprite;
}

gxSprite* gxSprite_Create(gxSpriteTemplate* templ)
{
	UE_ASSERT(s_data);

	gxSprite* sprite = new(s_data->m_spritesPool) gxSprite();
	UE_ASSERT(sprite);

	sprite->m_template = templ;
	sprite->m_template->m_refCount++;

#if defined(GL_MARMALADE)
	gxSprite_CreateMaterials(sprite);
#endif
	gxSprite_PlayDefaultAnimation(sprite);

	return sprite;
}

gxSprite* gxSprite_CreateFromTexture(const char* textureName)
{
	UE_ASSERT(s_data);

	gxSprite* sprite = new(s_data->m_spritesPool) gxSprite();
	UE_ASSERT(sprite);

	sprite->m_defaultTexture.SetByName(textureName);
	UE_ASSERT(sprite->m_defaultTexture.IsReady());
	sprite->m_defaultTextureBuffer = gxTexture_GetBuffer(*sprite->m_defaultTexture);

#if defined(GL_MARMALADE)
	gxSprite_CreateMaterials(sprite);
#endif

	return sprite;
}

gxSprite* gxSprite_CreateFromTexture(gxTexture* texture)
{
	UE_ASSERT(s_data);

	gxSprite* sprite = new(s_data->m_spritesPool) gxSprite();
	UE_ASSERT(sprite);

	sprite->m_defaultTexture = texture;
	sprite->m_defaultTextureBuffer = gxTexture_GetBuffer(texture);
#if defined(GL_MARMALADE)
	gxSprite_CreateMaterials(sprite);
#endif

	return sprite;
}

gxSprite* gxSprite_CreateFromTextureBuffer(glTextureBuffer* tb)
{
	UE_ASSERT(s_data);

	gxSprite* sprite = new(s_data->m_spritesPool) gxSprite();
	UE_ASSERT(sprite);

	sprite->m_defaultTextureBuffer = tb;
#if defined(GL_MARMALADE)
	gxSprite_CreateMaterials(sprite);
#endif

	return sprite;
}

void gxSprite_Destroy(gxSprite* sprite)
{
	UE_ASSERT(s_data);
	while (gxAnimation2DInstance* inst = sprite->m_animInstances.PopFront())
		ueDelete(inst, s_data->m_animInstancesPool);
#if defined(GL_MARMALADE)
	ueDelete(sprite->m_materials[0], s_data->m_allocator);
	if (sprite->m_materials[1])
		ueDelete(sprite->m_materials[1], s_data->m_allocator);
#endif
	if (sprite->m_template)
		gxSpriteTemplate_Release(sprite->m_template);
	if (!sprite->m_defaultTexture.IsValid() && sprite->m_defaultTextureBuffer)
		glTextureBuffer_Destroy(sprite->m_defaultTextureBuffer);
	ueDelete(sprite, s_data->m_spritesPool);
}

f32 gxSprite_GetAnimationLength(gxSprite* sprite, const char* animationName)
{
	UE_ASSERT(s_data);

	// Get template

	gxSpriteTemplate* templ = sprite->m_template;
	UE_ASSERT(templ);

	// Get animation

	gxAnimation2D* anim = NULL;
	for (u32 i = 0; i < templ->m_numAnims; i++)
		if (!ueStrCmp(templ->m_anims[i]->m_name, animationName))
		{
			anim = templ->m_anims[i];
			break;
		}
	UE_ASSERT_MSGP(anim, "Animation '%s' not found in sprite template '%s'", animationName, templ->m_name);

	return anim->m_totalTime;
}

void gxSprite_PlayDefaultAnimation(gxSprite* sprite, gxSpriteAnimationPlayMode mode, f32 transitionTime)
{
	gxSprite_PlayAnimation(sprite, sprite->m_template->m_defaultAnim->m_name, mode, transitionTime);
}

void gxSprite_PlayAnimation(gxSprite* sprite, const char* animationName, gxSpriteAnimationPlayMode mode, f32 transitionTime)
{
	UE_ASSERT(s_data);

	// Get template

	gxSpriteTemplate* templ = sprite->m_template;
	UE_ASSERT(templ);

	// Get animation

	gxAnimation2D* anim = NULL;
	for (u32 i = 0; i < templ->m_numAnims; i++)
		if (!ueStrCmp(templ->m_anims[i]->m_name, animationName))
		{
			anim = templ->m_anims[i];
			break;
		}
	UE_ASSERT_MSGP(anim, "Animation '%s' not found in sprite template '%s'", animationName, templ->m_name);

	// Check if not already played

	gxAnimation2DInstance* existingInstance = sprite->m_animInstances.Front();
	while (existingInstance && existingInstance->m_animation != anim)
		existingInstance = existingInstance->Next();

	if (existingInstance)
		return;

	// Fade out or kill all other animations

	const f32 weightChangeSpeed = transitionTime == 0.0f ? 0.0f : 1.0f / transitionTime;

	if (mode != gxSpriteAnimationPlayMode_OnceWhenDone && mode != gxSpriteAnimationPlayMode_LoopWhenDone)
	{
		if (transitionTime == 0.0f)
			while (gxAnimation2DInstance* iter = sprite->m_animInstances.PopFront())
				ueDelete(iter, s_data->m_animInstancesPool);
		else
			for (gxAnimation2DInstance* iter = sprite->m_animInstances.Front(); iter; iter = iter->Next())
				iter->m_weightChangeSpeed = -weightChangeSpeed;
	}

	// Create animation instance

	gxAnimation2DInstance* inst = new(s_data->m_animInstancesPool) gxAnimation2DInstance();
	UE_ASSERT(inst);
	inst->m_animation = anim;
	inst->m_mode = mode;
	inst->m_time = 0.0f;
	inst->m_weight = transitionTime == 0.0f ? 1.0f : 0.0f;
	inst->m_weightChangeSpeed = weightChangeSpeed;

	// Add animation instance to sprite

	sprite->m_animInstances.PushBack(inst);
}

void gxSprite_FireAnimationEvents(gxSprite* sprite, gxAnimation2D* anim, f32 oldTime, f32 newTime, f32 dt)
{
	if (!sprite->m_eventCallback)
		return;

	for (u32 i = 0; i < anim->m_numEvents; i++)
	{
		const gxAnimation2DEvent& ev = anim->m_events[i];
		if (oldTime <= ev.m_time && ev.m_time < newTime)
			sprite->m_eventCallback(ev.m_name, sprite->m_userData);
	}
}

void gxSprite_Update(gxSprite* sprite, f32 dt)
{
	UE_ASSERT(s_data);

	ueList<gxAnimation2DInstance> list;
	list.CopyFrom(sprite->m_animInstances);

	ueBool doneAnim = UE_FALSE;
	gxAnimation2DInstance* instWhenDone = NULL;

	while (gxAnimation2DInstance* inst = list.PopBack())
	{
		const f32 prevInstTime = inst->m_time;

		if (inst->m_mode == gxSpriteAnimationPlayMode_OnceWhenDone || inst->m_mode == gxSpriteAnimationPlayMode_LoopWhenDone)
			instWhenDone = inst;
		else
			inst->m_time += dt;
		
		if (inst->m_time >= inst->m_animation->m_totalTime)
		{
			switch (inst->m_mode)
			{
			case gxSpriteAnimationPlayMode_Loop:
				inst->m_time = ueMod(inst->m_time, inst->m_animation->m_totalTime);
				gxSprite_FireAnimationEvents(sprite, inst->m_animation, prevInstTime, inst->m_time, dt);
				doneAnim = UE_TRUE;
				break;
			case gxSpriteAnimationPlayMode_Once:
				gxSprite_FireAnimationEvents(sprite, inst->m_animation, prevInstTime, inst->m_animation->m_totalTime, dt);
				ueDelete(inst, s_data->m_animInstancesPool);
				doneAnim = UE_TRUE;
				continue;
			case gxSpriteAnimationPlayMode_OnceAndFreeze:
				inst->m_time = inst->m_animation->m_totalTime;
				gxSprite_FireAnimationEvents(sprite, inst->m_animation, prevInstTime, inst->m_time, dt);
				break;
			}
		}
		else
			gxSprite_FireAnimationEvents(sprite, inst->m_animation, prevInstTime, inst->m_time, dt);

		inst->m_weight += inst->m_weightChangeSpeed * dt;

		if (inst->m_weight <= 0.0f)
		{
			ueDelete(inst, s_data->m_animInstancesPool);
			continue;
		}

		if (inst->m_weight >= 1.0f)
		{
			inst->m_weight = 1.0f;
			inst->m_weightChangeSpeed = 0.0f;
		}

		sprite->m_animInstances.PushBack(inst);
	}

	// Kick off "when done" instance

	if ((doneAnim || sprite->m_animInstances.Length() == 1) && instWhenDone)
	{
		gxAnimation2DInstance* inst = sprite->m_animInstances.Front();
		while (inst)
		{
			gxAnimation2DInstance* next = inst->Next();
			if (inst != instWhenDone)
			{
				sprite->m_animInstances.Remove(inst);
				ueDelete(inst, s_data->m_animInstancesPool);
			}

			inst = next;
		}

		instWhenDone->m_mode = (instWhenDone->m_mode == gxSpriteAnimationPlayMode_OnceWhenDone) ? gxSpriteAnimationPlayMode_Once : gxSpriteAnimationPlayMode_Loop;
		instWhenDone->m_weight = 1.0f;
	}

	// Start default animation if there's no animations left

	if (sprite->m_template && sprite->m_animInstances.IsEmpty())
		gxSprite_PlayDefaultAnimation(sprite);
}

void gxSprite_BeginDraw(gxSpriteBeginDrawParams* params)
{
	UE_ASSERT(s_data);
	UE_ASSERT(!s_data->m_isDrawing);

	s_data->m_isDrawing = UE_TRUE;
	s_data->m_ctx = params->m_ctx;
	s_data->m_numDrawBatches = 0;

#if defined(GL_MARMALADE)

	IwGxSetOrtho(
		(iwfixed) GL_TO_MARMALADE_COORD(params->m_rect.m_left),
		(iwfixed) GL_TO_MARMALADE_COORD(params->m_rect.m_right),
		(iwfixed) GL_TO_MARMALADE_COORD(params->m_rect.m_top),
		(iwfixed) GL_TO_MARMALADE_COORD(params->m_rect.m_bottom),
		-1, -1);

	if (params->m_globalTransform)
	{
		CIwMat viewTransform;
		glUtils_ToMarmaladeMatrix(&viewTransform, params->m_globalTransform);
		IwGxSetModelMatrix(&viewTransform);
	}
	else
		IwGxSetModelMatrix(&CIwMat::g_Identity);

#else // defined(GL_MARMALADE)

	// Set appropriate render states

	glCtx_SetBlending(params->m_ctx, UE_TRUE);
	glCtx_SetBlendFunc(params->m_ctx, glBlendingFunc_SrcAlpha, glBlendingFunc_InvSrcAlpha);

	glCtx_SetCullMode(params->m_ctx, glCullMode_None);

	glCtx_SetDepthTest(params->m_ctx, UE_FALSE);
	glCtx_SetDepthWrite(params->m_ctx, UE_FALSE);

	// Set orthographic projection

	ueMat44 orthoProj;
	orthoProj.SetOrthoOffCenter(params->m_rect.m_left, params->m_rect.m_right, params->m_rect.m_top, params->m_rect.m_bottom, -1, 1, glCtx_IsRenderingToTexture(params->m_ctx));

	ueMat44 viewProj;
	if (params->m_globalTransform)
		ueMat44::Mul(viewProj, *params->m_globalTransform, orthoProj);
	else
		viewProj = orthoProj;

	glCtx_SetFloat4x4Constant(params->m_ctx, gxCommonConstants::ViewProj, &viewProj);

#endif // defined(GL_MARMALADE)
}

void gxSprite_EndDraw()
{
	UE_ASSERT(s_data);
	UE_ASSERT(s_data->m_isDrawing);

	// Flush queued draw commands

	gxSprite_Flush();

	// Revert default render states

#if !defined(GL_MARMALADE)

	glCtx_SetCullMode(s_data->m_ctx, glCullMode_CCW);

	glCtx_SetDepthTest(s_data->m_ctx, UE_TRUE);
	glCtx_SetDepthWrite(s_data->m_ctx, UE_TRUE);

#endif // defined(GL_MARMALADE)

	s_data->m_isDrawing = UE_FALSE;
}

#if defined(GL_MARMALADE)

UE_INLINE void gxSprite_SetTriangle(u16* index, u16 t0, u16 t1, u16 t2)
{
	index[0] = t0;
	index[1] = t1;
	index[2] = t2;
}

UE_INLINE void gxSprite_SetVertex(CIwSVec3* pos, CIwSVec2* uv, CIwColour* color, f32 x, f32 y, f32 u, f32 v, f32 originX, f32 originY, f32 rotSin, f32 rotCos, u32 colorAsShaderAttr, const f32* uvScale, const f32* uvTranslation)
{
	// Set color

	*color = colorAsShaderAttr;

	// Rotate position around origin

	if (rotSin != 0.0f)
	{
		x -= originX;
		y -= originY;

		f32 newPos[2];
		newPos[0] = -rotSin * y + rotCos * x;
		newPos[1] = rotSin * x + rotCos * y;

		x = newPos[0] + originX;
		y = newPos[1] + originY;
	}

	// Set position

	pos->x = GL_TO_MARMALADE_COORD(x);
	pos->y = GL_TO_MARMALADE_COORD(y);
	pos->z = GL_TO_MARMALADE_COORD(2.0f);

	// Set UVs

	uv->x = GL_TO_MARMALADE_UV(u * uvScale[0] + uvTranslation[0]);
	uv->y = GL_TO_MARMALADE_UV(v * uvScale[1] + uvTranslation[1]);
}

UE_INLINE void gxSprite_SetPolyVertex(CIwSVec3* pos, CIwSVec2* uv, CIwColour* color, f32 x, f32 y, f32 u, f32 v, u32 colorAsShaderAttr, const f32* uvScale, const f32* uvTranslation)
{
	// Set color

	*color = colorAsShaderAttr;

	// Set position

	pos->x = GL_TO_MARMALADE_COORD(x);
	pos->y = GL_TO_MARMALADE_COORD(y);
	pos->z = GL_TO_MARMALADE_COORD(2.0f);

	// Set up UVs within texture atlas

	uv->x = GL_TO_MARMALADE_UV(u * uvScale[0] + uvTranslation[0]);
	uv->y = GL_TO_MARMALADE_UV(v * uvScale[1] + uvTranslation[1]);
}

void gxSprite_Draw(gxSprite* sprite, gxSpriteDrawParams* params)
{
	UE_ASSERT(s_data);
	UE_ASSERT(s_data->m_isDrawing);

	gxSpriteTemplate* templ = sprite ? sprite->m_template : NULL;

	// Set up rectangle if only left & top coordinates were set

	ueRect tmpRect;
	if (!params->m_rect && !params->m_poly)
	{
		u32 width, height;
		gxSprite_GetSize(sprite, width, height);
		tmpRect.Set(params->m_left, params->m_top, params->m_left + width, params->m_top + height);

		params->m_rect = &tmpRect;
	}

	// Check if flush is necessary

	const u32 numVerts = params->m_rect ? 4 : params->m_poly->m_numVerts;
	UE_ASSERT(numVerts <= s_data->m_maxVerts);
	const u32 numIndices = (numVerts - 2) * 3;
	UE_ASSERT(numIndices <= s_data->m_maxIndices);

	if (s_data->m_numDrawCommands == s_data->m_maxDrawCommands ||
		s_data->m_numVerts + numVerts >= s_data->m_maxVerts ||
		s_data->m_numIndices + numIndices >= s_data->m_maxIndices)
		gxSprite_Flush();

	// Add new draw command

	gxSpriteDrawCommand& command = s_data->m_drawCommands[s_data->m_numDrawCommands++];
	command.m_commandIndex = s_data->m_numDrawCommands;
	command.m_layer = params->m_layer;
	command.m_sprite = sprite;
	command.m_firstVertex = s_data->m_numVerts;
	command.m_numVerts = numVerts;
	command.m_firstIndex = s_data->m_numIndices;
	command.m_numIndices = numIndices;
	command.m_layer = params->m_layer;
	command.m_additiveBlending = params->m_additiveBlending;

	// Figure out textures to blend

	ueRect texRect;
	if (params->m_texRect)
		texRect = *params->m_texRect;
	else
		texRect.Set(0, 0, 1, 1);

	if (!templ)
	{
		command.m_textures[0] = sprite ? *sprite->m_defaultTexture : NULL;
		command.m_textureBuffers[0] = sprite ? sprite->m_defaultTextureBuffer : NULL;
		command.m_textures[1] = NULL;
		command.m_textureBuffers[1] = NULL;
		command.m_lerpScale = 0.0f;
	}
	else
	{
		// Find animation instance with largest weight

		gxAnimation2DInstance* animInst = NULL;
		for (gxAnimation2DInstance* iter = sprite->m_animInstances.Front(); iter; iter = iter->Next())
			if (iter->m_mode != gxSpriteAnimationPlayMode_OnceWhenDone && iter->m_mode != gxSpriteAnimationPlayMode_LoopWhenDone &&
				(!animInst || iter->m_weight > animInst->m_weight))
				animInst = iter;
		UE_ASSERT(animInst);

		// Determine frame indices to blend

		gxAnimation2D* anim = animInst->m_animation;

		if (anim->m_numFrames == 1)
		{
			const gxAnimation2DFrame& frame = anim->m_frames[0];

			command.m_textures[0] = *frame.m_texture;
			command.m_textureBuffers[0] = frame.m_texture->GetBuffer();
			command.m_textures[1] = NULL;
			command.m_textureBuffers[1] = NULL;
			command.m_lerpScale = 0.0f;
		}
		else
		{
			const f32 numFramesF = (f32) anim->m_numFrames;
			const f32 frameIndexF = numFramesF * (animInst->m_time / anim->m_totalTime);
			const f32 firstFrameIndexF = ueFloor(frameIndexF);

			const u32 firstFrameIndex = ueClamp((u32) firstFrameIndexF, 0U, anim->m_numFrames - 1);
			const u32 nextFrameIndex = (firstFrameIndex + 1) % anim->m_numFrames;

			const gxAnimation2DFrame& firstFrame = anim->m_frames[firstFrameIndex];
			const gxAnimation2DFrame& nextFrame = anim->m_frames[nextFrameIndex];

			command.m_textures[0] = *firstFrame.m_texture;
			command.m_textureBuffers[0] = firstFrame.m_texture->GetBuffer();
			command.m_textures[1] = *nextFrame.m_texture;
			command.m_textureBuffers[1] = nextFrame.m_texture->GetBuffer();
			command.m_lerpScale = anim->m_blendFrames ? (frameIndexF - firstFrameIndexF) : 0.0f;

			command.m_blendFrames = anim->m_blendFrames;
		}
	}

	// Build sprite geometry

	CIwSVec3* pos = s_data->m_pos + s_data->m_numVerts;
	CIwColour* color = s_data->m_color + s_data->m_numVerts;
	CIwSVec2* uv = s_data->m_uv + s_data->m_numVerts;
	u16* indices = s_data->m_drawIndices + s_data->m_numIndices;

	const f32 rotSin = ueSin(params->m_rotation);
	const f32 rotCos = ueCos(params->m_rotation);

	ueColor32 rgba = params->m_color;
	rgba.SetAlphaF((1.0f - command.m_lerpScale) * params->m_color.AlphaF());
	u32 colorAsShaderAttr = rgba.AsShaderAttr();

	f32 uvScale[2] = {1.0f, 1.0f};
	f32 uvTranslation[2] = {0.0f, 0.0f};
	if (command.m_textures[0] && command.m_textures[0]->IsInAtlas())
	{
		const gxTextureAtlasEntry* entry = command.m_textures[0]->GetAtlasEntry();
		uvScale[0] = entry->m_scale[0];
		uvScale[1] = entry->m_scale[1];
		uvTranslation[0] = entry->m_translation[0];
		uvTranslation[1] = entry->m_translation[1];
	}

	if (params->m_rect)
	{
		const ueRect rect = *params->m_rect;

		const f32 worldOrigin[2] =
		{
			params->m_origin[0] + rect.CenterX(),
			params->m_origin[1] + rect.CenterY()
		};

		gxSprite_SetVertex(pos, uv, color, rect.m_left, rect.m_top, texRect.m_left, texRect.m_top, worldOrigin[0], worldOrigin[1], rotSin, rotCos, colorAsShaderAttr, uvScale, uvTranslation);
		gxSprite_SetVertex(pos + 1, uv + 1, color + 1, rect.m_right, rect.m_top, texRect.m_right, texRect.m_top, worldOrigin[0], worldOrigin[1], rotSin, rotCos, colorAsShaderAttr, uvScale, uvTranslation);
		gxSprite_SetVertex(pos + 2, uv + 2, color + 2, rect.m_right, rect.m_bottom, texRect.m_right, texRect.m_bottom, worldOrigin[0], worldOrigin[1], rotSin, rotCos, colorAsShaderAttr, uvScale, uvTranslation);
		gxSprite_SetVertex(pos + 3, uv + 3, color + 3, rect.m_left, rect.m_bottom, texRect.m_left, texRect.m_bottom,  worldOrigin[0], worldOrigin[1], rotSin, rotCos, colorAsShaderAttr, uvScale, uvTranslation);

		gxSprite_SetTriangle(indices + 0, 0, 1, 2);
		gxSprite_SetTriangle(indices + 3, 0, 2, 3);
	}
	else
	{
		UE_ASSERT_MSG(params->m_rotation == 0.0f, "Rotation must be 0.0f for polygons.");

		const gxSpriteDrawParams::Poly* poly = params->m_poly;

		const u32 posStride = poly->m_posStride ? poly->m_posStride : sizeof(f32) * 2;
		const u32 texStride = poly->m_texStride ? poly->m_texStride : sizeof(f32) * 2;

		for (u32 i = 0; i < poly->m_numVerts; i++)
		{
			const f32* posF32 = (const f32*) ((u8*) poly->m_pos + posStride * i);
			const f32* texCoord = (const f32*) ((u8*) poly->m_tex + texStride * i);

			gxSprite_SetPolyVertex(pos + i, uv + i, color + i, posF32[0], posF32[1], texCoord[0], texCoord[1], colorAsShaderAttr, uvScale, uvTranslation);
		}

		if (poly->m_indices)
		{
			const u16* indicesPtr = poly->m_indices;
			for (u32 i = 2; i < poly->m_numVerts; i++)
			{
				gxSprite_SetTriangle(indices + (i - 2) * 3, indicesPtr[0], indicesPtr[1], indicesPtr[2]);
				indicesPtr += 3;
			}
		}
		else
			for (u32 i = 2; i < poly->m_numVerts; i++)
				gxSprite_SetTriangle(indices + (i - 2) * 3, 0, i - 1, i);
	}

	command.m_material = sprite ? sprite->m_materials[0] : params->m_material;

	s_data->m_numVerts += numVerts;
	s_data->m_numIndices += numIndices;

	// Optionally queue up 2nd layer as a separate draw command

	if (command.m_textures[1] && command.m_blendFrames)
	{
		gxSpriteDrawCommand& command2 = s_data->m_drawCommands[s_data->m_numDrawCommands++];
		command2.m_commandIndex = s_data->m_numDrawCommands;
		command2.m_layer = params->m_layer;
		command2.m_sprite = sprite;
		command2.m_firstVertex = s_data->m_numVerts;
		command2.m_numVerts = numVerts;
		command2.m_firstIndex = command.m_firstIndex;
		command2.m_numIndices = numIndices;
		command2.m_layer = params->m_layer + 0.001f; // Draw right on top
		command2.m_textures[0] = command.m_textures[1];
		command2.m_material = sprite->m_materials[1];

		CIwSVec3* pos2 = s_data->m_pos + s_data->m_numVerts;
		CIwColour* color2 = s_data->m_color + s_data->m_numVerts;
		CIwSVec2* uv2 = s_data->m_uv + s_data->m_numVerts;

		ueMemCpy(pos2, pos, sizeof(CIwSVec3) * numVerts);
		ueMemCpy(uv2, uv, sizeof(CIwSVec2) * numVerts);

		rgba = params->m_color;
		rgba.SetAlphaF(command.m_lerpScale * params->m_color.AlphaF());
		colorAsShaderAttr = rgba.AsShaderAttr();
		for (u32 i = 0; i < numVerts; i++)
			color2[i] = colorAsShaderAttr;

		s_data->m_numVerts += numVerts;
		s_data->m_numIndices += numIndices;
	}

	if (params->m_rect == &tmpRect)
		params->m_rect = NULL;
}

void gxSprite_Flush()
{
	UE_ASSERT(s_data);
	UE_ASSERT(s_data->m_isDrawing);

	// Sort by layer and rendering state

	gxSpriteDrawCommand::SortCmp sortCmp;
	ueSort(s_data->m_drawCommands, s_data->m_numDrawCommands, sortCmp);

	// Draw

	gxSpriteDrawCommand::BatchCmp commandCmp;

	u32 i = 0;
	while (i < s_data->m_numDrawCommands)
	{
		gxSpriteDrawCommand& command = s_data->m_drawCommands[i];

		// Set material

		command.m_material->SetAlphaMode(command.m_additiveBlending ? CIwMaterial::ALPHA_ADD : CIwMaterial::ALPHA_BLEND);

		if (command.m_textures[0])
		{
			CIwTexture* tex = glTextureBuffer_GetIwTexture(command.m_textures[0]->GetBuffer());
			command.m_material->SetTexture(tex);
		}
		IwGxSetMaterial(command.m_material);

		// Do multiple draw calls with the same material

		u32 count = 0;
		while (i + count < s_data->m_numDrawCommands)
		{
			gxSpriteDrawCommand& nextCommand = s_data->m_drawCommands[i + count];
			if (count && commandCmp(nextCommand, command))
				break;

			// Set draw data

			CIwSVec3* pos = s_data->m_pos + nextCommand.m_firstVertex;
			CIwSVec2* uv = s_data->m_uv + nextCommand.m_firstVertex;
			CIwColour* color = s_data->m_color + nextCommand.m_firstVertex;

			IwGxSetVertStream(pos, nextCommand.m_numVerts);
			IwGxSetUVStream(uv);
			IwGxSetColStream(color, nextCommand.m_numVerts);

			// Draw indexed primitive

			u16* indices = s_data->m_drawIndices + nextCommand.m_firstIndex;

			IwGxDrawPrims(IW_GX_TRI_LIST, indices, nextCommand.m_numIndices);

			// Move to the next command

			count++;
		}

		i += count;
		IwGxFlush();

		// Update stats

		s_data->m_numDrawBatches++;
	}

	// TEMP
	//IwGxSetDebugFlags(IW_GX_DEBUG_WIREFRAME_F); 

	s_data->m_numDrawCommands = 0;
	s_data->m_numVerts = 0;
	s_data->m_numIndices = 0;
}

#else

UE_INLINE void gxSprite_SetVertex(gxSpriteVertex* vertex, f32 x, f32 y, f32 u, f32 v, const f32* uvScale, const f32* uvTranslation)
{
	vertex->pos[0] = x;
	vertex->pos[1] = y;
	vertex->uv0[0] = u * uvScale[0] + uvTranslation[0];
	vertex->uv0[1] = v * uvScale[1] + uvTranslation[1];
	vertex->uv1[0] = vertex->uv0[0];
	vertex->uv1[1] = vertex->uv0[1];
}

UE_INLINE void gxSprite_FinalizeVertex(gxSpriteVertex* vertex, f32 originX, f32 originY, f32 rotation, u32 colorAsShaderAttr, f32 lerp)
{
	vertex->lerp = lerp;
	vertex->color = colorAsShaderAttr;

	// Rotate around origin

	if (rotation != 0.0f)
	{
		vertex->pos[0] -= originX;
		vertex->pos[1] -= originY;

		const f32 sin = ueSin(rotation);
		const f32 cos = ueCos(rotation);

		f32 newPos[2];
		newPos[0] = -sin * vertex->pos[1] + cos * vertex->pos[0];
		newPos[1] = sin * vertex->pos[0] + cos * vertex->pos[1];

		vertex->pos[0] = newPos[0] + originX;
		vertex->pos[1] = newPos[1] + originY;
	}
}

UE_INLINE void gxSprite_SetPolyVertex(gxSpriteVertex* vertex, u32 colorAsShaderAttr, f32 lerp, f32 x, f32 y, f32 u, f32 v, const f32* uvScale, const f32* uvTranslation)
{
	vertex->pos[0] = x;
	vertex->pos[1] = y;
	vertex->color = colorAsShaderAttr;
	vertex->lerp = lerp;
	vertex->uv0[0] = u * uvScale[0] + uvTranslation[0];
	vertex->uv0[1] = v * uvScale[1] + uvTranslation[1];
	vertex->uv1[0] = vertex->uv0[0];
	vertex->uv1[1] = vertex->uv0[1];
}

UE_INLINE void gxSprite_SetTriangle(u16* index, u16 t0, u16 t1, u16 t2)
{
	index[0] = t0;
	index[1] = t1;
	index[2] = t2;
}

void gxSprite_Draw(gxSprite* sprite, gxSpriteDrawParams* params)
{
	UE_ASSERT(s_data);
	UE_ASSERT(s_data->m_isDrawing);

	// Set up rectangle if only left & top coordinates were set

	ueRect tmpRect;
	if (!params->m_rect && !params->m_poly)
	{
		u32 width, height;
		gxSprite_GetSize(sprite, width, height);
		tmpRect.Set(params->m_left, params->m_top, params->m_left + width, params->m_top + height);

		params->m_rect = &tmpRect;
	}

	// Flush previous draw commands if needed

	const u32 numVerts = params->m_rect ? 4 : params->m_poly->m_numVerts;
	UE_ASSERT(numVerts <= s_data->m_maxVerts);
	const u32 numIndices = (numVerts - 2) * 3;
	UE_ASSERT(numIndices <= s_data->m_maxIndices);

	if (s_data->m_numDrawCommands == s_data->m_maxDrawCommands ||
		s_data->m_numVerts + numVerts >= s_data->m_maxVerts ||
		s_data->m_numIndices + numIndices >= s_data->m_maxIndices)
		gxSprite_Flush();

	// Add new draw command

	gxSpriteDrawCommand& command = s_data->m_drawCommands[s_data->m_numDrawCommands++];
	command.m_commandIndex = s_data->m_numDrawCommands;
	command.m_layer = params->m_layer;
	command.m_blendFrames = UE_FALSE;
	command.m_sprite = sprite;
	command.m_additiveBlending = params->m_additiveBlending;

	ueRect texRect;
	if (params->m_texRect)
		texRect = *params->m_texRect;
	else
		texRect.Set(0, 0, 1, 1);

	// Determine textures to blend

	gxSpriteTemplate* templ = sprite->m_template;

	if (!templ)
	{
		command.m_textures[0] = *sprite->m_defaultTexture;
		command.m_textureBuffers[0] = gxTexture_GetBuffer(command.m_textures[0]);
		command.m_textures[1] = NULL;
		command.m_textureBuffers[1] = NULL;
		command.m_lerpScale = 0.0f;
	}
	else
	{
		// Find animation instance with largest weight

		gxAnimation2DInstance* animInst = NULL;
		for (gxAnimation2DInstance* iter = sprite->m_animInstances.Front(); iter; iter = iter->Next())
			if (iter->m_mode != gxSpriteAnimationPlayMode_OnceWhenDone && iter->m_mode != gxSpriteAnimationPlayMode_LoopWhenDone &&
				(!animInst || iter->m_weight > animInst->m_weight))
				animInst = iter;
		UE_ASSERT(animInst);

		// Determine frame indices to blend

		gxAnimation2D* anim = animInst->m_animation;

		if (anim->m_numFrames == 1)
		{
			const gxAnimation2DFrame& frame = anim->m_frames[0];

			command.m_textures[0] = *frame.m_texture;
			command.m_textureBuffers[0] = gxTexture_GetBuffer(command.m_textures[0]);
			command.m_textures[1] = NULL;
			command.m_textureBuffers[1] = NULL;
			command.m_lerpScale = 0.0f;
		}
		else
		{
			const f32 numFramesF = (f32) anim->m_numFrames;
			const f32 frameIndexF = numFramesF * (animInst->m_time / anim->m_totalTime);
			const f32 firstFrameIndexF = ueFloor(frameIndexF);

			const u32 firstFrameIndex = ueClamp((u32) firstFrameIndexF, 0U, anim->m_numFrames - 1);
			const u32 nextFrameIndex = (firstFrameIndex + 1) % anim->m_numFrames;

			const gxAnimation2DFrame& firstFrame = anim->m_frames[firstFrameIndex];
			const gxAnimation2DFrame& nextFrame = anim->m_frames[nextFrameIndex];

			command.m_textures[0] = *firstFrame.m_texture;
			command.m_textureBuffers[0] = gxTexture_GetBuffer(command.m_textures[0]);
			command.m_textures[1] = *nextFrame.m_texture;
			command.m_textureBuffers[1] = gxTexture_GetBuffer(command.m_textures[1]);
			command.m_lerpScale = frameIndexF - firstFrameIndexF;

			command.m_blendFrames = anim->m_blendFrames;
		}
	}

	// Set up geometry for the draw command

	command.m_firstVertex = s_data->m_numVerts;
	s_data->m_numVerts += numVerts;
	command.m_firstIndex = s_data->m_numIndices;
	s_data->m_numIndices += numIndices;
	command.m_color = params->m_color;

	gxSpriteVertex* verts = s_data->m_drawVerts + command.m_firstVertex;
	u16* indices = s_data->m_drawIndices + command.m_firstIndex;

	// Determine uv scale and translation depending on whether the texture is part of atlas

	f32 uvScale[2] = {1.0f, 1.0f};
	f32 uvTranslation[2] = {0.0f, 0.0f};
	if (command.m_textures[0] && gxTexture_IsInAtlas(command.m_textures[0]))
	{
		const gxTextureAtlasEntry* entry = gxTexture_GetAtlasEntry(command.m_textures[0]);
		uvScale[0] = entry->m_scale[0];
		uvScale[1] = entry->m_scale[1];
		uvTranslation[0] = entry->m_translation[0];
		uvTranslation[1] = entry->m_translation[1];
	}

	const u32 colorAsShaderAttr = params->m_color.AsShaderAttr();

	if (params->m_rect)
	{
		const ueRect rect = *params->m_rect;

		gxSprite_SetVertex(verts + 0, rect.m_left, rect.m_top, texRect.m_left, texRect.m_top, uvScale, uvTranslation);
		gxSprite_SetVertex(verts + 1, rect.m_right, rect.m_top, texRect.m_right,texRect.m_top, uvScale, uvTranslation);
		gxSprite_SetVertex(verts + 2, rect.m_right, rect.m_bottom, texRect.m_right, texRect.m_bottom, uvScale, uvTranslation);
		gxSprite_SetVertex(verts + 3, rect.m_left, rect.m_bottom, texRect.m_left, texRect.m_bottom, uvScale, uvTranslation);
		command.m_numVerts = 4;

		const f32 worldOrigin[2] =
		{
			params->m_origin[0] + rect.CenterX(),
			params->m_origin[1] + rect.CenterY()
		};

		for (u32 i = 0; i < 4; i++)
			gxSprite_FinalizeVertex(verts + i, worldOrigin[0], worldOrigin[1], params->m_rotation, colorAsShaderAttr, command.m_lerpScale);

		gxSprite_SetTriangle(indices + 0, 0, 1, 2);
		gxSprite_SetTriangle(indices + 3, 0, 2, 3);
		command.m_numIndices = 6;
	}
	else
	{
		UE_ASSERT_MSG(params->m_rotation == 0.0f, "Rotation must be 0.0f for polygons.");

		const gxSpriteDrawParams::Poly* poly = params->m_poly;

		const u32 posStride = poly->m_posStride ? poly->m_posStride : sizeof(f32) * 2;
		const u32 texStride = poly->m_texStride ? poly->m_texStride : sizeof(f32) * 2;

		for (u32 i = 0; i < poly->m_numVerts; i++)
		{
			const f32* pos = (const f32*) ((u8*) poly->m_pos + posStride * i);
			const f32* texCoord = (const f32*) ((u8*) poly->m_tex + texStride * i);

			gxSprite_SetPolyVertex(verts + i, colorAsShaderAttr, command.m_lerpScale, pos[0], pos[1], texCoord[0], texCoord[1], uvScale, uvTranslation);
		}

		command.m_numVerts = poly->m_numVerts;
		command.m_numIndices = (poly->m_numVerts - 2) * 3;

		if (poly->m_indices)
		{
			const u16* indicesPtr = poly->m_indices;
			for (u32 i = 2; i < poly->m_numVerts; i++)
			{
				gxSprite_SetTriangle(indices + (i - 2) * 3, indicesPtr[0], indicesPtr[1], indicesPtr[2]);
				indicesPtr += 3;
			}
		}
		else
			for (u32 i = 2; i < poly->m_numVerts; i++)
				gxSprite_SetTriangle(indices + (i - 2) * 3, 0, i - 1, i);
	}

	if (params->m_rect == &tmpRect)
		params->m_rect = NULL;
}

UE_INLINE void gxSprite_ApplyTextureAtlasTransformToUVs(gxTexture* tex, u32 vertexCount, f32* uv, u32 vertexStride)
{
	if (!tex || !gxTexture_IsInAtlas(tex))
		return;

	const gxTextureAtlasEntry* atlasEntry = gxTexture_GetAtlasEntry(tex);

	const f32 scale[2] = { atlasEntry->m_scale[0], atlasEntry->m_scale[1] };
	const f32 translation[2] = { atlasEntry->m_translation[0], atlasEntry->m_translation[1] };

	for (u32 i = 0; i < vertexCount; i++)
	{
		uv[0] = uv[0] * scale[0] + translation[0];
		uv[1] = uv[1] * scale[1] + translation[1];
		((u8*&) uv) += vertexStride;
	}
}

void gxSprite_Flush()
{
	UE_ASSERT(s_data);
	UE_ASSERT(s_data->m_isDrawing);

	// Sort by layer and rendering state

	gxSpriteDrawCommand::SortCmp sortCmp;
	ueSort(s_data->m_drawCommands, s_data->m_numDrawCommands, sortCmp);

	// Draw

	gxSpriteDrawCommand::BatchCmp commandCmp;

	for (u32 i = 0; i < s_data->m_numDrawCommands;)
	{
		gxSpriteDrawCommand& command = s_data->m_drawCommands[i];

		// Set render states

		glCtx_SetSamplerConstant(s_data->m_ctx, gxCommonConstants::ColorMap, command.m_textureBuffers[0], &glSamplerParams::DefaultClamp);
		if (command.m_textures[1])
			glCtx_SetSamplerConstant(s_data->m_ctx, gxCommonConstants::ColorMap2, command.m_textureBuffers[1], &glSamplerParams::DefaultClamp);

		if (command.m_additiveBlending)
			glCtx_SetBlendFunc(s_data->m_ctx, glBlendingFunc_SrcAlpha, glBlendingFunc_One);
		else
			glCtx_SetBlendFunc(s_data->m_ctx, glBlendingFunc_SrcAlpha, glBlendingFunc_InvSrcAlpha);

		// Set shader program

		glCtx_SetProgram(s_data->m_ctx, (command.m_textures[1] && command.m_blendFrames ? s_data->m_animatedProgram : s_data->m_program).GetProgram());

		// Determine number of commands with matching render state

		u32 batchNumVerts = 0;
		u32 batchNumIndices = 0;

		u32 count = 0;
		while (i + count < s_data->m_numDrawCommands)
		{
			gxSpriteDrawCommand& nextCommand = s_data->m_drawCommands[i + count];
			if (commandCmp(nextCommand, command))
				break;

			batchNumVerts += nextCommand.m_numVerts;
			batchNumIndices += nextCommand.m_numIndices;
			count++;
		}

		// Allocate index and vertex buffer memory

		glVertexBufferFactory::LockedChunk vbChunk;
		if (!s_data->m_VB.Allocate(batchNumVerts * sizeof(gxSpriteVertex), &vbChunk))
			break;

		glIndexBufferFactory::LockedChunk ibChunk;
		if (!s_data->m_IB.Allocate(batchNumIndices, &ibChunk))
		{
			s_data->m_VB.Unlock(&vbChunk);
			break;
		}

		// Generate final geometry for the whole batch

		gxSpriteVertex* dstVerts = (gxSpriteVertex*) vbChunk.m_data;
		u32 dstVertexCounter = 0;

		u16* dstIndices = (u16*) ibChunk.m_data;

		for (u32 j = i; j < i + count; j++)
		{
			gxSpriteDrawCommand& command = s_data->m_drawCommands[j];

			const gxSpriteVertex* srcVerts = s_data->m_drawVerts + command.m_firstVertex;
			const u16* srcIndices = s_data->m_drawIndices + command.m_firstIndex;

			ueMemCpy(dstVerts, srcVerts, sizeof(gxSpriteVertex) * command.m_numVerts);
			for (u32 k = 0; k < command.m_numIndices; k++)
				dstIndices[k] = srcIndices[k] + dstVertexCounter;

			dstVertexCounter += command.m_numVerts;
			dstVerts += command.m_numVerts;
			dstIndices += command.m_numIndices;
		}

		// Account for texture atlasing

//		gxSprite_ApplyTextureAtlasTransformToUVs(command.m_textures[0], dstVertexCounter, ((gxSpriteVertex*) vbChunk.m_data)->uv0, sizeof(gxSpriteVertex));
//		gxSprite_ApplyTextureAtlasTransformToUVs(command.m_textures[1], dstVertexCounter, ((gxSpriteVertex*) vbChunk.m_data)->uv1, sizeof(gxSpriteVertex));

		// Unlock draw buffers

		s_data->m_VB.Unlock(&vbChunk);
		s_data->m_IB.Unlock(&ibChunk);

		// Draw

		glCtx_SetStream(s_data->m_ctx, 0, vbChunk.m_VB, s_data->m_SF, vbChunk.m_offset);
		glCtx_SetIndices(s_data->m_ctx, ibChunk.m_IB);
		glCtx_DrawIndexed(s_data->m_ctx, glPrimitive_TriangleList, 0, 0, batchNumVerts, ibChunk.m_firstIndex, batchNumIndices);

		// Skip to next batch

		i += count;
	}

	// Reset command list

	s_data->m_numDrawCommands = 0;
	s_data->m_numVerts = 0;
	s_data->m_numIndices = 0;

	// Reset render states

	glCtx_SetBlendFunc(s_data->m_ctx, glBlendingFunc_SrcAlpha, glBlendingFunc_InvSrcAlpha);
}

#endif // defined(GL_MARMALADE)