#include "Base/ueMath.h"

ueVec2 ueVec2::Zero(0, 0);
ueVec2 ueVec2::One(1, 1);
ueVec2 ueVec2::UnitX(1, 0);
ueVec2 ueVec2::UnitY(0, 1);

ueVec3 ueVec3::Zero(0, 0, 0);
ueVec3 ueVec3::One(1, 1, 1);
ueVec3 ueVec3::UnitX(1, 0, 0);
ueVec3 ueVec3::UnitY(0, 1, 0);
ueVec3 ueVec3::UnitZ(0, 0, 1);

ueVec4 ueVec4::Zero(0, 0, 0, 0);
ueVec4 ueVec4::One(1, 1, 1, 1);

ueQuat ueQuat::Identity(0, 0, 0, 1);

ueMat44 ueMat44::Identity(ueVec4(1, 0, 0, 0), ueVec4(0, 1, 0, 0), ueVec4(0, 0, 1, 0), ueVec4(0, 0, 0, 1));
ueMat44 ueMat44::Zero(ueVec4(0, 0, 0, 0), ueVec4(0, 0, 0, 0), ueVec4(0, 0, 0, 0), ueVec4(0, 0, 0, 0));

ueColor32 ueColor32::White(255, 255, 255, 255);
ueColor32 ueColor32::Black(0, 0, 0, 255);
ueColor32 ueColor32::Red(255, 0, 0, 255);
ueColor32 ueColor32::Green(0, 255, 0, 255);
ueColor32 ueColor32::Blue(0, 0, 255, 255);
ueColor32 ueColor32::Yellow(255, 255, 0, 255);
ueColor32 ueColor32::Violet(255, 0, 255, 255);

// Projection matrices implementation
// All functions account for differences between OpenGL and DirectX:
// (1) in OpenGL perspective space Z range is -1..1 while in DirectX it's 0..1
// (2) in OpenGL texture coordinates are inversed wrt. DirectX textures coordinate (top <-> bottom); this means
//     that when rendering to a texture we need to flip Y coords

void ueMat44::SetPerspectiveFov(f32 fov, f32 aspect, f32 nearZ, f32 farZ, ueBool isDrawToTexture)
{
	const f32 fovHalf = 0.5f * fov;
	const f32 sinFov = ueSin(fovHalf);
	const f32 cosFov = ueCos(fovHalf);

	const f32 height = cosFov / sinFov;
	const f32 width = height / aspect;

#if defined(GL_OPENGL)
	const f32 yMult = isDrawToTexture ? -1.0f : 1.0f;
	const f32 C = (farZ + nearZ) / (farZ - nearZ);
	const f32 D = -(2.0f * farZ * nearZ) / (farZ - nearZ);
#else
	const f32 yMult = 1.0f;
	const f32 C = farZ / (farZ - nearZ);
	const f32 D = - (nearZ * farZ) / (farZ - nearZ);
#endif

	SetColumn(0, width, 0.0f, 0.0f, 0.0f);
	SetColumn(1, 0.0f, yMult * height, 0.0f, 0.0f);
	SetColumn(2, 0.0f, 0.0f, C, 1.0);
	SetColumn(3, 0.0f, 0.0f, D, 0.0f);
}

void ueMat44::SetOrtho(f32 width, f32 height, f32 nearZ, f32 farZ, ueBool isDrawToTexture)
{
#if defined(GL_OPENGL)
	const f32 yMult = isDrawToTexture ? -1.0f : 1.0f;
	const f32 C = nearZ / (farZ - nearZ);
	const f32 D = 2.0f * nearZ / (nearZ - farZ);
#else
	const f32 yMult = 1.0f;
	const f32 C = 1.0f / (farZ - nearZ);
	const f32 D = nearZ / (nearZ - farZ);
#endif
	SetColumn(0, 2.0f / width, 0.0f, 0.0f, 0.0f);
	SetColumn(1, 0.0f, yMult * 2.0f / height, 0.0f, 0.0f);
	SetColumn(2, 0.0f, 0.0f, C, 0.0);
	SetColumn(3, 0.0f, 0.0f, D, 1.0f);
}

