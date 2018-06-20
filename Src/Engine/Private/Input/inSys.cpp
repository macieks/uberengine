#include "Input/inSys.h"
#include "Input/inSys_Private.h"
#include "Base/Containers/ueList.h"
#include "Base/Containers/ueGenericPool.h"
#if defined(GX_ENABLE_DEBUG_WINDOWS)
	#include "GraphicsExt/gxDebugWindow.h"
#endif

struct inBinding_Private : inBinding, ueList<inBinding_Private>::Node
{
};

struct inEvent
{
	char m_name[64];
	u8 m_consumerId;
	u16 m_stateMask;

	ueList<inBinding_Private> m_bindings;
};

struct inConsumer
{
	static const u8 ID_MASK					= ((u8) (~UE_POW2(7)));
	static const u8 ALWAYS_ACTIVE_ID_MASK	= UE_POW2(7);

	char m_name[64];
	f32 m_priority;

	ueBool m_isEnabled;
};

struct inSysData
{
	ueAllocator* m_allocator;

	ueGenericPool m_bindingsPool;
	ueGenericPool m_eventsPool;
	ueGenericPool m_consumersPool;

	u8 m_activeConsumerId;

#if defined(GX_ENABLE_DEBUG_WINDOWS)
	gxDebugWindow* m_debugWindow;
#endif

	inActiveConsumerChangedCallback m_callback;

	inSysData() : m_allocator(NULL) {}
};

static inSysData* s_data = NULL;

void inSys_UpdateDebugWindow();

// Input system

void inSys_Startup(inSysStartupParams* params)
{
	UE_ASSERT(!s_data);

	s_data = new(params->m_stackAllocator) inSysData();
	UE_ASSERT(s_data);
	s_data->m_allocator = params->m_stackAllocator;

	UE_ASSERT_FUNC(s_data->m_bindingsPool.Init(params->m_stackAllocator, sizeof(inBinding_Private), params->m_maxBindings));
	UE_ASSERT_FUNC(s_data->m_eventsPool.Init(params->m_stackAllocator, sizeof(inEvent), params->m_maxEvents, UE_DEFAULT_ALIGNMENT, UE_TRUE));
	UE_ASSERT_FUNC(s_data->m_consumersPool.Init(params->m_stackAllocator, sizeof(inConsumer), params->m_maxConsumers, UE_DEFAULT_ALIGNMENT, UE_TRUE));

	s_data->m_activeConsumerId = U8_MAX;
	s_data->m_callback = NULL;

#if defined(GX_ENABLE_DEBUG_WINDOWS)
	if (gxDebugWindowSys_IsInitialized() && params->m_createDebugWindow)
	{
		gxDebugWindowDesc desc;
		desc.m_name = "Active Inputs";
		s_data->m_debugWindow = gxDebugWindow_Create(&desc);
	}
	else
		s_data->m_debugWindow = NULL;
#endif // defined(GX_ENABLE_DEBUG_WINDOWS)
}

void inSys_Shutdown()
{
	UE_ASSERT(s_data);

#if defined(GX_ENABLE_DEBUG_WINDOWS)
	if (s_data->m_debugWindow)
		gxDebugWindow_Destroy(s_data->m_debugWindow);
#endif

	s_data->m_consumersPool.Deinit();
	s_data->m_eventsPool.Deinit();
	s_data->m_bindingsPool.Deinit();

	ueDelete(s_data, s_data->m_allocator);
	s_data->m_allocator = NULL;
}

UE_INLINE ueBool inSys_IsDown(inDev dt, u32 keyId)
{
	switch (dt)
	{
		case inDev_Keyboard: return inKeyboard_IsDown((inKey) keyId);
#if defined(IN_ENABLE_MOUSE)
		case inDev_Mouse: return inMouse_IsDown((inMouseButton) keyId);
#endif
#if defined(IN_ENABLE_GAMEPAD)
		case inDev_Gamepad: return inGamepad_IsDown((inGamepadButton) keyId);
#endif
	}
	return UE_FALSE;
}

UE_INLINE ueBool inSys_IsDown(inDev dt, u32 userId, u32 keyId)
{
	switch (dt)
	{
		case inDev_Keyboard: return inKeyboard_IsDown(userId, (inKey) keyId);
#if defined(IN_ENABLE_MOUSE)
		case inDev_Mouse: return inMouse_IsDown(userId, (inMouseButton) keyId);
#endif
#if defined(IN_ENABLE_GAMEPAD)
		case inDev_Gamepad: return inGamepad_IsDown(userId, (inGamepadButton) keyId);
#endif
	}
	return UE_FALSE;
}

