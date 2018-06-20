#ifndef GX_SOFTWARE_DEPTH_RASTERIZER_H
#define GX_SOFTWARE_DEPTH_RASTERIZER_H

TODO: This is an old version

enum RasterMode
{
    RasterMode_MaxDepth = 0,
    RasterMode_MinDepth,
    RasterMode_ExactDepth,

    RasterMode_MAX
};

void gxSoftwareDepthRasterizer_Startup(ueAllocator* allocator);
void gxSoftwareDepthRasterizer_Shutdown();

void	gxSoftwareDepthRasterizer_SetBufferDimensions(s32 width, s32 height);
void	gxSoftwareDepthRasterizer_Clear();
s32		gxSoftwareDepthRasterizer_DrawTriangle(const f32* pos0, const f32* pos1, const f32* pos2, RasterMode rasterMode = RasterMode_MinDepth);
ueBool	gxSoftwareDepthRasterizer_CheckTriangle(const f32* pos0, const f32* pos1, const f32* pos2, RasterMode rasterMode = RasterMode_MaxDepth);
s32		gxSoftwareDepthRasterizer_DrawAABB(const f32* minPos, const f32* maxPos, RasterMode rasterMode = RasterMode_MinDepth);
ueBool	gxSoftwareDepthRasterizer_CheckAABB(const f32* min, const f32* max, RasterMode rasterMode = RasterMode_MaxDepth);

#endif // GX_SOFTWARE_DEPTH_RASTERIZER_H