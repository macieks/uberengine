#ifndef IN_SYS_H
#define IN_SYS_H

#include "Input/inKeyboard.h"
#include "Input/inMouse.h"
#include "Input/inGamepad.h"
#include "Input/inTouchpad.h"
#include "Input/inAccelerometer.h"

/**
 *	@addtogroup in
 *	@{
 */

//! Max number of input users
#define IN_MAX_USERS 4

//! Supported input device types
enum inDev
{
	inDev_Keyboard = 0,		//!< Keyboard
	inDev_Mouse,			//!< Mouse
	inDev_Gamepad,			//!< Gamepad (e.g. XBox 360 controller)
	inDev_Touchpad,			//!< Touchpad (e.g. iPhone)
	inDev_Accelerometer,	//!< Accelerometer (e.g. iPhone)

	inDev_MAX
};

//! Input event id
typedef u32 inEventId;

//! Input consumer id
typedef u32 inConsumerId;
//! Callback for when active consumer changes
typedef void (*inActiveConsumerChangedCallback)(const char* consumerName);

//! Input event binding to key set
struct inBinding
{
	//! Max number of keys per binding
	static const u32 MAX_KEYS = 3;

	//! Bound key description
	struct Key
	{
		static const u8 USER_MAPPING_NONE_DEVICE	= UE_POW2(6);
		static const u8 USER_MAPPING_ALL_DEVICES	= UE_POW2(7);

		inDev m_deviceType;					//!< Device type
		u16 m_keyId;						//!< Device specific key / button id
		ueRect m_area;						//!< Touchpad area
		u8 m_userMappings[IN_MAX_USERS];	//!< Mapping from user id to device user id

		UE_INLINE Key() { SetUserMappings_OnlyUser0(); }

		//! Sets up mapping for user 0
		void SetUserMappings_OnlyUser0()
		{
			m_userMappings[0] = USER_MAPPING_ALL_DEVICES;
			for (u32 i = 1; i < IN_MAX_USERS; i++)
				m_userMappings[i] = USER_MAPPING_NONE_DEVICE;
		}

		//! Sets up mapping for all users
		void SetUserMappings_AllUsers()
		{
			for (u32 i = 0; i < IN_MAX_USERS; i++)
				m_userMappings[i] = i;
		}
	};

	u32 m_numKeys;			//!< Number of bound keys
	Key m_keys[MAX_KEYS];	//!< Bound keys

	inBinding() :
		m_numKeys(0)
	{}

	//! Constructs event binding with area
	inBinding(inDev dev, const ueRect& area) :
		m_numKeys(1)
	{
		m_keys[0].m_deviceType = dev;
		m_keys[0].m_area = area;
	}

	//! Constructs event binding with 1 key
	inBinding(inDev dev, u32 key) :
		m_numKeys(1)
	{
		m_keys[0].m_deviceType = dev;
		m_keys[0].m_keyId = key;
	}

	//! Constructs event binding with 2 keys
	inBinding(inDev dev, u32 key0, u32 key1) :
		m_numKeys(2)
	{
		m_keys[0].m_deviceType = dev;
		m_keys[0].m_keyId = key0;
		m_keys[1].m_deviceType = dev;
		m_keys[1].m_keyId = key1;
	}

	//! Constructs event binding with 3 keys
	inBinding(inDev dev, u32 key0, u32 key1, u32 key2) :
		m_numKeys(3)
	{
		m_keys[0].m_deviceType = dev;
		m_keys[0].m_keyId = key0;
		m_keys[1].m_deviceType = dev;
		m_keys[1].m_keyId = key1;
		m_keys[2].m_deviceType = dev;
		m_keys[2].m_keyId = key2;
	}
};

//! Input system startup parameters
struct inSysStartupParams
{
	ueAllocator* m_stackAllocator;	//!< Prefarably stack allocator
	u32 m_maxConsumers;				//!< Max. input consumers
	u32 m_maxEvents;				//!< Max. input events
	u32 m_maxBindings;				//!< Max. input event bindings
	ueBool m_createDebugWindow;		//!< Indicates whether to create debug window (containing available events and their bindings)

	inSysStartupParams() :
		m_stackAllocator(NULL),
		m_maxConsumers(1 << 4),
		m_maxEvents(1 << 8),
		m_maxBindings(1 << 9),
#if defined(UE_FINAL)
		m_createDebugWindow(UE_FALSE)
#else
		m_createDebugWindow(UE_TRUE)
#endif
	{}
};

//! Starts up input system
void inSys_Startup(inSysStartupParams* params);
//! Shuts down input system
void inSys_Shutdown();

//! Updates input system
void inSys_Update(ueBool alsoUpdateAllDevices = UE_TRUE);

//! Sets on-active-consumer-changed callback
void inSys_SetConsumerCallback(inActiveConsumerChangedCallback callback);

//! Registers new input consumer; when priority is set to F32_MAX, the consumer will always be active (unless disabled)
inConsumerId inSys_RegisterConsumer(const char* name, f32 priority = F32_MAX);
//! Unregisters input consumer
void inSys_UnregisterConsumer(inConsumerId& consumerId);
//! Enables or disables given consumer
void inSys_EnableConsumer(inConsumerId consumerId, ueBool enable);
//! Tells whether given consumer is active (which is the case when both: consumer is enabled and either its priority is F32_MAX or it has the highest priority among all other active input consumers)
ueBool inSys_IsConsumerActive(inConsumerId consumerId);

//! Registers names input event and optionally binds it to (up to 3) given bindings
inEventId inSys_RegisterEvent(inConsumerId consumerId, const char* name, const inBinding* binding0 = NULL, const inBinding* binding1 = NULL, const inBinding* binding2 = NULL);
//! Unregisters input event
void inSys_UnregisterEvent(inEventId& eventId);
//! Adds binding for a given input event
void inSys_AddBinding(inEventId eventId, const inBinding& binding);
//! Removes all bindings from an event
void inSys_RemoveAllBindings(inEventId eventId);

//! Tells whether an event is down for a given user
ueBool inSys_IsDown(u32 userId, u32 eventId);
//! Tells whether an event was just pressed for a given user
ueBool inSys_WasPressed(u32 userId, u32 eventId);
//! Tells whether an event was just released for a given user
ueBool inSys_WasReleased(u32 userId, u32 eventId);
//! Tells whether an event is down for any user
ueBool inSys_IsDown(u32 eventId);
//! Tells whether an event was just pressed for any user
ueBool inSys_WasPressed(u32 eventId);
//! Tells whether an event was just released for any user
ueBool inSys_WasReleased(u32 eventId);

// @}

#endif // IN_SYS_H
