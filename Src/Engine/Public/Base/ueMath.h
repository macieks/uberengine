#ifndef UE_MATH_H
#define UE_MATH_H

#include "Base/ueSimdMath.h"

class ueVec2
{
private:
	f32 v[2];

public:
	static ueVec2 Zero;
	static ueVec2 One;
	static ueVec2 UnitX;
	static ueVec2 UnitY;

	UE_INLINE ueVec2() {}
	UE_INLINE ueVec2(f32 x, f32 y) { Set(x, y); }
	UE_INLINE ueVec2(const f32* values) { Set(values); }
	UE_INLINE ueVec2(const ueVec2& other) { *this = other; }

	UE_INLINE void Set(f32 x, f32 y) { v[0] = x; v[1] = y; }
	UE_INLINE void Set(const f32* values) { v[0] = values[0]; v[1] = values[1]; }
	UE_INLINE void Zeroe() { v[0] = v[1] = 0; }
	UE_INLINE void operator = (const ueVec2& other) { v[0] = other.v[0]; v[1] = other.v[1]; }

	UE_INLINE void Store2(f32* out) const { out[0] = v[0]; out[1] = v[1]; }

	UE_INLINE void operator += (const ueVec2& other) { v[0] += other.v[0]; v[1] += other.v[1]; }
	UE_INLINE ueVec2 operator + (const ueVec2& other) const { return ueVec2(v[0] + other.v[0], v[1] + other.v[1]); }
	UE_INLINE ueVec2 operator + (f32 value) const { return ueVec2(v[0] + value, v[1] + value); }
	UE_INLINE void operator -= (const ueVec2& other) { v[0] -= other.v[0]; v[1] -= other.v[1]; }
	UE_INLINE ueVec2 operator - (const ueVec2& other) const { return ueVec2(v[0] - other.v[0], v[1] - other.v[1]); }
	UE_INLINE ueVec2 operator - (f32 value) const { return ueVec2(v[0] - value, v[1] - value); }
	UE_INLINE void operator *= (const ueVec2& other) { v[0] *= other.v[0]; v[1] *= other.v[1]; }
	UE_INLINE ueVec2 operator * (const ueVec2& other) const { return ueVec2(v[0] * other.v[0], v[1] * other.v[1]); }
	UE_INLINE void operator *= (f32 value) { v[0] *= value; v[1] *= value; }
	UE_INLINE ueVec2 operator * (f32 value) const { return ueVec2(v[0] * value, v[1] * value); }
	UE_INLINE void operator /= (const ueVec2& other) { v[0] /= other.v[0]; v[1] /= other.v[1]; }
	UE_INLINE ueVec2 operator / (const ueVec2& other) const { return ueVec2(v[0] / other.v[0], v[1] / other.v[1]); }
	UE_INLINE void operator /= (f32 value) { v[0] /= value; v[1] /= value; }
	UE_INLINE ueVec2 operator / (f32 value) const { return ueVec2(v[0] / value, v[1] / value); }
	UE_INLINE ueVec2 operator - () const { return ueVec2(-v[0], -v[1]); }
	UE_INLINE void Negate() { Set(-v[0], -v[1]); }

	UE_INLINE f32 Len() const { return ueSqrt(ueSqr(v[0]) + ueSqr(v[1])); }
	UE_INLINE f32 LenSq() const { return ueSqr(v[0]) + ueSqr(v[1]); }

	UE_INLINE void Normalize() { *this *= 1.0f / Len(); }

	UE_INLINE static f32 Dot(const ueVec2& a, const ueVec2& b) { return a.v[0] * b.v[0] + a.v[1] * b.v[1]; }
	UE_INLINE static f32 Dist(const ueVec2& a, const ueVec2& b) { return ueSqrt(ueSqr(a.v[0] - b.v[0]) + ueSqr(a.v[1] - b.v[1])); }
	UE_INLINE static void Lerp(ueVec2& out, const ueVec2& a, const ueVec2& b, f32 scale) { out = a + (b - a) * scale; }
	static void CatmullRomInterpolate(ueVec2& out, const ueVec2& P0, const ueVec2& P1, const ueVec2& P2, const ueVec2& P3, f32 u);

	UE_INLINE ueBool operator == (const ueVec2& other) const { return v[0] == other.v[0] && v[1] == other.v[1]; }
	UE_INLINE ueBool operator != (const ueVec2& other) const { return v[0] != other.v[0] || v[1] != other.v[1]; }
	UE_INLINE ueBool operator < (const ueVec2& other) const { return v[0] < other.v[0] && v[1] < other.v[1]; }
	UE_INLINE ueBool operator <= (const ueVec2& other) const { return v[0] <= other.v[0] && v[1] <= other.v[1]; }

	UE_INLINE const f32 operator [] (s32 index) const { UE_ASSERT(0 <= index && index < 2); return v[index]; }
	UE_INLINE f32& operator [] (s32 index) { UE_ASSERT(0 <= index && index < 2); return v[index]; }

	UE_INLINE operator const f32* () const { return v; }