void inSys_Update(ueBool alsoUpdateAllDevices)
{
	UE_PROF_SCOPE("inSys_Update");

	// Update devices (optional)

	if (alsoUpdateAllDevices)
	{
		UE_PROF_SCOPE("Update Input Devices");
		{
			UE_PROF_SCOPE("Update Keyboard");
			inKeyboard_Update();
		}
#if defined(IN_ENABLE_MOUSE)
		{
			UE_PROF_SCOPE("Update Mouse");
			inMouse_Update(UE_TRUE);
		}
#endif // defined(IN_ENABLE_MOUSE)
#if defined(IN_ENABLE_GAMEPAD)
		{
			UE_PROF_SCOPE("Update Gamepad");
			inGamepad_Update();
		}
#endif
#if defined(IN_ENABLE_TOUCHPAD)
		{
			UE_PROF_SCOPE("Update Touchpad");
			inTouchpad_Update();
		}
#endif
#if defined(IN_ENABLE_ACCELEROMETER)
		{
			UE_PROF_SCOPE("Update Accelerometer");
			inAccelerometer_Update();
		}
#endif
	}

	// Update input events state

	ueGenericPool::Iterator iter(s_data->m_eventsPool);
	while (inEvent* e = (inEvent*) iter.Next())
	{
		// Store old "was" state

		e->m_stateMask &= ~inKeyStateFlags_WasDown_AllUsers;
		e->m_stateMask |= (e->m_stateMask & inKeyStateFlags_IsDown_AllUsers) << IN_MAX_USERS;
		e->m_stateMask &= ~inKeyStateFlags_IsDown_AllUsers;

		// Determine new "is" state

		inBinding_Private* b = e->m_bindings.Front();
		while (b)
		{
			for (u32 i = 0; i < IN_MAX_USERS; i++)
			{
				ueBool isActive = UE_TRUE;
				for (u32 k = 0; k < b->m_numKeys; k++)
				{
					const inBinding::Key& key = b->m_keys[k];
					if (key.m_userMappings[i] == inBinding::Key::USER_MAPPING_ALL_DEVICES)
						isActive = inSys_IsDown(key.m_deviceType, key.m_keyId);
					else if (key.m_userMappings[i] == inBinding::Key::USER_MAPPING_NONE_DEVICE)
						isActive = UE_FALSE;
					else
						isActive = inSys_IsDown(key.m_deviceType, key.m_userMappings[i], key.m_keyId);
				}
				if (isActive)
					e->m_stateMask |= inKeyStateFlags_IsDown_User0 << i;
			}

			b = b->Next();
		}
	}

	// Update debug window

#if defined(GX_ENABLE_DEBUG_WINDOWS)
	if (s_data->m_debugWindow)
		inSys_UpdateDebugWindow();
#endif // defined(GX_ENABLE_DEBUG_WINDOWS)
}

// Consumers

void inSys_SetConsumerCallback(inActiveConsumerChangedCallback callback)
{
	s_data->m_callback = callback;
}

void inSys_RefreshActiveConsumer()
{
	inConsumer* highestPriorityConsumer = NULL;

	ueGenericPool::Iterator iter(s_data->m_consumersPool);
	while (inConsumer* consumer = (inConsumer*) iter.Next())
		if (consumer->m_isEnabled && consumer->m_priority != F32_MAX &&
			(!highestPriorityConsumer || consumer->m_priority > highestPriorityConsumer->m_priority))
			highestPriorityConsumer = consumer;

	const u32 oldConsumerId = s_data->m_activeConsumerId;
	s_data->m_activeConsumerId = highestPriorityConsumer ? s_data->m_consumersPool.GetIndex(highestPriorityConsumer) : U8_MAX;

	if (s_data->m_callback && oldConsumerId != s_data->m_activeConsumerId && s_data->m_activeConsumerId != U8_MAX)
		s_data->m_callback(highestPriorityConsumer->m_name);
}

inConsumerId inSys_RegisterConsumer(const char* name, f32 priority)
{
	inConsumer* consumer = new(s_data->m_consumersPool) inConsumer;
	ueStrCpyS(consumer->m_name, name);
	consumer->m_priority = priority;
	consumer->m_isEnabled = UE_TRUE;
	inSys_RefreshActiveConsumer();
	return s_data->m_consumersPool.GetIndex(consumer) | (priority == F32_MAX ? inConsumer::ALWAYS_ACTIVE_ID_MASK : 0);
}

