#include <Xnamath.h>

#define UE_SIMD_MATH_ENABLED
#define UE_SIMD_MATH_ALIGNMENT 16

#define smVec4 XMVECTOR
#define smVec4Arg const XMVECTOR&
#define smMat44 XMMATRIX

extern smVec4 smVec4_Zero;
extern smVec4 smVec4_One;
extern smVec4 smVec4_UnitX;
extern smVec4 smVec4_UnitY;
extern smVec4 smVec4_UnitZ;
extern smVec4 smVec4_UnitW;

extern smVec4 smQuat_Identity;

extern smMat44 smMat44_Identity;
extern smMat44 smMat44_Zero;

/*-------------------- smVec4 --------------------------*/

UE_INLINE smVec4 smVec4_Set(f32 x, f32 y, f32 z, f32 w) { return XMVectorSet(x, y, z, w); }
UE_INLINE smVec4 smVec4_Load2(const f32* ptr) { return XMLoadFloat2((const XMFLOAT2*) ptr); }
UE_INLINE smVec4 smVec4_Load3(const f32* ptr) { return XMLoadFloat3((const XMFLOAT3*) ptr); }
UE_INLINE smVec4 smVec4_Load4(const f32* ptr) { return XMLoadFloat4((const XMFLOAT4*) ptr); }
UE_INLINE smVec4 smVec4_Replicate(f32 value) { return XMVectorReplicate(value); }
UE_INLINE smVec4 smVec4_SetX(smVec4 in, f32 x) { return XMVectorSetX(in, x); }
UE_INLINE smVec4 smVec4_SetY(smVec4 in, f32 y) { return XMVectorSetY(in, y); }
UE_INLINE smVec4 smVec4_SetZ(smVec4 in, f32 z) { return XMVectorSetZ(in, z); }
UE_INLINE smVec4 smVec4_SetW(smVec4 in, f32 w) { return XMVectorSetW(in, w); }
UE_INLINE smVec4 smVec4_SetByIndex(smVec4 in, u32 index, f32 value) { return XMVectorSetByIndex(in, value, index); }

UE_INLINE void smVec4_Store2(f32* out, smVec4 in) { XMStoreFloat2((XMFLOAT2*) out, in); }
UE_INLINE void smVec4_Store3(f32* out, smVec4 in) { XMStoreFloat3((XMFLOAT3*) out, in); }
UE_INLINE void smVec4_Store4(f32* out, smVec4 in) { XMStoreFloat4((XMFLOAT4*) out, in); }
UE_INLINE f32 smVec4_GetX(smVec4 in) { return XMVectorGetX(in); }
UE_INLINE f32 smVec4_GetY(smVec4 in) { return XMVectorGetY(in); }
UE_INLINE f32 smVec4_GetZ(smVec4 in) { return XMVectorGetZ(in); }
UE_INLINE f32 smVec4_GetW(smVec4 in) { return XMVectorGetW(in); }
UE_INLINE f32 smVec4_GetByIndex(smVec4 in, u32 index) { return XMVectorGetByIndex(in, index); }

UE_INLINE smVec4 smVec4_SplatX(smVec4 v) { return XMVectorSplatX(v); }
UE_INLINE smVec4 smVec4_SplatY(smVec4 v) { return XMVectorSplatY(v); }
UE_INLINE smVec4 smVec4_SplatZ(smVec4 v) { return XMVectorSplatZ(v); }
UE_INLINE smVec4 smVec4_SplatW(smVec4 v) { return XMVectorSplatW(v); }

UE_INLINE smVec4 smVec4_Add(smVec4 a, smVec4 b) { return XMVectorAdd(a, b); }
UE_INLINE smVec4 smVec4_Sub(smVec4 a, smVec4 b) { return XMVectorSubtract(a, b); }
UE_INLINE smVec4 smVec4_Div(smVec4 a, smVec4 b) { return XMVectorDivide(a, b); }
UE_INLINE smVec4 smVec4_Mul(smVec4 a, smVec4 b) { return XMVectorMultiply(a, b); }
UE_INLINE smVec4 smVec4_Neg(smVec4 in) { return XMVectorNegate(in); }
UE_INLINE smVec4 smVec4_MulAdd(smVec4 a, smVec4 b, smVec4 c) { return XMVectorMultiplyAdd(a, b, c); }

UE_INLINE smVec4 smVec4_Lerp(smVec4 a, smVec4 b, f32 scale) { return XMVectorLerp(a, b, scale); }
UE_INLINE smVec4 smVec4_Min(smVec4 a, smVec4 b) { return XMVectorMin(a, b); }
UE_INLINE smVec4 smVec4_Max(smVec4 a, smVec4 b) { return XMVectorMax(a, b); }