	void Rotate(f32 angle);

#ifdef UE_SIMD_MATH_ENABLED
	UE_INLINE ueVec2(smVec4 vec) { smVec4_Store2((f32*) v, vec); }
	UE_INLINE smVec4 ToSmVec4(f32 z = 0.0f, f32 w = 1.0f) const { return smVec4_Set(v[0], v[1], z, w); }
#endif
};

class ueVec3
{
private:
	f32 v[3];

public:
	static ueVec3 Zero;
	static ueVec3 One;
	static ueVec3 UnitX;
	static ueVec3 UnitY;
	static ueVec3 UnitZ;

	UE_INLINE ueVec3() {}
	UE_INLINE ueVec3(f32 x, f32 y, f32 z) { Set(x, y, z); }
	UE_INLINE ueVec3(const ueVec2& xy, f32 z) { Set(xy, z); }
	UE_INLINE ueVec3(const f32* values) { Set(values); }
	UE_INLINE ueVec3(const ueVec3& other) { *this = other; }

	UE_INLINE void Set(f32 x, f32 y, f32 z) { v[0] = x; v[1] = y; v[2] = z; }
	UE_INLINE void Set(const ueVec2& xy, f32 z) { v[0] = xy[0]; v[1] = xy[1]; v[2] = z; }
	UE_INLINE void Set(const f32* values) { v[0] = values[0]; v[1] = values[1]; v[2] = values[2]; }
	UE_INLINE void Zeroe() { v[0] = v[1] = v[2] = 0; }
	UE_INLINE void operator = (const ueVec3& other) { v[0] = other.v[0]; v[1] = other.v[1]; v[2] = other.v[2]; }

	UE_INLINE void Store2(f32* out) const { out[0] = v[0]; out[1] = v[1]; }
	UE_INLINE void Store3(f32* out) const { out[0] = v[0]; out[1] = v[1]; out[2] = v[2]; }

	UE_INLINE static void Add(ueVec3& out, const ueVec3& a, const ueVec3& b) { out.Set(a[0] + b[0], a[1] + b[1], a[2] + b[2]); }
	UE_INLINE void operator += (const ueVec3& other) { v[0] += other.v[0]; v[1] += other.v[1]; v[2] += other.v[2]; }
	UE_INLINE void operator += (f32 value) { v[0] += value; v[1] += value; v[2] += value; }
	UE_INLINE ueVec3 operator + (const ueVec3& other) const { return ueVec3(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2]); }
	UE_INLINE ueVec3 operator + (f32 value) const { return ueVec3(v[0] + value, v[1] + value, v[2] + value); }
	UE_INLINE static void Sub(ueVec3& out, const ueVec3& a, const ueVec3& b) { out.Set(a[0] - b[0], a[1] - b[1], a[2] - b[2]); }
	UE_INLINE void operator -= (const ueVec3& other) { v[0] -= other.v[0]; v[1] -= other.v[1]; v[2] -= other.v[2]; }
	UE_INLINE ueVec3 operator - (const ueVec3& other) const { return ueVec3(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2]); }
	UE_INLINE void operator -= (f32 value) { v[0] -= value; v[1] -= value; v[2] -= value; }
	UE_INLINE static void Mul(ueVec3& out, const ueVec3& a, const ueVec3& b) { out.Set(a[0] * b[0], a[1] * b[1], a[2] * b[2]); }
	UE_INLINE void operator *= (const ueVec3& other) { v[0] *= other.v[0]; v[1] *= other.v[1]; v[2] *= other.v[2]; }
	UE_INLINE ueVec3 operator * (const ueVec3& other) const { return ueVec3(v[0] * other.v[0], v[1] * other.v[1], v[2] * other.v[2]); }
	UE_INLINE void operator *= (f32 value) { v[0] *= value; v[1] *= value; v[2] *= value; }
	UE_INLINE ueVec3 operator * (f32 value) const { return ueVec3(v[0] * value, v[1] * value, v[2] * value); }
	UE_INLINE static void Div(ueVec3& out, const ueVec3& a, const ueVec3& b) { out.Set(a[0] / b[0], a[1] / b[1], a[2] / b[2]); }
	UE_INLINE void operator /= (const ueVec3& other) { v[0] /= other.v[0]; v[1] /= other.v[1]; v[2] /= other.v[2]; }
	UE_INLINE ueVec3 operator / (const ueVec3& other) const { return ueVec3(v[0] / other.v[0], v[1] / other.v[1], v[2] / other.v[2]); }
	UE_INLINE void operator /= (f32 value) { v[0] /= value; v[1] /= value; v[2] /= value; }
	UE_INLINE ueVec3 operator / (f32 value) const { return ueVec3(v[0] / value, v[1] / value, v[2] / value); }
	UE_INLINE void Negate() { Set(-v[0], -v[1], -v[2]); }
	UE_INLINE ueVec3 operator - () const { return ueVec3(-v[0], -v[1], -v[2]); }

	UE_INLINE f32 Len() const { return ueSqrt(ueSqr(v[0]) + ueSqr(v[1]) + ueSqr(v[2])); }
	UE_INLINE f32 LenSq() const { return ueSqr(v[0]) + ueSqr(v[1]) + ueSqr(v[2]); }

	UE_INLINE void Normalize() { *this *= 1.0f / Len(); }
			  void ClampLen(f32 maxLength);
	UE_INLINE void MakeNormal(const ueVec3& start, const ueVec3& end) { *this = end; *this -= start; Normalize(); }

	UE_INLINE static f32 Dot(const ueVec3& a, const ueVec3& b) { return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2]; }
	UE_INLINE static void Cross(ueVec3& out, const ueVec3& a, const ueVec3& b) { out.Set(a.v[1] * b.v[2] - a.v[2] * b.v[1], a.v[2] * b.v[0] - a.v[0] * b.v[2], a.v[0] * b.v[1] - a.v[1] * b.v[0]); }
	UE_INLINE static f32 Dist(const ueVec3& a, const ueVec3& b) { return ueSqrt(ueSqr(a.v[0] - b.v[0]) + ueSqr(a.v[1] - b.v[1]) + ueSqr(a.v[2] - b.v[2])); }
	UE_INLINE static void Lerp(ueVec3& out, const ueVec3& a, const ueVec3& b, f32 scale) { out = a + (b - a) * scale; }

	UE_INLINE static void Min(ueVec3& out, const ueVec3& a, const ueVec3& b) { out.Set(ueMin(a.v[0], b.v[0]), ueMin(a.v[1], b.v[1]), ueMin(a.v[2], b.v[2])); }
	UE_INLINE static void Max(ueVec3& out, const ueVec3& a, const ueVec3& b) { out.Set(ueMax(a.v[0], b.v[0]), ueMax(a.v[1], b.v[1]), ueMax(a.v[2], b.v[2])); }

	UE_INLINE ueBool operator == (const ueVec3& other) const { return v[0] == other.v[0] && v[1] == other.v[1] && v[2] == other.v[2]; }
	UE_INLINE ueBool operator != (const ueVec3& other) const { return v[0] != other.v[0] || v[1] != other.v[1] || v[2] != other.v[2]; }
	UE_INLINE ueBool operator < (const ueVec3& other) const { return v[0] < other.v[0] && v[1] < other.v[1] && v[2] < other.v[2]; }
	UE_INLINE ueBool operator <= (const ueVec3& other) const { return v[0] <= other.v[0] && v[1] <= other.v[1] && v[2] <= other.v[2]; }

	UE_INLINE const f32 operator [] (s32 index) const { UE_ASSERT(0 <= index && index < 3); return v[index]; }
	UE_INLINE f32& operator [] (s32 index) { UE_ASSERT(0 <= index && index < 3); return v[index]; }

	UE_INLINE operator const f32* () const { return v; }

