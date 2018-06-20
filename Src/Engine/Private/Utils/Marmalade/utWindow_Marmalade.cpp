#include "Utils/utWindow.h"

utWindowMsgBoxResult utWindowUtils_ShowMessageBox(utWindow* window, const char* message, const char* title, utWindowMsgBoxType type)
{
	// FIXME: For now I'm just using it for debugging

	for (u32 i = 0; i < 400; i++)
	{
		IwGxClear();
		IwGxPrintString(100, 100, message);
		IwGxFlush();
		IwGxSwapBuffers();
	}

	return utWindowMsgBoxResult_Ok;
}
