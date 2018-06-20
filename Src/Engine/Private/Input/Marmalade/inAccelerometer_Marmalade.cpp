#include "Input/inAccelerometer.h"

static inDevicePosition s_fixPos = inDevicePosition_None;

static f32 s_x = 0.0f;
static f32 s_y = 0.0f;
static f32 s_z = 0.0f;

static f32 s_prevX = 0.0f;
static f32 s_prevY = 0.0f;
static f32 s_prevZ = 0.0f;

void inAccelerometer_Startup()
{
	s3eAccelerometerStart();
}

void inAccelerometer_Shutdown()
{
	s3eAccelerometerStop();
}

inDevicePosition inAccelerometer_GetCurrentPosition()
{
	switch (s3eSurfaceGetInt(S3E_SURFACE_DEVICE_BLIT_DIRECTION))
	{
		case S3E_SURFACE_BLIT_DIR_NORMAL: return inDevicePosition_PortraitRight;
		case S3E_SURFACE_BLIT_DIR_ROT90: return inDevicePosition_LandscapeRight;
		case S3E_SURFACE_BLIT_DIR_ROT180: return inDevicePosition_PortraitLeft;
		case S3E_SURFACE_BLIT_DIR_ROT270: return inDevicePosition_LandscapeLeft;
	}
	return inDevicePosition_LandscapeLeft;
}

void inAccelerometer_SetFixedPosition(inDevicePosition fixPos)
{
	s_fixPos = fixPos;
}

inDevicePosition inAccelerometer_GetFixedPosition()
{
	return s_fixPos;
}

void inAccelerometer_UpdateFixedPosition()
{
	inAccelerometer_SetFixedPosition(inAccelerometer_GetCurrentPosition());
}

void inAccelerometer_Update()
{
	int32 accelX = s3eAccelerometerGetX();
	int32 accelY = s3eAccelerometerGetY();
	int32 accelZ = s3eAccelerometerGetZ();

	// Rotate acceleration vector based on the actual current device orientation

	if (inAccelerometer_IsFixedPositionInverted())
	{
		accelX = -accelX;
		accelY = -accelY;
	}

	// Normalize to -1..1 range

	s_x = (f32) accelX * 0.001f;
	s_y = (f32) accelY * 0.001f;
	s_z = (f32) accelZ * 0.001f;
}

ueBool inAccelerometer_IsFixedPositionInverted()
{
	const inDevicePosition currentDevicePosition = inAccelerometer_GetCurrentPosition();
	switch (s_fixPos)
	{
		case inDevicePosition_LandscapeLeft: return currentDevicePosition == inDevicePosition_LandscapeRight;
		case inDevicePosition_LandscapeRight: return currentDevicePosition == inDevicePosition_LandscapeLeft;
		case inDevicePosition_PortraitLeft: return currentDevicePosition == inDevicePosition_PortraitRight;
		case inDevicePosition_PortraitRight: return currentDevicePosition == inDevicePosition_PortraitLeft;
	}
	return UE_FALSE;
}

ueBool inAccelerometer_IsAvailable()
{
	return s3eAccelerometerGetInt(S3E_ACCELEROMETER_AVAILABLE) ? UE_TRUE : UE_FALSE;
}

void inAccelerometer_GetOrientation(f32& x, f32& y, f32& z)
{
	x = s_x;
	y = s_y;
	z = s_z;
}

void inAccelerometer_GetPrevOrientation(f32& x, f32& y, f32& z)
{
	x = s_prevX;
	y = s_prevY;
	z = s_prevZ;
}