#ifdef UE_SIMD_MATH_ENABLED
	UE_INLINE ueVec3(smVec4 vec) { smVec4_Store3((f32*) v, vec); }
	UE_INLINE smVec4 ToSmVec4(f32 w = 1.0f) const { return smVec4_Set(v[0], v[1], v[2], w); }
#endif
};

class ueVec4
{
private:
	f32 v[4];

public:
	static ueVec4 Zero;
	static ueVec4 One;

	UE_INLINE ueVec4() {}
	UE_INLINE ueVec4(f32 x, f32 y, f32 z, f32 w) { Set(x, y, z, w); }
	UE_INLINE ueVec4(const ueVec4& other) { *this = other; }
	UE_INLINE ueVec4(const f32* values) { Set(values); }
	UE_INLINE ueVec4(const ueVec3& xyz, f32 w) { Set(xyz, w); }

	UE_INLINE void Set(f32 x, f32 y, f32 z, f32 w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
	UE_INLINE void Set(const f32* values) { v[0] = values[0]; v[1] = values[1]; v[2] = values[2]; v[3] = values[3]; }
	UE_INLINE void Set(const ueVec3& xyz, f32 w) { Set(xyz[0], xyz[1], xyz[2], w); }
	UE_INLINE void Set(const ueVec3& xyz) { v[0] = xyz[0]; v[1] = xyz[1]; v[2] = xyz[2]; }
	UE_INLINE void Zeroe() { v[0] = v[1] = v[2] = v[3] = 0; }
	UE_INLINE void operator = (const ueVec4& other) { v[0] = other.v[0]; v[1] = other.v[1]; v[2] = other.v[2]; v[3] = other.v[3]; }

	UE_INLINE const f32 operator [] (s32 index) const { UE_ASSERT(0 <= index && index < 4); return v[index]; }
	UE_INLINE f32& operator [] (s32 index) { UE_ASSERT(0 <= index && index < 4); return v[index]; }

	UE_INLINE operator const f32* () const { return v; }

	UE_INLINE void Store2(f32* out) const { out[0] = v[0]; out[1] = v[1]; }
	UE_INLINE void Store3(f32* out) const { out[0] = v[0]; out[1] = v[1]; out[2] = v[2]; }
	UE_INLINE void Store4(f32* out) const { out[0] = v[0]; out[1] = v[1]; out[2] = v[2]; out[3] = v[3]; }

	UE_INLINE static void Add(ueVec4& out, const ueVec4& a, const ueVec4& b) { out.Set(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]); }
	UE_INLINE void operator += (const ueVec4& other) { v[0] += other.v[0]; v[1] += other.v[1]; v[2] += other.v[2]; v[3] += other.v[3]; }
	UE_INLINE ueVec4 operator + (const ueVec4& other) const { return ueVec4(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2], v[3] + other.v[3]); }
	UE_INLINE static void Sub(ueVec4& out, const ueVec4& a, const ueVec4& b) { out.Set(a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]); }
	UE_INLINE void operator -= (const ueVec4& other) { v[0] -= other.v[0]; v[1] -= other.v[1]; v[2] -= other.v[2]; v[3] -= other.v[3]; }
	UE_INLINE ueVec4 operator - (const ueVec4& other) const { return ueVec4(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2], v[3] - other.v[3]); }
	UE_INLINE static void Mul(ueVec4& out, const ueVec4& a, const ueVec4& b) { out.Set(a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]); }
	UE_INLINE void operator *= (const ueVec4& other) { v[0] *= other.v[0]; v[1] *= other.v[1]; v[2] *= other.v[2]; v[3] *= other.v[3]; }
	UE_INLINE ueVec4 operator * (const ueVec4& other) const { return ueVec4(v[0] * other.v[0], v[1] * other.v[1], v[2] * other.v[2], v[3] * other.v[3]); }
	UE_INLINE void operator *= (f32 value) { v[0] *= value; v[1] *= value; v[2] *= value; v[3] *= value; }
	UE_INLINE ueVec4 operator * (f32 value) const { return ueVec4(v[0] * value, v[1] * value, v[2] * value, v[3] * value); }
	UE_INLINE static void Div(ueVec4& out, const ueVec4& a, const ueVec4& b) { out.Set(a[0] / b[0], a[1] / b[1], a[2] / b[2], a[3] / b[3]); }
	UE_INLINE void operator /= (const ueVec4& other) { v[0] /= other.v[0]; v[1] /= other.v[1]; v[2] /= other.v[2]; v[3] /= other.v[3]; }
	UE_INLINE ueVec4 operator / (const ueVec4& other) const { return ueVec4(v[0] / other.v[0], v[1] / other.v[1], v[2] / other.v[2], v[3] / other.v[3]); }
	UE_INLINE void operator /= (f32 value) { v[0] /= value; v[1] /= value; v[2] /= value; v[3] /= value; }
	UE_INLINE ueVec4 operator / (f32 value) const { return ueVec4(v[0] / value, v[1] / value, v[2] / value, v[3] / value); }
	UE_INLINE ueVec4 operator - () const { return ueVec4(-v[0], -v[1], -v[2], -v[3]); }
	UE_INLINE void Negate() { Set(-v[0], -v[1], -v[2], -v[3]); }

	UE_INLINE static void Lerp(ueVec4& out, const ueVec4& a, const ueVec4& b, f32 scale) { out = a + (b - a) * scale; }
	static void Cross(ueVec4& out, const ueVec4& pv1, const ueVec4& pv2, const ueVec4& pv3);

	UE_INLINE ueBool operator == (const ueVec4& other) const { return v[0] == other.v[0] && v[1] == other.v[1] && v[2] == other.v[2] && v[3] == other.v[3]; }
	UE_INLINE ueBool operator != (const ueVec4& other) const { return v[0] != other.v[0] || v[1] != other.v[1] || v[2] != other.v[2] || v[3] != other.v[3]; }
	UE_INLINE ueBool operator < (const ueVec4& other) const { return v[0] < other.v[0] && v[1] < other.v[1] && v[2] < other.v[2] && v[3] < other.v[3]; }
	UE_INLINE ueBool operator <= (const ueVec4& other) const { return v[0] <= other.v[0] && v[1] <= other.v[1] && v[2] <= other.v[2] && v[3] <= other.v[3]; }

