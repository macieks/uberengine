#pragma once

/*
 * Simple "jump" helper (adapted from NVIDIA PhysX samples).
 *
 * a(t) = cte = g
 * v(t) = g*t + v0
 * y(t) = g*t^2 + v0*t + y0
 */
class phJump
{
public:
	UE_INLINE phJump();
	UE_INLINE void Update(f32 dt);
	UE_INLINE void StartJump(f32 v0);
	UE_INLINE void StopJump();
	UE_INLINE f32 GetHeight() const;

private:
	f32	m_jumpStrength;
	f32	m_jumpTime;
	ueBool m_isJumping;

	f32 m_gravity;
	f32 m_g;
};

UE_INLINE phJump::phJump() :
	m_isJumping(UE_FALSE),
	m_jumpStrength(0.0f),
	m_jumpTime(0.0f),
	m_gravity(-50.0f),
	m_g(-400.0f)
{}

UE_INLINE void phJump::Update(f32 dt)
{
	m_jumpTime += dt;
}

UE_INLINE void phJump::StartJump(f32 v0)
{
	if (m_isJumping) return;
	m_jumpTime = 0.0f;
	m_jumpStrength	= v0;
	m_isJumping = UE_TRUE;
}

UE_INLINE void phJump::StopJump()
{
	if (!m_isJumping) return;
	m_isJumping = UE_FALSE;
}

UE_INLINE f32 phJump::GetHeight() const
{
	if (!m_isJumping) return 0.0f;
	const f32 h = m_g * m_jumpTime * m_jumpTime + m_jumpStrength * m_jumpTime;
	return (h - m_gravity);
}