void ueMat44::SetOrthoOffCenter(f32 left, f32 right, f32 top, f32 bottom, f32 nearZ, f32 farZ, ueBool isDrawToTexture)
{
#if defined(GL_OPENGL)
	const f32 yMult = isDrawToTexture ? -1.0f : 1.0f;
	const f32 C = nearZ / (farZ - nearZ);
	const f32 D = 2.0f * nearZ / (nearZ - farZ);
#else
	const f32 yMult = 1.0f;
	const f32 C = 1.0f / (farZ - nearZ);
	const f32 D = nearZ / (nearZ - farZ);
#endif
	SetColumn(0, 2.0f / (right - left), 0.0f, 0.0f, 0.0f);
	SetColumn(1, 0.0f, yMult * 2.0f / (top - bottom), 0.0f, 0.0f);
	SetColumn(2, 0.0f, 0.0f, C, 0.0);
	SetColumn(3, (left + right) / (left - right), yMult * (top + bottom) / (bottom - top), D, 1.0f);
}

void ueMat44::SetLookAt(const ueVec3& eyePosition, const ueVec3& focusPosition, const ueVec3& upDirection)
{
	ueVec3 right, rightn, up, upn, vec, vec2;

    vec2 = focusPosition - eyePosition;
	vec = vec2;
    vec.Normalize();
	ueVec3::Cross(right, upDirection, vec);
    ueVec3::Cross(up, vec, right);
    rightn = right;
	rightn.Normalize();
    upn = up;
	upn.Normalize();
    
	Set(
		rightn[0], upn[0], vec[0], 0.0f,
		rightn[1], upn[1], vec[1], 0.0f,
		rightn[2], upn[2], vec[2], 0.0f,
		-ueVec3::Dot(rightn, eyePosition), -ueVec3::Dot(upn, eyePosition), -ueVec3::Dot(vec, eyePosition), 1.0f);
}

void ueMat44::SetAffineTransformation(f32 scale, const ueVec3& translation, const ueQuat& rotation, const ueVec3* rotationCenter)
{
    SetScale(scale, scale, scale);

	if (rotationCenter)
		Translate(-*rotationCenter);
	Rotate(rotation);
	if (rotationCenter)
		Translate(*rotationCenter);

	Translate(translation);
}

void ueMat44::SetRotation(const ueQuat& rot)
{
	Set(
		1.0f - 2.0f * (rot[1] * rot[1] + rot[2] * rot[2]),
		2.0f * (rot[0] * rot[1] + rot[2] * rot[3]),
		2.0f * (rot[0] * rot[2] - rot[1] * rot[3]),
		0,

		2.0f * (rot[0] * rot[1] - rot[2] * rot[3]),
		1.0f - 2.0f * (rot[0] * rot[0] + rot[2] * rot[2]),
		2.0f * (rot[1] * rot[2] + rot[0] * rot[3]),
		0,

		2.0f * (rot[0] * rot[2] + rot[1] * rot[3]),
		2.0f * (rot[1] * rot[2] - rot[0] * rot[3]),
		1.0f - 2.0f * (rot[0] * rot[0] + rot[1] * rot[1]),
		0,

		0,
		0,
		0,
		1);
}

void ueMat44::SetAxisRotation(const ueVec3& axis, f32 angle)
{
	ueVec3 v = axis;
	v.Normalize();

	const f32 c = ueCos(angle);
	const f32 s = ueSin(angle);

	Set(
		(1.0f - c) * v[0] * v[0] + c,
		(1.0f - c) * v[1] * v[0] + s * v[2],
		(1.0f - c) * v[2] * v[0] - s * v[1],
		0,

		(1.0f - c) * v[0] * v[1] - s * v[2],
		(1.0f - c) * v[1] * v[1] + c,
		(1.0f - c) * v[2] * v[1] + s * v[0],
		0,

		(1.0f - c) * v[0] * v[2] + s * v[1],
		(1.0f - c) * v[1] * v[2] - s * v[0],
		(1.0f - c) * v[2] * v[2] + c,
		0,
	
		0,
		0,
		0,
		1);
}