#ifdef UE_SIMD_MATH_ENABLED
	UE_INLINE ueVec4(smVec4 vec) { smVec4_Store4((f32*) v, vec); }
	UE_INLINE smVec4 ToSmVec4() const { return smVec4_Load4((const f32*) v); }
#endif
};

class ueQuat
{
private:
	f32 v[4];

public:
	static ueQuat Identity;

	UE_INLINE ueQuat() {}
	UE_INLINE ueQuat(const ueQuat& other) { *this = other; }
	UE_INLINE ueQuat(f32 x, f32 y, f32 z, f32 w) { Set(x, y, z, w); }
	UE_INLINE void SetIdentity() { v[0] = v[1] = v[2] = 0; v[3] = 1; }
	UE_INLINE void Set(f32 x, f32 y, f32 z, f32 w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
	UE_INLINE void Set(const f32* values) { v[0] = values[0]; v[1] = values[1]; v[2] = values[2]; v[3] = values[3]; }
	UE_INLINE void operator = (const ueQuat& other) { v[0] = other.v[0]; v[1] = other.v[1]; v[2] = other.v[2]; v[3] = other.v[3]; }

	UE_INLINE ueBool operator == (const ueQuat& other) const { return v[0] == other.v[0] && v[1] == other.v[1] && v[2] == other.v[2] && v[3] == other.v[3]; }

	UE_INLINE const f32 operator [] (s32 index) const { UE_ASSERT(0 <= index && index < 4); return v[index]; }
	UE_INLINE f32& operator [] (s32 index) { UE_ASSERT(0 <= index && index < 4); return v[index]; }

	UE_INLINE operator const f32* () const { return (const f32*) v; }

	UE_INLINE void Store4(f32* out) const { out[0] = v[0]; out[1] = v[1]; out[2] = v[2]; out[3] = v[3]; }

	static void Mul(ueQuat& out, const ueQuat& a, const ueQuat& b);
	UE_INLINE void operator *= (const ueQuat& other) { Mul(*this, *this, other); }
	UE_INLINE ueQuat operator * (const ueQuat& other) const { ueQuat ret = *this; ret *= other; return ret; }

	UE_INLINE static f32 Dot(const ueQuat& a, const ueQuat& b) { return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2] + a.v[3] * b.v[3]; }
	void SetAxisRotation(const ueVec3& axis, f32 angle);
	void Invert();

