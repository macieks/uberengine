#include "Physics/phLib.h"
#include "GraphicsExt/gxShapeDraw.h"

class phDefaultDebugDraw : public phDebugDraw
{
public:
	void DrawLine(const ueVec3& from, const ueVec3& to, ueColor32 color)
	{
		gxShape_Line line;
		line.m_color = color;
		line.m_a = from;
		line.m_b = to;
		gxShapeDraw_DrawLine(line);
	}

	void DrawText(const ueVec3& location, const char* textString)
	{
		// TODO
	}
};

phDefaultDebugDraw s_defaultDebugDraw_Private;
phDebugDraw* s_defaultDebugDraw = &s_defaultDebugDraw_Private;