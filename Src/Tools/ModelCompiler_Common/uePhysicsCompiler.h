#pragma once

class ioSegmentWriter;
struct ueToolCollisionGeometry;

//! Function implemented for a specific physics engine (e.g. PhysX, bullet)
typedef void (*ueWriteCollisionGeometryFunc)(ioSegmentWriter* sw, ioSegmentWriter* initSw, const ueToolCollisionGeometry* geom);

struct uePhysicsCompilerFuncs
{
	ueWriteCollisionGeometryFunc m_writeCollisionGeometryFunc;
};

uePhysicsCompilerFuncs* uePhysicsCompiler_Load(const char* name);
void uePhysicsCompiler_Unload();