	static void Slerp(ueQuat& out, const ueQuat& a, const ueQuat& b, f32 scale);
};

class uePlane
{
private:
	f32 v[4];

public:
	UE_INLINE uePlane() {}
	UE_INLINE uePlane(f32 x, f32 y, f32 z, f32 w) { Set(x, y, z, w); }
	UE_INLINE void Set(f32 x, f32 y, f32 z, f32 w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
	UE_INLINE void Set(const f32* values) { v[0] = values[0]; v[1] = values[1]; v[2] = values[2]; v[3] = values[3]; }

	UE_INLINE const f32 operator [] (s32 index) const { UE_ASSERT(0 <= index && index < 4); return v[index]; }
	UE_INLINE f32& operator [] (s32 index) { UE_ASSERT(0 <= index && index < 4); return v[index]; }

	UE_INLINE operator const f32* () const { return v; }

	UE_INLINE void Store4(f32* out) const { out[0] = v[0]; out[1] = v[1]; out[2] = v[2]; out[3] = v[3]; }

	void Normalize();
	UE_INLINE f32 DotCoord(const ueVec3& point) const { return v[0] * point[0] + v[1] * point[1] + v[2] * point[2] + v[3]; }
	UE_INLINE const ueVec3& GetDirection() const { return *(const ueVec3*) (const f32*) v; }
};

class ueMat44
{
private:
	ueVec4 m[4];

public:
	static ueMat44 Identity;
	static ueMat44 Zero;

	UE_INLINE ueMat44() {}
	UE_INLINE ueMat44(const ueMat44& other) { *this = other; }
	UE_INLINE ueMat44(const ueVec4& column0, const ueVec4& column1, const ueVec4& column2, const ueVec4& column3) { Set(column0, column1, column2, column3); }
	UE_INLINE ueMat44(const f32* values) { Set(values); }
	UE_INLINE void operator = (const ueMat44& other) { for (u32 i = 0; i < 4; i++) m[i] = other.m[i]; }

	UE_INLINE void Set33(const ueVec3& column0, const ueVec3& column1, const ueVec3& column2)
	{
		m[0].Set(column0, 0);
		m[1].Set(column1, 0);
		m[2].Set(column2, 0);
		m[3].Set(0, 0, 0, 1);
	}
	UE_INLINE void Set43(const ueVec4& column0, const ueVec4& column1, const ueVec4& column2)
	{
		m[0] = column0;
		m[1] = column1;
		m[2] = column2;
		m[3].Set(0, 0, 0, 1);
	}
	UE_INLINE void Set(const ueVec4& column0, const ueVec4& column1, const ueVec4& column2, const ueVec4& column3)
	{
		m[0] = column0;
		m[1] = column1;
		m[2] = column2;
		m[3] = column3;
	}
	UE_INLINE void Set(
		f32 column0X, f32 column0Y, f32 column0Z, f32 column0W,
		f32 column1X, f32 column1Y, f32 column1Z, f32 column1W,
		f32 column2X, f32 column2Y, f32 column2Z, f32 column2W,
		f32 column3X, f32 column3Y, f32 column3Z, f32 column3W)
	{
		m[0].Set(column0X, column0Y, column0Z, column0W);
		m[1].Set(column1X, column1Y, column1Z, column1W);
		m[2].Set(column2X, column2Y, column2Z, column2W);
		m[3].Set(column3X, column3Y, column3Z, column3W);
	}
	UE_INLINE void Set(const f32* values) { ueMemCpy(this, values, sizeof(f32) * 16); }
	UE_INLINE void SetColumn(u32 index, const ueVec4& vec) { m[index] = vec; }
	UE_INLINE void SetColumn(u32 index, const ueVec3& xyz, f32 w) { m[index].Set(xyz, w); }
	UE_INLINE void SetColumn(u32 index, f32 x, f32 y, f32 z, f32 w) { m[index].Set(x, y, z, w); }
	UE_INLINE void SetColumn(u32 index, const ueVec3& xyz) { m[index].Set(xyz); }
	UE_INLINE void SetRow(u32 index, const ueVec4& vec) { for (u32 i = 0; i < 4; i++) m[i][index] = vec[i]; }