void ueMat44::GetRotation(ueQuat& out) const
{
	const f32 trace = m[0][0] + m[1][1] + m[2][2] + 1.0f;
	if (trace > 1.0f)
	{
		const f32 sqrtTrace = ueSqrt(trace);
		const f32 multFactor = 1.0f / (2.0f * sqrtTrace); 
		out.Set(
			(m[1][2] - m[2][1]) * multFactor,
			(m[2][0] - m[0][2]) * multFactor,
			(m[0][1] - m[1][0]) * multFactor,
			sqrtTrace * 0.5f);
		return;
	}

	u32 maxi = 0;
	f32 maxdiag = m[0][0];
	for (u32 i = 1; i < 3; i++)
		if (m[i][i] > maxdiag)
		{
			maxi = i;
			maxdiag = m[i][i];
		}

	f32 S, SInv;
	switch (maxi)
	{
	case 0:
		S = 2.0f * ueSqrt(1.0f + m[0][0] - m[1][1] - m[2][2]);
		SInv = 1.0f / S;
		out.Set(
			0.25f * S,
			(m[0][1] + m[1][0]) * SInv,
			(m[0][2] + m[2][0]) * SInv,
			(m[1][2] - m[2][1]) * SInv);
		break;
	case 1:
		S = 2.0f * ueSqrt(1.0f + m[1][1] - m[0][0] - m[2][2]);
		SInv = 1.0f / S;
		out.Set(
			(m[0][1] + m[1][0]) * SInv,
			0.25f * S,
			(m[1][2] + m[2][1]) * SInv,
			(m[2][0] - m[0][2]) * SInv);
		break;
	case 2:
		S = 2.0f * ueSqrt(1.0f + m[2][2] - m[0][0] - m[1][1]);
		SInv = 1.0f / S;
		out.Set(
			(m[0][2] + m[2][0]) * SInv,
			(m[1][2] + m[2][1]) * SInv,
			0.25f * S,
			(m[0][1] - m[1][0]) * SInv);
		break;
	}
}

void ueMat44::Mul(ueMat44& out, const ueMat44& a, const ueMat44& b)
{
	ueMat44 temp;
	for (u32 i = 0; i < 4; i++)
        for (u32 j = 0; j < 4; j++)
            temp[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];
	out = temp;
}

void ueMat44::Transpose()
{
	const ueMat44 temp(*this);
    for (u32 i = 0; i < 4; i++)
        for (u32 j = 0; j < 4; j++)
			m[i][j] = temp[j][i];
}

f32 ueMat44::Determinant() const
{
	ueVec4 minor, v1, v2, v3;
    f32 det;

    v1[0] = m[0][0]; v1[1] = m[1][0]; v1[2] = m[2][0]; v1[3] = m[3][0];
    v2[0] = m[0][1]; v2[1] = m[1][1]; v2[2] = m[2][1]; v2[3] = m[3][1];
    v3[0] = m[0][2]; v3[1] = m[1][2]; v3[2] = m[2][2]; v3[3] = m[3][2];
	ueVec4::Cross(minor, v1, v2, v3);
    det = -(m[0][3] * minor[0] + m[1][3] * minor[1] + m[2][3] * minor[2] + m[3][3] * minor[3]);
    return det;
}

ueBool ueMat44::Invert(ueMat44& out, const ueMat44& in)
{
	out = in;
	return out.Invert();
}

ueBool ueMat44::Invert()
{
	const f32 det = Determinant();
	if (det == 0.0f)
		return UE_FALSE;
	
    ueMat44 out;
    ueVec4 vv, vec[3];

	for (u32 i = 0; i < 4; i++)
	{
		for (u32 j = 0; j < 4; j++)
			if (j != i)
			{
				u32 a = j;
				if (j > i)
					a = a - 1;
				vec[a] = m[j];
			}
		ueVec4::Cross(vv, vec[0], vec[1], vec[2]);
		out.SetRow(i, vv * (uePow(-1.0f, (f32) i) / det));
	}

	*this = out;
	return UE_TRUE;
}

void ueMat44::Lerp(ueMat44& out, const ueMat44& a, const ueMat44& b, f32 scale)
{
	UE_ASSERT_MSG(&out != &a && &out != &b, "Output matrix is the same object as one of the input matrices");
	out = b;
	out -= a;
	out *= scale;
	out += a;
}

void ueMat44::Transform(ueVec4& out) const
{
	out.Set(
		m[0][0] * out[0] + m[1][0] * out[1] + m[2][0] * out[2] + m[3][0] * out[3],
		m[0][1] * out[0] + m[1][1] * out[1] + m[2][1] * out[2] + m[3][1] * out[3],
		m[0][2] * out[0] + m[1][2] * out[1] + m[2][2] * out[2] + m[3][2] * out[3],
		m[0][3] * out[0] + m[1][3] * out[1] + m[2][3] * out[2] + m[3][3] * out[3]);
}

