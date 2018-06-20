#include "Base/ueSimdMath.h"

#ifdef UE_SIMD_MATH_ENABLED

smVec4 smVec4_Zero;
smVec4 smVec4_One;
smVec4 smVec4_UnitX;
smVec4 smVec4_UnitY;
smVec4 smVec4_UnitZ;

smVec4 smQuat_Identity;

smMat44 smMat44_Identity;
smMat44 smMat44_Zero;

class smMath_AutoInitializer
{
public:
	smMath_AutoInitializer()
	{
		smVec4_Zero = smVec4_Set(0.0f, 0.0f, 0.0f, 0.0f);
		smVec4_One = smVec4_Set(1.0f, 1.0f, 1.0f, 1.0f);
		smVec4_UnitX = smVec4_Set(1.0f, 0.0f, 0.0f, 0.0f);
		smVec4_UnitY = smVec4_Set(0.0f, 1.0f, 0.0f, 0.0f);
		smVec4_UnitZ = smVec4_Set(0.0f, 0.0f, 1.0f, 0.0f);
		smVec4_UnitW = smVec4_Set(0.0f, 0.0f, 0.0f, 1.0f);

		smQuat_Identity = smVec4_Set(0.0f, 0.0f, 0.0f, 1.0f);

		smMat44_Set44(smMat44_Identity, smVec4_UnitX, smVec4_UnitY, smVec4_UnitZ, smVec4_UnitW);
		smMat44_Set44(smMat44_Zero, smVec4_Zero, smVec4_Zero, smVec4_Zero, smVec4_Zero);
	}
};

smMath_AutoInitializer s_mathInitializer;

#endif // UE_MATH_HAS_SIMD