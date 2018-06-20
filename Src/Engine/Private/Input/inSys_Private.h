#ifndef IN_SYS_PRIVATE_H
#define IN_SYS_PRIVATE_H

enum inKeyStateFlags
{
	inKeyStateFlags_IsDown_User0	= UE_POW2(0),
	inKeyStateFlags_IsDown_User1	= UE_POW2(1),
	inKeyStateFlags_IsDown_User2	= UE_POW2(2),
	inKeyStateFlags_IsDown_User3	= UE_POW2(3),
	inKeyStateFlags_WasDown_User0	= UE_POW2(4),
	inKeyStateFlags_WasDown_User1	= UE_POW2(5),
	inKeyStateFlags_WasDown_User2	= UE_POW2(6),
	inKeyStateFlags_WasDown_User3	= UE_POW2(7),

	inKeyStateFlags_IsDown_AllUsers	= inKeyStateFlags_IsDown_User0 | inKeyStateFlags_IsDown_User1 | inKeyStateFlags_IsDown_User2 | inKeyStateFlags_IsDown_User3,
	inKeyStateFlags_WasDown_AllUsers= inKeyStateFlags_WasDown_User0 | inKeyStateFlags_WasDown_User1 | inKeyStateFlags_WasDown_User2 | inKeyStateFlags_WasDown_User3
};

#endif // IN_SYS_PRIVATE_H