	UE_INLINE const ueVec4& GetColumn(u32 index) const { return m[index]; }
	UE_INLINE ueVec4& GetColumn(u32 index) { return m[index]; }
	UE_INLINE const ueVec3& GetColumn3(u32 index) const { return *(const ueVec3*)(const void*) &m[index]; }
	UE_INLINE ueVec3& GetColumn3(u32 index) { return *(ueVec3*)(void*) &m[index]; }
	UE_INLINE ueVec4 GetRow(u32 index) const { ueVec4 ret; GetRow(ret, index); return ret; }
	UE_INLINE void GetRow(ueVec4& row, u32 index) const { row.Set(m[0][index], m[1][index], m[2][index], m[3][index]); }
	UE_INLINE ueVec3 GetRow3(u32 index) const { ueVec3 ret; GetRow3(ret, index); return ret; }
	UE_INLINE void GetRow3(ueVec3& row, u32 index) const { row.Set(m[0][index], m[1][index], m[2][index]); }

	UE_INLINE const f32 operator () (u32 column, u32 row) const { return m[column][row]; }
	UE_INLINE f32& operator () (u32 column, u32 row) { return m[column][row]; }

	UE_INLINE operator const f32* () const { return (const f32*) m[0]; }

	UE_INLINE const ueVec4& operator [] (s32 index) const { UE_ASSERT(0 <= index && index < 4); return m[index]; }
	UE_INLINE ueVec4& operator [] (s32 index) { UE_ASSERT(0 <= index && index < 4); return m[index]; }

	UE_INLINE void SetIdentity() { m[0].Set(1, 0, 0, 0); m[1].Set(0, 1, 0, 0); m[2].Set(0, 0, 1, 0); m[3].Set(0, 0, 0, 1); }
	UE_INLINE void Zeroe() { m[0].Set(0, 0, 0, 0); m[1].Set(0, 0, 0, 0); m[2].Set(0, 0, 0, 0); m[3].Set(0, 0, 0, 0); }

	UE_INLINE ueVec3 GetRightVec() const { return GetRow3(0); }
	UE_INLINE ueVec3 GetUpVec() const { return GetRow3(1); }
	UE_INLINE ueVec3 GetFrontVec() const { return GetRow3(2); }

	static void Mul(ueMat44& out, const ueMat44& a, const ueMat44& b);
	UE_INLINE void operator *= (const ueMat44& other) { Mul(*this, *this, other); }
	UE_INLINE void operator *= (f32 value) { for (u32 i = 0; i < 4; i++) m[i] *= value; }
	UE_INLINE void operator += (const ueMat44& other) { for (u32 i = 0; i < 4; i++) m[i] += other.m[i]; }
	UE_INLINE void operator -= (const ueMat44& other) { for (u32 i = 0; i < 4; i++) m[i] -= other.m[i]; }
	UE_INLINE void operator /= (f32 value)  { for (u32 i = 0; i < 4; i++) m[i] /= value; }

	UE_INLINE void SetTranslation(f32 x, f32 y, f32 z) { SetTranslation(ueVec3(x, y, z)); }
	UE_INLINE void SetTranslation(const ueVec3& translation) { m[0].Set(1, 0, 0, 0); m[1].Set(0, 1, 0, 0); m[2].Set(0, 0, 1, 0); m[3].Set(translation, 1); }

			  void SetRotation(const ueQuat& rotation);
			  void SetAxisRotation(const ueVec3& axis, f32 angle);

	UE_INLINE void SetScale(const ueVec3& scale) { m[0].Set(scale[0], 0, 0, 0); m[1].Set(0, scale[1], 0, 0); m[2].Set(0, 0, scale[2], 0); m[3].Set(0, 0, 0, 1); }
	UE_INLINE void SetScale(f32 scaleX, f32 scaleY, f32 scaleZ) { SetScale(ueVec3(scaleX, scaleY, scaleZ)); }
	UE_INLINE void SetScale(f32 scale) { SetScale(ueVec3(scale, scale, scale)); }

	UE_INLINE const ueVec3& GetTranslation() const { return *(const ueVec3*) (const void*) m[3]; }
			  void GetRotation(ueQuat& out) const;

  	UE_INLINE void Translate(const ueVec3& translation) { ueMat44 temp; temp.SetTranslation(translation); *this *= temp; }
	UE_INLINE void Translate(f32 translationX, f32 translationY, f32 translationZ) { Translate(ueVec3(translationX, translationY, translationZ)); }
	UE_INLINE void Rotate(const ueQuat& rotation) { ueMat44 temp; temp.SetRotation(rotation); *this *= temp; }
	UE_INLINE void Rotate(const ueVec3& axis, f32 angle) { ueMat44 temp; temp.SetAxisRotation(axis, angle); *this *= temp; }
	UE_INLINE void Scale(const ueVec3& scale) { ueMat44 temp; temp.SetScale(scale); *this *= temp; }
	UE_INLINE void Scale(f32 scale) { Scale(ueVec3(scale, scale, scale)); }

	void Transpose();
	f32 Determinant() const;
	ueBool Invert();
	static ueBool Invert(ueMat44& out, const ueMat44& in);

	static void Lerp(ueMat44& out, const ueMat44& a, const ueMat44& b, f32 scale);

