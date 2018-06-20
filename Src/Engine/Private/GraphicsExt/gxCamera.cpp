#include "GraphicsExt/gxCamera.h"
#include "Input/inSys.h"
#include "Graphics/glLib.h"

gxCamera::gxCamera() :
	m_movementSpeed(18.0f),
	m_rotationSpeed(0.5f),
	m_yaw(0),
	m_pitch(0),
	m_roll(0),
	m_nearZ(0.1f),
	m_farZ(100.0f),
	m_drawingToTexture(UE_FALSE)
{
	m_position.Zeroe();
	SetAspectRatio(glDevice_GetAspectRatio());
	UpdateFreeCamera(0);
}

void gxCamera::RotateX(f32 angle)
{
	m_pitch += angle;
	CalcMatrices();
}

void gxCamera::RotateY(f32 angle)
{
	m_yaw += angle;
	CalcMatrices();
}

void gxCamera::RotateZ(f32 angle)
{
	m_roll += angle;
	CalcMatrices();
}

void gxCamera::SetDrawingToTexture(ueBool enable)
{
	if (m_drawingToTexture == enable)
		return;

	m_drawingToTexture = enable;
#if defined(GL_OPENGL)
	CalcMatrices();
#endif
}

void gxCamera::SetAspectRatio(f32 aspectRatio, f32 refFov, f32 refAspectRatio)
{
	m_aspect = aspectRatio;
	m_FOV = ueCalcFOVForAspectRatio(m_aspect, refFov, refAspectRatio);
	CalcMatrices();
}

void gxCamera::SetLookAt(const ueVec3& eyePos, const ueVec3& atPos)
{
	// Set camera position

	m_position = eyePos;

	// Set viewing angles

	const ueVec3 toVec = atPos - eyePos;
	const f32 toVecXZLen = ueVec2(toVec[0], toVec[2]).Len();

	m_yaw = ueATan2(toVec[0], toVec[2]);
	m_pitch = -ueATan2(toVec[1], toVecXZLen);
	m_roll = 0.0f;

	// Update matrices

	CalcMatrices();
}

void gxCamera::SetNearZ(f32 nearZ)
{
	m_nearZ = nearZ;
	CalcMatrices();
}

void gxCamera::SetFarZ(f32 farZ)
{
	m_farZ = farZ;
	CalcMatrices();
}

void gxCamera::UpdateFreeCamera(f32 dt)
{
	// Update rotation (mouse and game pad)

	s32 mouseX = 0;
	s32 mouseY = 0;
	if (inMouse_IsValidAny())
		inMouse_GetVector(mouseX, mouseY);

	m_yaw += dt * (f32) mouseX * m_rotationSpeed;
	m_pitch += dt * (f32) mouseY * m_rotationSpeed;

	if (inGamepad_IsValidAny())
	{
		m_yaw += dt * inGamepad_GetAnalog(inGamepadAnalog_RightStick_X) * m_rotationSpeed * 10.0f;
		m_pitch -= dt * inGamepad_GetAnalog(inGamepadAnalog_RightStick_Y) * m_rotationSpeed * 10.0f;
	}

	// Handle movement (keyboard and gamepad)

	ueVec3 movementDir = ueVec3::Zero;

	if (inKeyboard_IsValidAny())
	{
		if (inKeyboard_IsDown(inKey_W)) movementDir[2] = 1;
		else if (inKeyboard_IsDown(inKey_S)) movementDir[2] = -1;
		if (inKeyboard_IsDown(inKey_D)) movementDir[0] = 1;
		else if (inKeyboard_IsDown(inKey_A)) movementDir[0] = -1;
		if (inKeyboard_IsDown(inKey_Q)) movementDir[1] = 1;
		else if (inKeyboard_IsDown(inKey_E)) movementDir[1] = -1;
	}

	f32 speed = 1.0f;
	if (inGamepad_IsValidAny() && movementDir == ueVec3::Zero)
	{
		ueVec2 padVec(inGamepad_GetAnalog(inGamepadAnalog_LeftStick_Y), inGamepad_GetAnalog(inGamepadAnalog_LeftStick_X));
		speed = padVec.Len();
		padVec.Normalize();

		movementDir[0] = padVec[0];
		movementDir[1] = padVec[1];
	}

	ueVec3 moveVec = ueVec3::Zero;
	for (u32 i = 0; i < 3; i++)
		if (movementDir[i] != 0.0f)
			moveVec += m_view.GetRow3(i) * movementDir[i];

	if (moveVec != ueVec3::Zero)
	{
		moveVec.Normalize();
		moveVec *= speed * dt * m_movementSpeed;

		m_position += moveVec;
	}

	CalcMatrices();
}