UE_INLINE smVec4 smVec4_Equal(smVec4 a, smVec4 b) { return XMVectorEqual(a, b); }
UE_INLINE smVec4 smVec4_NotEqual(smVec4 a, smVec4 b) { return XMVectorNotEqual(a, b); }
UE_INLINE smVec4 smVec4_Less(smVec4 a, smVec4 b) { return XMVectorLess(a, b); }

/*-------------------- smVec2 --------------------------*/

UE_INLINE smVec4 smVec2_Len(smVec4 in) { return XMVector2Length(in); }
UE_INLINE smVec4 smVec2_LenSq(smVec4 in) { return XMVector2LengthSq(in); }
UE_INLINE smVec4 smVec2_Normalize(smVec4 in) { return XMVector2Normalize(in); }
UE_INLINE smVec4 smVec2_Dot(smVec4 a, smVec4 b) { return XMVector2Dot(a, b); }
UE_INLINE smVec4 smVec2_Lerp(smVec4 a, smVec4 b, f32 scale) { return XMVectorLerp(a, b, scale); }
UE_INLINE BOOL smVec2_Equal(smVec4 a, smVec4 b) { return XMVector2Equal(a, b); }
UE_INLINE BOOL smVec2_NotEqual(smVec4 a, smVec4 b) { return XMVector2NotEqual(a, b); }
UE_INLINE BOOL smVec2_Less(smVec4 a, smVec4 b) { return XMVector2Less(a, b); }
UE_INLINE BOOL smVec2_LEqual(smVec4 a, smVec4 b) { return XMVector2LessOrEqual(a, b); }

/*-------------------- smVec3 --------------------------*/

UE_INLINE smVec4 smVec3_Len(smVec4 in) { return XMVector3Length(in); }
UE_INLINE smVec4 smVec3_LenSq(smVec4 in) { return XMVector3LengthSq(in); }
UE_INLINE smVec4 smVec3_Normalize(smVec4 in) { return XMVector3Normalize(in); }
UE_INLINE smVec4 smVec3_Dot(smVec4 a, smVec4 b) { return XMVector3Dot(a, b); }
UE_INLINE smVec4 smVec3_Cross(smVec4 a, smVec4 b) { return XMVector3Cross(a, b); }
UE_INLINE BOOL smVec3_Equal(smVec4 a, smVec4 b) { return XMVector3Equal(a, b); }
UE_INLINE BOOL smVec3_NotEqual(smVec4 a, smVec4 b) { return XMVector3NotEqual(a, b); }
UE_INLINE BOOL smVec3_Less(smVec4 a, smVec4 b) { return XMVector3Less(a, b); }
UE_INLINE BOOL smVec3_LEqual(smVec4 a, smVec4 b) { return XMVector3LessOrEqual(a, b); }

/*-------------------- smQuat --------------------------*/

UE_INLINE smVec4 smQuat_SetAxisRotation(smVec4 axis, f32 angle) { return XMQuaternionRotationAxis(axis, angle); }
UE_INLINE smVec4 smQuat_Mul(smVec4 a, smVec4 b) { return XMQuaternionMultiply(a, b); }
UE_INLINE smVec4 smQuat_Invert(smVec4 in) { return XMQuaternionInverse(in); }
UE_INLINE smVec4 smQuat_Slerp(smVec4 a, smVec4 b, f32 scale) { return XMQuaternionSlerp(a, b, scale); }

/*-------------------- smPlane --------------------------*/

UE_INLINE smVec4 smPlane_Normalize(smVec4 in) { return XMPlaneNormalize(in); }
UE_INLINE smVec4 smPlane_DotCoord(smVec4 plane, smVec4 point) { return XMPlaneDotCoord(plane, point); }

/*-------------------- smMat44 --------------------------*/

UE_INLINE void smMat44_SetIdentity(smMat44& out) { out = XMMatrixIdentity(); }
UE_INLINE void smMat44_Zeroe(smMat44& out) { ueMemSet(&out, 0, sizeof(smMat44)); }
UE_INLINE void smMat44_Copy(smMat44& out, const smMat44& in) { out = in; }

