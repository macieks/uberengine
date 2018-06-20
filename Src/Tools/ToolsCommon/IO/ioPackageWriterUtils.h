#pragma once

#include "Math/ueShapes.h"

class ioSegmentWriter;
struct utValueInTime;

void WriteVec2(ioSegmentWriter* sw, const ueVec2& v);
void WriteVec3(ioSegmentWriter* sw, const ueVec3& v);
void WriteVec4(ioSegmentWriter* sw, const ueVec4& v);
void WriteQuat(ioSegmentWriter* sw, const ueQuat& v);
void WriteMat44(ioSegmentWriter* sw, const ueMat44& m);
void WriteBox(ioSegmentWriter* sw, const ueBox& box);
void WriteRectI(ioSegmentWriter* sw, const ueRectI& rect);
void WriteEmptyList(ioSegmentWriter* sw);
void WriteVec2Stride(ioSegmentWriter* sw);
void WriteVec3Stride(ioSegmentWriter* sw);
void WriteValueInTime(ioSegmentWriter* sw, utValueInTime* value);