void gxCamera::UpdateEditorCamera(f32 dt, f32 focusDistance)
{
	const ueVec3 focus = m_viewDir * focusDistance + m_position;

	// Get rotation amount

	s32 mouseX, mouseY;
	inMouse_GetVector(mouseX, mouseY);

	// Update movement, rotation and scale

	if (inKeyboard_IsDown(inKey_Alt))
	{
		const ueBool lmb = inMouse_IsDown(inMouseButton_Left);
		const ueBool rmb = inMouse_IsDown(inMouseButton_Right);
		const ueBool mmb = inMouse_IsDown(inMouseButton_Middle);

		if (lmb && !rmb)
		{
			const f32 yawChange = dt * (f32) mouseX * m_rotationSpeed;
			const f32 pitchChange = dt * (f32) mouseY * m_rotationSpeed;

			m_yaw += yawChange;
			m_pitch += pitchChange;

			ueMat44 rotationMatrix;
			rotationMatrix.SetIdentity();
			rotationMatrix.Rotate(ueVec3::UnitY, -m_yaw);
			rotationMatrix.Rotate(ueVec3::UnitX, -m_pitch);
			rotationMatrix.Invert();

			rotationMatrix.TransformCoord(m_position, ueVec3(0, 0, -focusDistance));
			m_position += focus;
		}
		else if (rmb && !lmb)
		{
			ueVec3 offset = m_viewDir;
			offset *= -dt * (f32) mouseY * m_movementSpeed;

			m_position += offset;
		}
		else if (mmb || (rmb && lmb))
		{
			ueVec3 rightMovement = m_viewI.GetRightVec();
			rightMovement *= -(f32) mouseX;

			ueVec3 upMovement = m_viewI.GetUpVec();
			upMovement *= (f32) mouseY;

			ueVec3 offset = rightMovement + upMovement;
			offset *= dt * m_movementSpeed;
			
			m_position += offset;
		}
	}

	CalcMatrices();
}