UE_INLINE void smMat44_Set33(smMat44& out, smVec4 column0, smVec4 column1, smVec4 column2)
{
	column0 = XMVectorSetW(column0, 0);
	column1 = XMVectorSetW(column1, 0);
	column2 = XMVectorSetW(column2, 0);
	new(&out) XMMATRIX(column0, column1, column2, XMVectorSet(0, 0, 0, 1));
}
UE_INLINE void smMat44_Set43(smMat44& out, smVec4 column0, smVec4 column1, smVec4 column2)
{
	new(&out) XMMATRIX(column0, column1, column2, XMVectorSet(0, 0, 0, 1));
}
UE_INLINE void smMat44_Set44(smMat44& out, smVec4 column0, smVec4 column1, smVec4 column2, smVec4Arg column3)
{
	new(&out) XMMATRIX(column0, column1, column2, column3);
}
UE_INLINE void smMat44_Set44(smMat44& out,
							 f32 column0X, f32 column0Y, f32 column0Z, f32 column0W,
							 f32 column1X, f32 column1Y, f32 column1Z, f32 column1W,
							 f32 column2X, f32 column2Y, f32 column2Z, f32 column2W,
							 f32 column3X, f32 column3Y, f32 column3Z, f32 column3W)
{
	new(&out) XMMATRIX(
		column0X, column0Y, column0Z, column0W,
		column1X, column1Y, column1Z, column1W,
		column2X, column2Y, column2Z, column2W,
		column3X, column3Y, column3Z, column3W);
}
UE_INLINE void smMat44_Set44(smMat44& out, const f32* values) { new(&out) XMMATRIX(values); }
UE_INLINE void smMat44_SetElem(smMat44& in, u32 row, u32 column, f32 value) { in(row, column) = value; }
UE_INLINE void smMat44_SetColumnV(smMat44& inout, u32 index, smVec4 vec) { inout.r[index] = vec; }
//UE_INLINE void smMat44_SetColumnV3(smMat44& inout, u32 index, smVec4 vec) { smVec4_SetReplicateVec3XYZ(inout.r[index], vec); }
UE_INLINE void smMat44_SetColumn(smMat44& inout, u32 index, f32 x, f32 y, f32 z, f32 w) { inout.r[index] = XMVectorSet(x, y, z, w); }
UE_INLINE void smMat44_SetTranslation(smMat44& out, f32 x, f32 y, f32 z) { out = XMMatrixTranslation(x, y, z); }
UE_INLINE void smMat44_SetTranslationV(smMat44& out, smVec4 translation) { out = XMMatrixTranslationFromVector(translation); }
UE_INLINE void smMat44_SetRotation(smMat44& out, smVec4 rotationQuaternion) { out = XMMatrixRotationQuaternion(rotationQuaternion); }
UE_INLINE void smMat44_SetAxisRotationV(smMat44& out, smVec4 axis, f32 angle) { out = XMMatrixRotationAxis(axis, angle); }
UE_INLINE void smMat44_SetAxisRotation(smMat44& out, f32 axisX, f32 axisY, f32 axisZ, f32 angle) { out = XMMatrixRotationAxis(XMVectorSet(axisX, axisY, axisZ, 0), angle); }
UE_INLINE void smMat44_SetScaleV(smMat44& out, smVec4 scale) { out = XMMatrixScalingFromVector(scale); }
UE_INLINE void smMat44_SetScale(smMat44& out, f32 scaleX, f32 scaleY, f32 scaleZ) { out = XMMatrixScaling(scaleX, scaleY, scaleZ); }

UE_INLINE f32 smMat44_GetElem(const smMat44& in, u32 row, u32 column) { return in(row, column); }
#define smMat44_GetColumn(/*smVec4*/ out, /*const smMat44&*/ in, /*u32*/ index) out = (in).r[index]
#define smMat44_GetTranslation(/*smVec3*/ out, /*const smMat44&*/ in) { out = (in).r[3]; }
#define smMat44_GetRotation(/*smQuat*/ out, /*const smMat44&*/ in) out = XMQuaternionRotationMatrix(in)
#define smMat44_GetRightVec(/*smVec3*/ out, /*const smMat44&*/ in) out = XMVectorSetW((in).r[0], 0)
#define smMat44_GetUpVec(/*smVec3*/ out, /*const smMat44&*/ in) out = XMVectorSetW((in).r[1], 0)
#define smMat44_GetFrontVec(/*smVec3*/ out, /*const smMat44&*/ in) out = XMVectorSetW((in).r[2], 0)

UE_INLINE void smMat44_Mul(smMat44& out, const smMat44& a, const smMat44& b) { out = XMMatrixMultiply(a, b); }
UE_INLINE void smMat44_Mul(smMat44& inout, const smMat44& a) { inout *= a; }
UE_INLINE void smMat44_Mul(smMat44& inout, f32 value) { smVec4 valueVec = smVec4_Replicate(value); for (u32 i = 0; i < 4; i++) inout.r[i] = smVec4_Mul(inout.r[i], valueVec); }
UE_INLINE void smMat44_Add(smMat44& inout, const smMat44& a) { for (u32 i = 0; i < 4; i++) inout.r[i] = smVec4_Add(inout.r[i], a.r[i]); }
UE_INLINE void smMat44_Sub(smMat44& inout, const smMat44& a)  { for (u32 i = 0; i < 4; i++) inout.r[i] = smVec4_Sub(inout.r[i], a.r[i]); }

