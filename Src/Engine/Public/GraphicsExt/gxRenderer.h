#ifndef GX_RENDERER_H
#define GX_RENDERER_H

struct gxPacket;
struct gxContext;

typedef void (gxDrawFunc*)(gxPacket* packet, gxContext* ctx);

enum gxPass
{
	gxPass_Depth = 0,
	gxPass_Occluders,
	gxPass_Occludees,
	gxPass_Shadow,
	gxPass_Opaque,
	gxPass_Transparent,
	gxPass_Velocity,
	gxPass_Light,
	gxPass_Debug,

	gxPass_MAX
};

#define GX_SORT_KEY_SIZE	1

struct gxPacket
{
	u32 m_sortKey[GX_SORT_KEY_SIZE];

	gxDrawFunc m_drawFunc;
	void* m_userData;

	gxPacket* m_next;

#ifdef UE_DEBUG
	UE_INLINE gxPacket() :
		m_drawFunc(NULL),
		m_userData(NULL),
		m_next(NULL)
	{}
#endif

	struct CmpPtrs
	{
		UE_INLINE s32 operator (const gxPacket** a, const gxPacket** b) const
		{
#if GX_SORT_KEY_SIZE = 1
			return (*a)->m_sortKey[0] - (*b)->m_sortKey[1];
#elif GX_SORT_KEY_SIZE == 2
			const s32 cmp = (*a)->m_sortKey[0] - (*b)->m_sortKey[0];
			return cmp ? cmp : ((*a)->m_sortKey[1] - (*b)->m_sortKey[1]);
#else
			#error "Unsupported sort key size"
#endif
		}
	};
};

struct gxContext
{
	u32 m_passIndex;
	gxFrame* m_frame;

	gxContext() :
		m_passIndex(-1),
		m_frame(NULL)
	{}
};

struct gxPass
{
	ueBool m_isUsed;
	gxPacket* m_packets;

	gxPass() :
		m_isUsed(UE_FALSE),
		m_packets(NULL)
	{}
};

typedef ueConvex<GX_MAX_POLY_OCCLUDER_PLANES> gxPolyOccluder;

struct gxFrame
{
	ueMat44 m_view;
	ueMat44 m_proj;
	ueMat44 m_viewProj;

	ueFrustum m_frustum;
	ueVector<gxPolyOccluder> m_polyOccluders;
	ueVector<ueBox> m_boxOccluders;

	gxPass m_passes[gxPass_MAX];

	static void* m_memory;
	static u32 m_memorySize;
	static u32 m_memoryPointer;

	ueVector<gaLightComponent*> m_lights;
	ueVector<gaPortalComponent*> m_portals;

	static void StaticInitialize(void* memory, u32 memorySize)
	{
		m_memory = memory;
		m_memorySize = memorySize;
		m_memoryPointer = 0;
	}

	void InitializeMemory(u32 maxLights = 1024, u32 maxPortals = 128, u32 maxBoxOccluders = 16, u32 maxPolyOccluders = 16)
	{
		const u32 lightsMemory = m_lights.CalcMemReq(maxLights);
		m_lights.InitMem(AllocMemory(lightsMemory), lightsMemory, maxLights);

		const u32 portalsMemory = m_portals.CalcMemReq(maxPortals);
		m_portals.InitMem(AllocMemory(portalsMemory), portalsMemory, maxPortals);

		const u32 boxOccludersMemory = m_boxOccluders.CalcMemReq(maxBoxOccluders);
		m_boxOccluders.InitMem(AllocMemory(boxOccludersMemory), boxOccludersMemory, maxBoxOccluders);

		const u32 polyOccludersMemory = m_polyOccluders.CalcMemReq(maxPolyOccluders);
		m_polyOccluders.InitMem(AllocMemory(polyOccludersMemory), polyOccludersMemory, maxPolyOccluders);
	}

	template <typename PACKET_TYPE>
	UE_INLINE PACKET_TYPE* AddPacket(gxPass pass, u32 size = sizeof(PACKET_TYPE))
	{
		if (void* memory = AllocMemory(size))
		{
			gxPacket* packet = new(memory) PACKET_TYPE();
			packet->m_next = m_passes[pass].m_packets;
			m_passes[pass].m_packets = packet;
			return packet;
		}
		return NULL;
	}

	UE_INLINE ueBool HasPass(gxPass pass) const { m_passes[pass].m_isUsed; }

	void SortPackets();

private:
	static UE_INLINE void* AllocMemory(u32 size)
	{
		size = ueAlignPow2(size, sizeof(u32));
		if (m_memoryPointer + size > m_memorySize)
			return NULL;
		void* ret = (u8*) m_memory + m_memoryPointer;
		m_memoryPointer += size;
		return ret;
	}
};

struct gxViewDesc
{
	enum Type
	{
		Type_MainView = 0,
		Type_StencilReflection,
		Type_TextureReflection,
		Type_MAX
	};

	Type m_type;
};

enum gxRenderResult
{
	gaRenderResult_Color = 0,
	gaRenderResult_Depth,
	gaRenderResult_Normals,
	gaRenderResult_Shadow,
	gaRenderResult_Velocity,

	gaRenderResult_MAX
};

struct gxRenderResult
{
	// Input

	ueBool m_isRequired[gaRenderResult_MAX];

	// Output

	glRenderBuffer* m_buffers[gaRenderResult_MAX];

	gxRenderResult()
	{
		for (u32 i = 0; i < gxRenderResult_MAX; i++)
		{
			m_isRequired[i] = UE_FALSE;
			m_buffers[i] = NULL;
		}
	}
};

class gxRenderer
{
public:
	void RenderScene(gaScene* scene, gaCamera* camera, gxRenderResult& result, gxRenderResult& rightEyeResult);

private:
	enum Mode
	{
		Mode_Forward = 0,
		Mode_Deferred,
		Mode_LightPrePass,

		Mode_MAX
	};

	enum StereoMode
	{
		StereoMode_None = 0,
		StereoMode_SeparateColors,

		StereoMode_MAX
	};

	struct StereoParams
	{
		StereoMode m_mode;
		f32 m_eyeOffset;
		f32 m_focusDistance;

		u32 m_leftEyeColorMask;
		u32 m_rightEyeColorMask;

		ueBool m_useReprojection;
	};

	struct RendererParams
	{
		ueBool m_enableDepthPrepass;
		ueBool m_enableSoftwareOcclusionCulling;
	};

	struct ForwardRendererParams : RendererParams
	{
	};

	Mode m_mode;
	StereoParams m_stereo;
	ForwardRendererParams m_forward;
	u32 m_maxReflectionLevel;
	u32 m_maxShadowLevel;
	gxFogDesc m_fog;
	gxToneMapping m_toneMapping;
	gxColorEncoding m_colorEncoding;

	u32 m_frameIndex;
};

#endif // GX_RENDERER_H