void gxCamera::UpdateRTSCamera(f32 dt, const RTSCameraParams& params)
{
	const f32 focusDistance = (params.m_focusHeight - m_position[1]) / m_viewDir[1];
	const ueVec3 focus = m_viewDir * focusDistance + m_position;

	// Get rotation

	s32 mouseX, mouseY;
	inMouse_GetVector(mouseX, mouseY);

	// Update rotation

	if (inKeyboard_IsDown(inKey_Alt))
	{
		const f32 yawChange = dt * (f32) mouseX * m_rotationSpeed;
		const f32 pitchChange = dt * (f32) mouseY * m_rotationSpeed;

		m_yaw += yawChange;

		m_pitch += pitchChange;
		m_pitch = ueClamp(m_pitch, params.m_minPitch, params.m_maxPitch);

		ueMat44 rotationMatrix;
		rotationMatrix.SetIdentity();
		rotationMatrix.Rotate(ueVec3::UnitY, -m_yaw);
		rotationMatrix.Rotate(ueVec3::UnitX, -m_pitch);
		rotationMatrix.Invert();

		rotationMatrix.TransformCoord(m_position, ueVec3(0, 0, -focusDistance));
		m_position += focus;
	}

	// Update zooming

	else if (inKeyboard_IsDown(inKey_Control))
	{
		ueVec3 offset = m_viewDir;
		offset *= -dt * (f32) mouseY * m_movementSpeed;

		ueVec3 newPos = m_position + offset;

		const f32 focusHeightDist = newPos[1] - params.m_focusHeight;
		if (focusHeightDist < params.m_minDistance)
		{
			const f32 scale = (params.m_focusHeight + params.m_minDistance - m_position[1]) / m_viewDir[1];
			newPos = m_viewDir * scale + m_position;
		}
		else if (params.m_maxDistance < focusHeightDist)
		{
			const f32 scale = (params.m_focusHeight + params.m_maxDistance - m_position[1]) / m_viewDir[1];
			newPos = m_viewDir * scale + m_position;
		}

		m_position = newPos;
	}

	// Get movement dir

	ueVec3 movementDir = ueVec3::Zero;
	if (inKeyboard_IsValidAny())
	{
		if (inKeyboard_IsDown(inKey_W)) movementDir[0] = 1;
		else if (inKeyboard_IsDown(inKey_S)) movementDir[0] = -1;
		if (inKeyboard_IsDown(inKey_D)) movementDir[1] = 1;
		else if (inKeyboard_IsDown(inKey_A)) movementDir[1] = -1;
		if (inKeyboard_IsDown(inKey_Q)) movementDir[2] = 1;
		else if (inKeyboard_IsDown(inKey_E)) movementDir[2] = -1;
	}

	f32 speed = 1.0f;
	if (inGamepad_IsValidAny())
		if (movementDir == ueVec3::Zero)
		{
			ueVec2 padVec(inGamepad_GetAnalog(inGamepadAnalog_LeftStick_Y), inGamepad_GetAnalog(inGamepadAnalog_LeftStick_X));
			speed = padVec.Len();
			padVec.Normalize();

			movementDir[0] = padVec[0];
			movementDir[1] = padVec[1];
		}

	// Calculate move vector

	ueVec3 moveVec = ueVec3::Zero;
	if (movementDir[0] != 0)
	{
		ueVec3 viewDir = m_viewDir;
		viewDir[1] = 0.0f;
		viewDir.Normalize();
		viewDir *= movementDir[0];

		moveVec += viewDir;
	}

	if (movementDir[1] != 0.0f)
	{
		ueVec3 rightDir = m_view.GetRightVec();
		rightDir[1] = 0.0f;
		rightDir.Normalize();
		rightDir *= movementDir[1];

		moveVec += rightDir;
	}

	if (moveVec != ueVec3::Zero)
	{
		moveVec.Normalize();
		moveVec *= speed * dt * m_movementSpeed;

		m_position += moveVec;
	}

	// Clamp position based on min/max distance

	const f32 focusHeightDist = m_position[1] - params.m_focusHeight;
	if (focusHeightDist < params.m_minDistance)
		m_position[1] = params.m_focusHeight + params.m_minDistance;
	else if (params.m_maxDistance < focusHeightDist)
		m_position[1] = params.m_focusHeight + params.m_maxDistance;

	CalcMatrices();
}

void gxCamera::CalcMatrices()
{
	// Calculate view matrix

	m_view.SetTranslation(-m_position);
	m_view.Rotate(ueVec3::UnitY, -m_yaw);
	m_view.Rotate(ueVec3::UnitX, -m_pitch);

	ueMat44::Invert(m_viewI, m_view);

	// Calculate view direction

	m_viewDir = m_view.GetFrontVec();

	// Calculate proj and inverse

	m_proj.SetPerspectiveFov(m_FOV, m_aspect, m_nearZ, m_farZ, m_drawingToTexture);
	ueMat44::Invert(m_projI, m_proj);

	// Calc view-proj and inverse

	ueMat44::Mul(m_viewProj, m_view, m_proj);
	ueMat44::Invert(m_viewProjI, m_viewProj);

	// Calculate frustum

	m_frustum.Build(m_viewProj);
}