	void Transform(ueVec4& vec) const;
	void TransformNormal(ueVec3& normal) const;
	void TransformCoord(ueVec3& point) const;
	void TransformCoord(ueVec3& outPoint, const ueVec3& inPoint) const;
	void TransformPlane(uePlane& plane) const;

	void SetPerspectiveFov(f32 fov, f32 aspect, f32 nearZ, f32 farZ, ueBool isDrawToTexture);
	void SetOrtho(f32 width, f32 height, f32 nearZ, f32 farZ, ueBool isDrawToTexture);
	void SetOrthoOffCenter(f32 left, f32 right, f32 top, f32 bottom, f32 nearZ, f32 farZ, ueBool isDrawToTexture);
	void SetLookAt(const ueVec3& eyePosition, const ueVec3& focusPosition, const ueVec3& upDirection);
	void SetAffineTransformation(f32 scale, const ueVec3& translation, const ueQuat& rotation, const ueVec3* rotationCenter = NULL);

	void Decompose(ueVec3& scale, ueQuat& rotation, ueVec3& translation) const;

	UE_INLINE ueBool operator == (const ueMat44& other) const { return m[0] == other.m[0] && m[1] == other.m[1] && m[2] == other.m[2] && m[3] == other.m[3]; }
};

//!	32 bit color (ARGB, each component 8 bit) representation.
class ueColor32
{
public:
	u8 r, g, b, a;

	static ueColor32 White;
	static ueColor32 Black;
	static ueColor32 Red;
	static ueColor32 Green;
	static ueColor32 Blue;
	static ueColor32 Yellow;
	static ueColor32 Violet;

	UE_INLINE ueColor32() : r(255), g(255), b(255), a(255) {}
	UE_INLINE ueColor32(s32 r, s32 g, s32 b, s32 a) : r(S32ToU8(r)), g(S32ToU8(g)), b(S32ToU8(b)), a(S32ToU8(a)) {}
	UE_INLINE ueColor32(const ueVec4& rgba) : r(F32ToU8(rgba[0])), g(F32ToU8(rgba[1])), b(F32ToU8(rgba[2])), a(F32ToU8(rgba[3])) {}
	UE_INLINE ueColor32(const ueVec3& rgb, f32 a) : r(F32ToU8(rgb[0])), g(F32ToU8(rgb[1])), b(F32ToU8(rgb[2])), a(F32ToU8(a)) {}
	UE_INLINE ueColor32(const f32* rgba) : r(F32ToU8(rgba[0])), g(F32ToU8(rgba[1])), b(F32ToU8(rgba[2])), a(F32ToU8(rgba[3])) {}

	UE_INLINE static ueColor32 F(f32 r, f32 g, f32 b, f32 a) { return ueColor32(F32ToU8(r), F32ToU8(g), F32ToU8(b), F32ToU8(a)); }
	UE_INLINE static ueColor32 FromRGBA(u32 v) { return ueColor32((v >> 24) & 255, (v >> 16) & 255, (v >> 8) & 255, v & 255); }
	UE_INLINE static ueColor32 FromARGB(u32 v) { return ueColor32((v >> 16) & 255, (v >> 8) & 255, v & 255, (v >> 24) & 255); }

	UE_INLINE u32 AsARGB() const { return UE_LE_4BYTE_SYMBOL(a, r, g, b); }
	UE_INLINE u32 AsRGBA() const { return UE_LE_4BYTE_SYMBOL(r, g, b, a); }
	UE_INLINE u32 AsARGB_LE() const { return UE_LE_4BYTE_SYMBOL(a, r, g, b); }
	UE_INLINE u32 AsRGBA_LE() const { return UE_LE_4BYTE_SYMBOL(r, g, b, a); }
	UE_INLINE u32 AsARGB_BE() const { return UE_BE_4BYTE_SYMBOL(a, r, g, b); }
	UE_INLINE u32 AsRGBA_BE() const { return UE_BE_4BYTE_SYMBOL(r, g, b, a); }

	UE_INLINE f32 RedF() const { return U8ToF32(r); }
	UE_INLINE f32 GreenF() const { return U8ToF32(g); }
	UE_INLINE f32 BlueF() const { return U8ToF32(b); }
	UE_INLINE f32 AlphaF() const { return U8ToF32(a); }

	UE_INLINE void SetRedF(f32 v) { r = F32ToU8(v); }
	UE_INLINE void SetGreenF(f32 v) { g = F32ToU8(v); }
	UE_INLINE void SetBlueF(f32 v) { b = F32ToU8(v); }
	UE_INLINE void SetAlphaF(f32 v) { a = F32ToU8(v); }

#if defined(GL_D3D9) || defined(GL_D3D11) || defined(GL_OPENGL) || defined(GL_MAC) || defined(GL_MARMALADE)
	UE_INLINE u32 AsShaderAttr() const { return AsRGBA_BE(); }
#elif defined(GL_X360) || defined(GL_PS3) || defined(GL_WII)
    UE_INLINE u32 AsShaderAttr() const { return AsRGBA_LE(); }
#endif

	UE_INLINE void AsRGBAVec4(ueVec4& out) const
	{
		const f32 s = 1.0f / 255.0f;
		out.Set(r * s, g * s, b * s, a * s);
	}