void ueMat44::TransformNormal(ueVec3& out) const
{
	out.Set(
		m[0][0] * out[0] + m[1][0] * out[1] + m[2][0] * out[2],
		m[0][1] * out[0] + m[1][1] * out[1] + m[2][1] * out[2],
		m[0][2] * out[0] + m[1][2] * out[1] + m[2][2] * out[2]);
}

void ueMat44::TransformCoord(ueVec3& out) const
{
	const f32 norm = m[0][3] * out[0] + m[1][3] * out[1] + m[2][3] * out[2] + m[3][3];
    out.Set(
		m[0][0] * out[0] + m[1][0] * out[1] + m[2][0] * out[2] + m[3][0],
		m[0][1] * out[0] + m[1][1] * out[1] + m[2][1] * out[2] + m[3][1],
		m[0][2] * out[0] + m[1][2] * out[1] + m[2][2] * out[2] + m[3][2]);
	out /= norm;
}

void ueMat44::TransformCoord(ueVec3& outPoint, const ueVec3& inPoint) const
{
	outPoint = inPoint;
	TransformCoord(outPoint);
}

void ueMat44::TransformPlane(uePlane& out) const
{
	out.Set(
		m[0][0] * out[0] + m[1][0] * out[1] + m[2][0] * out[2] + m[3][0] * out[3],
		m[0][1] * out[0] + m[1][1] * out[1] + m[2][1] * out[2] + m[3][1] * out[3],
		m[0][2] * out[0] + m[1][2] * out[1] + m[2][2] * out[2] + m[3][2] * out[3],
		m[0][3] * out[0] + m[1][3] * out[1] + m[2][3] * out[2] + m[3][3] * out[3]);
}

void ueMat44::Decompose(ueVec3& scale, ueQuat& rotation, ueVec3& translation) const
{
	scale.Set(GetColumn3(0).Len(), GetColumn3(1).Len(), GetColumn3(2).Len());
	translation = GetTranslation();

	ueMat44 normalized(*this);
	for (u32 i = 0; i < 3; i++)
		normalized.GetColumn3(i) /= scale[i];
	normalized.GetRotation(rotation);
}

void ueVec2::Rotate(f32 angle)
{
	const f32 sin = ueSin(angle);
	const f32 cos = ueCos(angle);
	Set(v[0] * cos - v[1] * sin, v[0] * sin + v[1] * cos);
}

void ueVec3::ClampLen(f32 maxLength)
{
	const f32 lenSq = LenSq();
	if (lenSq <= ueSqr(maxLength))
		return;
	*this *= maxLength / ueSqrt(lenSq);
}

void ueVec2::CatmullRomInterpolate(ueVec2& out, const ueVec2& P0, const ueVec2& P1, const ueVec2& P2, const ueVec2& P3, f32 u)
{
	const f32 u2 = u * u;
	const f32 u3 = u2 * u;

	const f32 f1 = -0.5f * u3 + u2 - 0.5f * u;
	const f32 f2 =  1.5f * u3 - 2.5f * u2 + 1.0f;
	const f32 f3 = -1.5f * u3 + 2.0f * u2 + 0.5f * u;
	const f32 f4 =  0.5f * u3 - 0.5f * u2;

	const f32 x = P0[0] * f1 + P1[0] * f2 + P2[0] * f3 + P3[0] * f4;
	const f32 y = P0[1] * f1 + P1[1] * f2 + P2[1] * f3 + P3[1] * f4;

	out.Set(x, y);
}

void ueVec4::Cross(ueVec4& out, const ueVec4& pv1, const ueVec4& pv2, const ueVec4& pv3)
{
    out.Set(
		pv1[1] * (pv2[2] * pv3[3] - pv3[2] * pv2[3]) - pv1[2] * (pv2[1] * pv3[3] - pv3[1] * pv2[3]) + pv1[3] * (pv2[1] * pv3[2] - pv2[2] *pv3[1]),
		-(pv1[0] * (pv2[2] * pv3[3] - pv3[2] * pv2[3]) - pv1[2] * (pv2[0] * pv3[3] - pv3[0] * pv2[3]) + pv1[3] * (pv2[0] * pv3[2] - pv3[0] * pv2[2])),
		pv1[0] * (pv2[1] * pv3[3] - pv3[1] * pv2[3]) - pv1[1] * (pv2[0] *pv3[3] - pv3[0] * pv2[3]) + pv1[3] * (pv2[0] * pv3[1] - pv3[0] * pv2[1]),
		-(pv1[0] * (pv2[1] * pv3[2] - pv3[1] * pv2[2]) - pv1[1] * (pv2[0] * pv3[2] - pv3[0] *pv2[2]) + pv1[2] * (pv2[0] * pv3[1] - pv3[0] * pv2[1])));
}