void inSys_UnregisterConsumer(inConsumerId& consumerId)
{
	// Remove all events of this consumer

	ueGenericPool::Iterator iter(s_data->m_eventsPool);
	while (inEvent* e = (inEvent*) iter.Next())
		if (e->m_consumerId == consumerId)
			s_data->m_eventsPool.Free(e);

	s_data->m_consumersPool.FreeIndex(consumerId & inConsumer::ID_MASK);
	inSys_RefreshActiveConsumer();
	consumerId = U8_MAX;
}

void inSys_EnableConsumer(inConsumerId consumerId, ueBool enable)
{
	inConsumer* consumer = (inConsumer*) s_data->m_consumersPool.GetBlock(consumerId & inConsumer::ID_MASK);
	if (consumer->m_isEnabled == enable)
		return;

	consumer->m_isEnabled = enable;
	inSys_RefreshActiveConsumer();
}

ueBool inSys_IsConsumerActive(inConsumerId consumerId)
{
	return (consumerId & inConsumer::ALWAYS_ACTIVE_ID_MASK) || (consumerId & inConsumer::ID_MASK) == s_data->m_activeConsumerId;
}

// Events & bindings

inEventId inSys_RegisterEvent(inConsumerId consumerId, const char* name, const inBinding* binding0, const inBinding* binding1, const inBinding* binding2)
{
	UE_ASSERT(s_data->m_consumersPool.IsValidIndex(consumerId & inConsumer::ID_MASK));

	inEvent* ev = new(s_data->m_eventsPool) inEvent;
	ev->m_consumerId = consumerId;
	ueStrCpyS(ev->m_name, name);
	ev->m_stateMask = 0;

	const inEventId eventId = s_data->m_eventsPool.GetIndex(ev);

	if (binding0)
		inSys_AddBinding(eventId, *binding0);
	if (binding1)
		inSys_AddBinding(eventId, *binding1);
	if (binding2)
		inSys_AddBinding(eventId, *binding2);

	return eventId;
}

void inSys_UnregisterEvent(inEventId& eventId)
{
	s_data->m_eventsPool.FreeIndex(eventId);
	eventId = U8_MAX;
}

void inSys_AddBinding(inEventId eventId, const inBinding& binding)
{
	inBinding_Private* newBinding = new(s_data->m_bindingsPool) inBinding_Private;
	ueMemCpy(newBinding, &binding, sizeof(inBinding));

	inEvent* ev = (inEvent*) s_data->m_eventsPool.GetBlock(eventId);
	ev->m_bindings.PushBack(newBinding);
}

void inSys_RemoveAllBindings(inEventId eventId)
{
	inEvent* ev = (inEvent*) s_data->m_eventsPool.GetBlock(eventId);
	while (!ev->m_bindings.IsEmpty())
		s_data->m_bindingsPool.Free( ev->m_bindings.PopFront() );
}

// Per user state

ueBool inSys_IsDown(u32 userId, u32 eventId)
{
	UE_ASSERT(userId < IN_MAX_USERS);
	inEvent* ev = (inEvent*) s_data->m_eventsPool.GetBlock(eventId);
	return
		((ev->m_consumerId & inConsumer::ALWAYS_ACTIVE_ID_MASK) || (ev->m_consumerId == s_data->m_activeConsumerId)) &&
		(ev->m_stateMask & (inKeyStateFlags_IsDown_User0 + userId)) != 0;
}

ueBool inSys_WasPressed(u32 userId, u32 eventId)
{
	UE_ASSERT(userId < IN_MAX_USERS);
	inEvent* ev = (inEvent*) s_data->m_eventsPool.GetBlock(eventId);
	return
		((ev->m_consumerId & inConsumer::ALWAYS_ACTIVE_ID_MASK) || (ev->m_consumerId == s_data->m_activeConsumerId)) &&
		(ev->m_stateMask & (inKeyStateFlags_WasDown_User0 + userId)) == 0 &&
		(ev->m_stateMask & (inKeyStateFlags_IsDown_User0 + userId)) != 0;
}

ueBool inSys_WasReleased(u32 userId, u32 eventId)
{
	UE_ASSERT(userId < IN_MAX_USERS);
	inEvent* ev = (inEvent*) s_data->m_eventsPool.GetBlock(eventId);
	return
		((ev->m_consumerId & inConsumer::ALWAYS_ACTIVE_ID_MASK) || (ev->m_consumerId == s_data->m_activeConsumerId)) &&
		(ev->m_stateMask & (inKeyStateFlags_WasDown_User0 + userId)) != 0 &&
		(ev->m_stateMask & (inKeyStateFlags_IsDown_User0 + userId)) == 0;
}