UE_INLINE void smMat44_Translate(smMat44& inout, smVec4 translation) { inout *= XMMatrixTranslationFromVector(translation); }
UE_INLINE void smMat44_Translate(smMat44& inout, f32 translationX, f32 translationY, f32 translationZ) { inout *= XMMatrixTranslation(translationX, translationY, translationZ); }
UE_INLINE void smMat44_Rotate(smMat44& inout, smVec4 rotationQuaternion) { inout *= XMMatrixRotationQuaternion(rotationQuaternion); }
UE_INLINE void smMat44_Rotate(smMat44& inout, smVec4 axis, f32 angle) { inout *= XMMatrixRotationAxis(axis, angle); }
UE_INLINE void smMat44_Rotate(smMat44& inout, f32 axisX, f32 axisY, f32 axisZ, f32 angle) { inout *= XMMatrixRotationAxis(XMVectorSet(axisX, axisY, axisZ, 0), angle); }
UE_INLINE void smMat44_Scale(smMat44& inout, smVec4 scale) { inout *= XMMatrixScalingFromVector(scale); }
UE_INLINE void smMat44_Scale(smMat44& inout, f32 scale) { inout *= XMMatrixScaling(scale, scale, scale); }

UE_INLINE void smMat44_Transpose(smMat44& out, const smMat44& in) { out = XMMatrixTranspose(in); }
UE_INLINE void smMat44_Transpose(smMat44& inout) { inout = XMMatrixTranspose(inout); }
UE_INLINE void smMat44_Invert(smMat44& out, const smMat44& in) { XMVECTOR det = XMMatrixDeterminant(in); out = XMMatrixInverse(&det, in); }
UE_INLINE void smMat44_Invert(smMat44& inout) { XMVECTOR det = XMMatrixDeterminant(inout); inout = XMMatrixInverse(&det, inout); }

UE_INLINE void smMat44_Lerp(smMat44& out, const smMat44& a, const smMat44& b, f32 scale)
{
	out = b;
	smMat44_Sub(out, a);
	smMat44_Mul(out, scale);
	smMat44_Add(out, a);
}

#define smMat44_TransformV3(/*smVec4*/ out, /*const smMat44&*/ transform, /*smVec3*/ vec) out = XMVector3Transform(vec, transform)
#define smMat44_Transform(/*smVec4*/ out, /*const smMat44&*/ transform, /*smVec4*/ vec) out = XMVector4Transform(vec, transform)
#define smMat44_TransformNormal(/*smVec3*/ out, /*const smMat44&*/ transform, /*smVec3*/ normal) out = XMVector3TransformNormal(normal, transform)
#define smMat44_TransformNormalI(/*smVec3*/ inout, /*const smMat44&*/ transform) inout = XMVec3TransformNormal(inout, transform)
#define smMat44_TransformCoord(/*smVec3*/ out, /*const smMat44&*/ transform, /*smVec3*/ coord) out = XMVector3TransformCoord(coord, transform)
#define smMat44_TransformCoordI(/*smVec3*/ inout, /*const smMat44&*/ transform) inout = XMVector3TransformCoord(inout, transform)
#define smMat44_TransformCoordXYZ(/*smVec3*/ out, /*const smMat44&*/ transform, /*f32*/ x, /*f32*/ y, /*f32*/ z) out = XMVector3TransformCoord(XMVectorSet(x, y, z, 1), transform)
#define smMat44_TransformPlane(/*smPlane*/ out, /*const smMat44&*/ transform, /*smPlane*/ plane) out = XMPlaneTransform(plane, transform)
UE_INLINE f32 smMat44_CalcDeterminant(const smMat44& in) { return XMVectorGetX(XMMatrixDeterminant(in)); }

UE_INLINE void smMat44_LookAt(smMat44& out, smVec4 eyePosition, smVec4 focusPosition, smVec4Arg upDirection) { out = XMMatrixLookAtLH(eyePosition, focusPosition, upDirection); }
UE_INLINE void smMat44_AffineTransformation(smMat44& out, f32 scale, smVec4 translation, smVec4 rotationQuaternion, smVec4Arg rotationCenter) { out = XMMatrixAffineTransformation(XMVectorReplicate(scale), rotationCenter, rotationQuaternion, translation); }

UE_INLINE ueBool smMat44_Equal(const smMat44& a, const smMat44& b) { return XMVector4Equal(a.r[0], b.r[0]) && XMVector4Equal(a.r[1], b.r[1]) && XMVector4Equal(a.r[2], b.r[2]) && XMVector4Equal(a.r[3], b.r[3]); }