void uePlane::Normalize()
{
	f32 lengthInv = ueSqrt(ueSqr(v[0]) + ueSqr(v[1]) + ueSqr(v[2]));
	if (lengthInv != 0)
		lengthInv = 1.0f / lengthInv;
	v[0] *= lengthInv;
	v[1] *= lengthInv;
	v[2] *= lengthInv;
	v[3] *= lengthInv;
}

void ueQuat::Mul(ueQuat& out, const ueQuat& a, const ueQuat& b)
{
    out.Set(
		b[3] * a[0] + b[0] * a[3] + b[1] * a[2] - b[2] * a[1],
		b[3] * a[1] - b[0] * a[2] + b[1] * a[3] + b[2] * a[0],
		b[3] * a[2] + b[0] * a[1] - b[1] * a[0] + b[2] * a[3],
		b[3] * a[3] - b[0] * a[0] - b[1] * a[1] - b[2] * a[2]);
}

void ueQuat::SetAxisRotation(const ueVec3& axis, f32 angle)
{
	ueVec3 temp = axis;
	temp.Normalize();
	const f32 halfAngle = angle * 0.5f;
	const f32 sinHalfAngle = ueSin(halfAngle);
	Set(sinHalfAngle * temp[0],
		sinHalfAngle * temp[1],
		sinHalfAngle * temp[2],
		ueCos(halfAngle));
}

void ueQuat::Invert()
{
	f32 normInv = Dot(*this, *this);
	if (normInv != 0)
		normInv = 1.0f / normInv;
	Set(
		-v[0] * normInv,
		-v[1] * normInv,
		-v[2] * normInv,
		v[3] * normInv);
}

void ueQuat::Slerp(ueQuat& out, const ueQuat& a, const ueQuat& b, f32 scale)
{
	f32 dot, epsilon, temp, theta, u;

	epsilon = 1.0f;
	temp = 1.0f - scale;
	u = scale;
	dot = Dot(a, b);
	if (dot < 0.0f)
	{
		epsilon = -1.0f;
		dot = -dot;
	}
	if (1.0f - dot > 0.001f)
	{
		theta = ueACos(dot);
		temp  = ueSin(theta * temp) / ueSin(theta);
		u = ueSin(theta * u) / ueSin(theta);
	}

	u *= epsilon;

	out.Set(
		temp * a[0] + u * b[0],
		temp * a[1] + u * b[1],
		temp * a[2] + u * b[2],
		temp * a[3] + u * b[3]);
}

// Misc

f32 ueCalcFOVForAspectRatio(const f32 aspectRatio, const f32 refFov, const f32 refAspectRatio)
{
	const f32 aspectRatioChange = aspectRatio / refAspectRatio;
	const f32 fovAdjust = 1.0f / ueSqrt(aspectRatioChange);
	return ueATan(ueTan(refFov * 0.5f) * fovAdjust) * 2.0f;
}

f32 ueBounce(const f32 from, const f32 to, const f32 numBounces, const f32 progress)
{
	if (progress <= 0.0f) return from;
	if (progress >= 1.0f) return to;

	const f32 dist = to - from;
	const f32 numSteps = numBounces - 0.5f;
	const f32 stepIndex = progress * numSteps;
	const f32 stepProgress = ueMod(stepIndex + 1.0f, 2.0f) - 1.0f;

	const f32 stepDone = ueSqr(stepProgress) * dist;
	const f32 pos = from + stepDone;

	return ueLerp(pos, to, uePow(progress, 8.5f));
}

f32 ueTriangle_CalcArea(const ueVec3& v0, const ueVec3& v1, const ueVec3& v2)
{
	const f32 a = ueVec3::Dist(v0, v1);
	const f32 b = ueVec3::Dist(v0, v2);
	const f32 c = ueVec3::Dist(v1, v2);
	if (a == 0 || b == 0 || c == 0) return 0.0f;

	const f32 s = (a + b + c) * 0.5f;
	return ueSqr(s * (s - a) * (s - b) * (s - c));
}