// Merged state

ueBool inSys_IsDown(u32 eventId)
{
	inEvent* ev = (inEvent*) s_data->m_eventsPool.GetBlock(eventId);
	return
		((ev->m_consumerId & inConsumer::ALWAYS_ACTIVE_ID_MASK) || (ev->m_consumerId == s_data->m_activeConsumerId)) &&
		(ev->m_stateMask & inKeyStateFlags_WasDown_AllUsers) != 0;
}

ueBool inSys_WasPressed(u32 eventId)
{
	inEvent* ev = (inEvent*) s_data->m_eventsPool.GetBlock(eventId);
	return
		((ev->m_consumerId & inConsumer::ALWAYS_ACTIVE_ID_MASK) || (ev->m_consumerId == s_data->m_activeConsumerId)) &&
		(ev->m_stateMask & inKeyStateFlags_WasDown_AllUsers) == 0 &&
		(ev->m_stateMask & inKeyStateFlags_IsDown_AllUsers) != 0;
}

ueBool inSys_WasReleased(u32 eventId)
{
	inEvent* ev = (inEvent*) s_data->m_eventsPool.GetBlock(eventId);
	return
		((ev->m_consumerId & inConsumer::ALWAYS_ACTIVE_ID_MASK) || (ev->m_consumerId == s_data->m_activeConsumerId)) &&
		(ev->m_stateMask & inKeyStateFlags_WasDown_AllUsers) != 0 &&
		(ev->m_stateMask & inKeyStateFlags_IsDown_AllUsers) == 0;
}

void inSys_UpdateDebugWindow()
{
	char buffer[1024];
	buffer[0] = 0;
	char* bufferPtr = (char*) buffer;

	ueBool isFirstEvent = UE_TRUE;

	// For each (active) consumer

	ueGenericPool::Iterator consumersIter(s_data->m_consumersPool);
	while (inConsumer* consumer = (inConsumer*) consumersIter.Next())
	{
		inConsumerId consumerId = s_data->m_consumersPool.GetIndex(consumer);
		if (!inSys_IsConsumerActive(consumerId))
			continue;

		// For each event of that consumer

		ueGenericPool::Iterator eventsIter(s_data->m_eventsPool);
		while (inEvent* ev = (inEvent*) eventsIter.Next())
			if (ev->m_consumerId == consumerId)
			{
				if (isFirstEvent)
					isFirstEvent = UE_FALSE;
				else
					ueStrCat2(bufferPtr, "\n");

				ueStrCat2(bufferPtr, "[");
				ueStrCat2(bufferPtr, ev->m_name);
				ueStrCat2(bufferPtr, "]: ");

				// Output all bindings

				ueBool isFirstBinding = UE_TRUE;
				
				for (inBinding_Private* binding = ev->m_bindings.Front(); binding; binding = binding->Next())
				{
					if (isFirstBinding)
						isFirstBinding = UE_FALSE;
					else
						ueStrCat2(bufferPtr, " <or> ");

					ueBool isFirstKey = UE_TRUE;

					// Output all keys in the binding

					for (u32 i = 0; i < binding->m_numKeys; i++)
					{
						if (isFirstKey)
							isFirstKey = UE_FALSE;
						else
							ueStrCat2(bufferPtr, "+");

						const inBinding::Key& key = binding->m_keys[i];
						const char* keyName = NULL;
						switch (key.m_deviceType)
						{
							case inDev_Keyboard: keyName = inKeyboard_GetKeyName((inKey) key.m_keyId); break;
#if defined(IN_ENABLE_MOUSE)
							case inDev_Mouse: keyName = inMouse_GetButtonName((inMouseButton) key.m_keyId); break;
#endif // defined(IN_ENABLE_MOUSE)
#if defined(IN_ENABLE_GAMEPAD)
							case inDev_Gamepad: keyName = inGamepad_GetButtonName((inGamepadButton) key.m_keyId); break;
#endif // defined(IN_ENABLE_GAMEPAD)
#if defined(IN_ENABLE_TOUCHPAD)
							case inDev_Touchpad: keyName = "touchpad-area"; break;
#endif // defined(IN_ENABLE_TOUCHPAD)
						}
						ueStrCat2(bufferPtr, keyName);
					}
				}
			}
	}

	// Update window content

#if defined(GX_ENABLE_DEBUG_WINDOWS)
	gxDebugWindow_SetContent(s_data->m_debugWindow, buffer);
#endif // defined(GX_ENABLE_DEBUG_WINDOWS)
}