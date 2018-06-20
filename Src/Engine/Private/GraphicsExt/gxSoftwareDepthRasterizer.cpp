#include "rdSoftwareDepthRasterizer.h"

#define MAX_DEPTH (1 << 16)

s32 width = 0;
s32 height = 0;
f32 widthF = 0;
f32 heightF = 0;
f32* buffer = NULL;

struct Pos
{
    s32 x;
    s32 y;
    f32 z;
};

void rdSoftwareDepthRasterizer_Startup(ueAllocator* allocator)
{
	s_allocator = allocator;
}

void rdSoftwareDepthRasterizer_Shutdown()
{
    if (buffer)
    {
        free(buffer);
        buffer = NULL;
    }
}

void rdSoftwareDepthRasterizer_SetBufferDimensions(s32 _width, s32 _height)
{
    if (buffer && (width != _width || height != _height))
        free(buffer);

    width = _width;
    height = _height;
    buffer = (f32*) malloc(width * height * sizeof(f32));
}

void rdSoftwareDepthRasterizer_ToIntCoords(const f32* in, Pos& out)
{
    out.x = (s32) (in[0] * widthF + 0.5f);
    out.y = (s32) (in[1] * heightF + 0.5f);
    out.z = in[2];
}

s32 rdSoftwareDepthRasterizer_DrawTriangle(const f32* _pos0, const f32* _pos1, const f32* _pos2, RasterMode rasterMode)
{
    Pos v[3];
    rdSoftwareDepthRasterizer_ToIntCoords(_pos0, v[0]);
    rdSoftwareDepthRasterizer_ToIntCoords(_pos1, v[1]);
    rdSoftwareDepthRasterizer_ToIntCoords(_pos2, v[2]);

    s32 minX = min3(v[0].x, v[1].x, v[2].x);
    s32 maxX = max3(v[0].x, v[1].x, v[2].x);
    s32 minY = min3(v[0].y, v[1].y, v[2].y);
    s32 maxY = max3(v[0].y, v[1].y, v[2].y);
    f32 minZ = min3(v[0].z, v[1].z, v[2].z);
    f32 maxZ = max3(v[0].z, v[1].z, v[2].z);

    if (minX >= width || maxX < 0 || minY >= height || maxY < 0 || minZ >= 1.0f || maxZ < 0.0f)
        return 0;

    minX = max(0, minX);
    maxX = min(width, maxX);
    minY = max(0, minY);
    maxY = min(height, maxY);
    minZ = max(0.0f, minZ);
    maxZ = min(1.0f, maxZ);

    // Fab(x, y) =     Ax       +       By     +      C              = 0
    // Fab(x, y) = (ya - yb)x   +   (xb - xa)y + (xa * yb - xb * ya) = 0
    // Compute A = (ya - yb) for the 3 line segments that make up each triangle
    s32 A0 = v[1].y - v[2].y;
    s32 A1 = v[2].y - v[0].y;
    s32 A2 = v[0].y - v[1].y;

    // Compute B = (xb - xa) for the 3 line segments that make up each triangle
    s32 B0 = v[2].x - v[1].x;
    s32 B1 = v[0].x - v[2].x;
    s32 B2 = v[1].x - v[0].x;

    // Compute C = (xa * yb - xb * ya) for the 3 line segments that make up each triangle
    s32 C0 = v[1].x * v[2].y - v[2].x * v[1].y;
    s32 C1 = v[2].x * v[0].y - v[0].x * v[2].y;
    s32 C2 = v[0].x * v[1].y - v[1].x * v[0].y;

    // Compute triangle area
    s32 triArea = A0 * v[0].x + B0 * v[0].y + C0;
    f32 oneOverTriArea = (1.0f/f32(triArea));

    // Use bounding box traversal strategy to determine which pixels to rasterize 
    s32 startX = max(min3(v[0].x, v[1].x, v[2].x), 0);
    s32 endX   = min(max3(v[0].x, v[1].x, v[2].x) + 1, width - 1);

    s32 startY = max(min3(v[0].y, v[1].y, v[2].y), 0);
    s32 endY   = min(max3(v[0].y, v[1].y, v[2].y) + 1, height - 1);

    // Extract this triangle's properties from the SIMD versions
    f32 zz[3];
    for (s32 vv = 0; vv < 3; vv++)
        zz[vv] = v[vv].z;

    zz[0] *= oneOverTriArea;
    zz[1] *= oneOverTriArea;
    zz[2] *= oneOverTriArea;

    s32 rowIdx = (startY * width + startX);
    s32 col = startX;
    s32 row = startY;

    s32 alpha0 = (A0 * col) + (B0 * row) + C0;
    s32 beta0 = (A1 * col) + (B1 * row) + C1;
    s32 gama0 = (A2 * col) + (B2 * row) + C2;

    const s32 fxptZero = 0;

    s32 numVisiblePixels = 0;

    // Incrementally compute Fab(x, y) for all the pixels inside the bounding box formed by (startX, endX) and (startY, endY)
    for (s32 r = startY; r < endY; r++,
        row++,
        rowIdx = rowIdx + width,
        alpha0 += B0,
        beta0 += B1,
        gama0 += B2)									 
    {
        // Compute barycentric coordinates 
        s32 idx = rowIdx;
        s32 alpha = alpha0;
        s32 beta = beta0;
        s32 gama = gama0;

        switch (rasterMode)
        {
            case RasterMode_ExactDepth:
            {
                for (s32 c = startX; c < endX; c++,
                    idx++,
                    alpha = alpha + A0,
                    beta  = beta  + A1,
                    gama  = gama  + A2)
                {
                    //Test Pixel inside triangle
                    const s32 mask = fxptZero < (alpha | beta | gama) ? 1 : 0;

                    // Early out if all of this quad's pixels are outside the triangle.
                    if ((mask & mask) == 0)
                    {
                        continue;
                    }

                    // Compute barycentric-interpolated depth
                    f32 depth = (alpha * zz[0]) + (beta * zz[1]) + (gama * zz[2]);
                    const f32 previousDepthValue = buffer[idx];

                    const s32 depthMask = (depth < previousDepthValue) ? 1 : 0;
                    const s32 finalMask = mask & depthMask;

                    depth = finalMask == 1 ? depth : previousDepthValue;
                    buffer[idx] = depth;

                    numVisiblePixels += finalMask;
                }
                break;
            }
            case RasterMode_MinDepth:
            case RasterMode_MaxDepth:
            {
                const f32 cmpDepth = rasterMode == RasterMode_MinDepth ? minZ : maxZ;

                for (s32 c = startX; c < endX; c++,
                    idx++,
                    alpha = alpha + A0,
                    beta  = beta  + A1,
                    gama  = gama  + A2)
                {
                    //Test Pixel inside triangle
                    const s32 mask = fxptZero < (alpha | beta | gama) ? 1 : 0;

                    // Early out if all of this quad's pixels are outside the triangle.
                    if((mask & mask) == 0)
                    {
                        continue;
                    }

                    // Compute barycentric-interpolated depth
                    const f32 previousDepthValue = buffer[idx];

                    const s32 depthMask = (cmpDepth < previousDepthValue) ? 1 : 0;
                    const s32 finalMask = mask & depthMask;

                    const f32 depth = finalMask == 1 ? cmpDepth : previousDepthValue;
                    buffer[idx] = depth;

                    numVisiblePixels += finalMask;
                }

                break;
            }
            UE_INVALID_CASE(rasterMode);
        }
    }

    return numVisiblePixels;
}

