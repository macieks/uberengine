#ifndef IN_ACCELEROMETER_H
#define IN_ACCELEROMETER_H

#include "Base/ueBase.h"

//! Device (smartphone/tablet) position
enum inDevicePosition
{
	inDevicePosition_LandscapeLeft = 0,
	inDevicePosition_LandscapeRight,
	inDevicePosition_PortraitLeft,
	inDevicePosition_PortraitRight,

	inDevicePosition_None,

	inDevicePosition_MAX
};

void inAccelerometer_Startup();
void inAccelerometer_Shutdown();

//! Gets current device position (regardless of any settings)
inDevicePosition inAccelerometer_GetCurrentPosition();
//! Sets fixed application position
void inAccelerometer_SetFixedPosition(inDevicePosition fixPos);
//! Gets fixed application position
inDevicePosition inAccelerometer_GetFixedPosition();

//! Updates accelerometer state
void inAccelerometer_Update();

void inAccelerometer_UpdateFixedPosition();
ueBool inAccelerometer_IsFixedPositionInverted();

//! Gets whether accelerometer is available
ueBool inAccelerometer_IsAvailable();
//! Gets accelerometer tilting values in range -1..1
void inAccelerometer_GetOrientation(f32& x, f32& y, f32& z);
//! Gets previous frame's accelerometer tilting values in range -1..1
void inAccelerometer_GetPrevOrientation(f32& x, f32& y, f32& z);

#endif // IN_ACCELEROMETER_H