	UE_INLINE ueColor32 operator * (ueColor32 other) const
	{
		return ueColor32(
			(s32) r * other.r >> 8,
			(s32) g * other.g >> 8,
			(s32) b * other.b >> 8,
			(s32) a * other.a >> 8);
	}

	UE_INLINE ueColor32 operator * (f32 scale) const
	{
		const s32 scaleInt32 = (s32) ueFloor(scale * 255.0f);
		return ueColor32(
			(s32) r * scaleInt32 >> 8,
			(s32) g * scaleInt32 >> 8,
			(s32) b * scaleInt32 >> 8,
			(s32) a * scaleInt32 >> 8);
	}

	UE_INLINE void SetRGB(u8 _r, u8 _g, u8 _b)
	{
		r = _r;
		g = _g;
		b = _b;
	}

	UE_INLINE void IncRGB(u8 value)
	{
		r = (u8) ueMin((u32) 255, (u32) r + (u32) value);
		g = (u8) ueMin((u32) 255, (u32) g + (u32) value);
		b = (u8) ueMin((u32) 255, (u32) b + (u32) value);
	}

	UE_INLINE void DecRGB(u8 value)
	{
		r = r <= value ? 0 : (r - value);
		g = g <= value ? 0 : (g - value);
		b = b <= value ? 0 : (b - value);
	}

	UE_INLINE void IncAlpha(u8 value) { a = (u8) ueMin((u32) 255, (u32) a + (u32) value); }
	UE_INLINE void DecAlpha(u8 value) { a = a <= value ? 0 : (a - value); }

	UE_INLINE static ueColor32 Lerp(ueColor32 x, ueColor32 y, f32 scale)
	{
		UE_ASSERT(0 <= scale && scale <= 1);
		const s32 scaleTimes256 = (s32) (scale * 256.0f);
		return ueColor32(
			x.r + ((y.r - x.r) * scaleTimes256 >> 8),
			x.g + ((y.g - x.g) * scaleTimes256 >> 8),
			x.b + ((y.b - x.b) * scaleTimes256 >> 8),
			x.a + ((y.a - x.a) * scaleTimes256 >> 8));
	}

	UE_INLINE ueBool operator == (ueColor32 other) const
	{
		return *(u32*) this == *(u32*) &other;
	}

	static u8 F32ToU8(f32 value)
	{
		return (u8) ueClamp((s32) (value * 255.0f), 0, 255);
	}
	static u8 S32ToU8(s32 value)
	{
		return (u8) ueClamp(value, 0, 255);
	}
	static f32 U8ToF32(u8 value)
	{
		return (f32) value * (1.0f / 255.0f);
	}

#ifdef UE_SIMD_MATH_ENABLED
	UE_INLINE ueColor32(smVec4 rgba)
	{
		f32 values[4];
		smVec4_Store4(values, rgba);
		r = F32ToU8(values[0]);
		g = F32ToU8(values[1]);
		b = F32ToU8(values[2]);
		a = F32ToU8(values[3]);
	}
#endif
};

//! 2-component integer vector
class ueVec2I
{
public:
	s32 m_x;
	s32 m_y;

	UE_INLINE ueVec2I() {}
	UE_INLINE ueVec2I(s32 x, s32 y) : m_x(x), m_y(y) {}
	UE_INLINE explicit ueVec2I(const ueVec2& vec) : m_x((s32) vec[0]), m_y((s32) vec[1]) {}
	UE_INLINE ueVec2I operator + (const ueVec2I& vec) const { return ueVec2I(m_x + vec.m_x, m_y + vec.m_y); }
	UE_INLINE ueVec2I operator - (const ueVec2I& vec) const { return ueVec2I(m_x - vec.m_x, m_y - vec.m_y); }
	UE_INLINE ueVec2I& operator += (const ueVec2I& vec) { m_x += vec.m_x; m_y += vec.m_y; return *this; }
	UE_INLINE ueVec2I& operator -= (const ueVec2I& vec) { m_x -= vec.m_x; m_y -= vec.m_y; return *this; }
	UE_INLINE ueBool operator == (const ueVec2I& vec) const { return m_x == vec.m_x && m_y == vec.m_y; }
	UE_INLINE ueBool operator != (const ueVec2I& vec) const { return m_x != vec.m_x && m_y != vec.m_y; }
	UE_INLINE ueBool operator == (const ueVec2& vec) const { return ueVec2((f32) m_x, (f32) m_y) == vec; }
	UE_INLINE ueBool operator != (const ueVec2& vec) const { return ueVec2((f32) m_x, (f32) m_y) != vec; }
	UE_INLINE void AsVec2f(ueVec2& out) const { out.Set((f32) m_x, (f32) m_y); }
};

struct ueVec3Lerper
{
	UE_INLINE static void Lerp(ueVec3& out, const ueVec3& a, const ueVec3& b, f32 scale)
	{
		ueVec3::Lerp(out, a, b, scale);
	}
};

struct ueQuatLerper
{
	UE_INLINE static void Lerp(ueQuat& out, const ueQuat& a, const ueQuat& b, f32 scale)
	{
		ueQuat::Slerp(out, a, b, scale);
	}
};

#endif // UE_MATH_H