s32 rdSoftwareDepthRasterizer_DrawAABB(const f32* minPos, const f32* maxPos, RasterMode rasterMode)
{
    Pos v[2];
    rdSoftwareDepthRasterizer_ToIntCoords(minPos, v[0]);
    rdSoftwareDepthRasterizer_ToIntCoords(maxPos, v[1]);

    s32 minX = min(v[0].x, v[1].x);
    s32 maxX = max(v[0].x, v[1].x);
    s32 minY = min(v[0].y, v[1].y);
    s32 maxY = max(v[0].y, v[1].y);
    f32 minZ = min(v[0].z, v[1].z);
    f32 maxZ = max(v[0].z, v[1].z);

    if (minX >= width || maxX < 0 || minY >= height || maxY < 0 || minZ >= 1.0f || maxZ < 0.0f)
        return 0;

    minX = max(0, minX);
    maxX = min(width, maxX);
    minY = max(0, minY);
    maxY = min(height, maxY);
    minZ = max(0.0f, minZ);
    maxZ = min(1.0f, maxZ);

    const s32 widthPadding = width - (maxX - minX - 1);

    s32 numVisiblePixels = 0;

    switch (rasterMode)
    {
        case RasterMode_MinDepth:
        case RasterMode_MaxDepth:
        {
            const f32 cmpDepth = rasterMode == RasterMode_MinDepth ? minZ : maxZ;

            s32 depthIndex = width * minY + minX;
            for (s32 y = minY; y < maxY; y++, depthIndex += width)
            {
                for (s32 x = minX; x < maxX; x++, depthIndex++)
                    if (buffer[depthIndex] <= cmpDepth)
                    {
                        buffer[depthIndex] = cmpDepth;
                        numVisiblePixels++;
                    }
                depthIndex += widthPadding;
            }
            break;
        }

		UE_INVALID_CASE(rasterMode);
    }

    return numVisiblePixels;
}

bool rdSoftwareDepthRasterizer_CheckAABB(const f32* minPos, const f32* maxPos, RasterMode rasterMode)
{
    Pos v[2];
    rdSoftwareDepthRasterizer_ToIntCoords(minPos, v[0]);
    rdSoftwareDepthRasterizer_ToIntCoords(maxPos, v[1]);

    s32 minX = min(v[0].x, v[1].x);
    s32 maxX = max(v[0].x, v[1].x);
    s32 minY = min(v[0].y, v[1].y);
    s32 maxY = max(v[0].y, v[1].y);
    f32 minZ = min(v[0].z, v[1].z);
    f32 maxZ = max(v[0].z, v[1].z);

    if (minX >= width || maxX < 0 || minY >= height || maxY < 0 || minZ >= 1.0f || maxZ < 0.0f)
        return 0;

    minX = max(0, minX);
    maxX = min(width, maxX);
    minY = max(0, minY);
    maxY = min(height, maxY);
    minZ = max(0.0f, minZ);
    maxZ = min(1.0f, maxZ);

    const s32 widthPadding = width - (maxX - minX - 1);

    switch (rasterMode)
    {
        case RasterMode_MinDepth:
        case RasterMode_MaxDepth:
        {
            const f32 cmpDepth = rasterMode == RasterMode_MinDepth ? minZ : maxZ;

            s32 depthIndex = width * minY + minX;
            for (s32 y = minY; y < maxY; y++, depthIndex += width)
            {
                for (s32 x = minX; x < maxX; x++, depthIndex++)
                    if (buffer[depthIndex] <= cmpDepth)
                        return true;
                depthIndex += widthPadding;
            }
            break;
        }

        UE_INVALID_CASE(rasterMode);
    }

    return false;
}