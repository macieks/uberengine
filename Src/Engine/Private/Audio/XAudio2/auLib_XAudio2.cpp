#include "Base/Containers/ueGenericPool.h"
#include "Base/Containers/ueList.h"
#include "Base/Containers/ueRingBuffer.h"
#include "IO/ioFile.h"
#include "Base/ueThreading.h"
#include "Audio/auLib.h"

#if defined(UE_WIN32)
	#include <objbase.h>
	#define AU_CONFIG_NAME "win32-xact"
#elif defined(UE_X360)
	#define AU_CONFIG_NAME "x360-xact"
#endif

#include <xact3.h>
#include <xact3d3.h>

struct auSoundBank : public ueList<auSoundBank>::Node
{
	char* m_name;

	void* m_waveBankFile;
	IXACT3WaveBank* m_waveBank;

	void* m_soundBankFile;
	IXACT3SoundBank* m_soundBank;

	u32 m_numSounds;
	auSoundId* m_soundIds;
};

struct auSoundId
{
	XACTINDEX m_cueIndex;
	auSoundBank* m_bank;
};

struct auSound : public ueList<auSound>::Node
{
	IXACT3Cue* m_cue;
	ueBool m_isUsed;
	ueBool m_is3D;
	ueBool m_destroyOnFinish;
	ueBool m_toBeDestroyed;
	ueVec3 m_position;

	auSoundHandle* m_handlePtr;
};

struct auDynamicSound : public IXAudio2VoiceCallback
{
	ueAllocator* m_allocator;

	IXAudio2SourceVoice* m_srcVoice;

	volatile u32 m_numSubmittedBuffers;
	ueRingBuffer m_ringBuffer;
	ueMutex m_mutex;

	// Implementation of IXAudio2VoiceCallback

	void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) {}
	void __stdcall OnVoiceProcessingPassEnd() {}
	void __stdcall OnStreamEnd() {}
	void __stdcall OnBufferStart(void* pBufferContext) {}
	void __stdcall OnBufferEnd(void* pBufferContext);
	void __stdcall OnLoopEnd(void* pBufferContext) {}
	void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error)
	{
	}
};

struct auLibData
{
	ueAllocator* m_stackAllocator;
	ueAllocator* m_freqAllocator;

	ueMutex m_mutex;

	IXAudio2* m_xaudio2;

	IXACT3Engine* m_engine;
	uePath m_settingsFileName;
	void* m_settingsFile;
	XACT_RENDERER_DETAILS m_renderer;

	X3DAUDIO_DSP_SETTINGS m_dspSettings;
	X3DAUDIO_LISTENER m_listener;
	X3DAUDIO_EMITTER m_emitter;
	FLOAT32 m_delayTimes[2];
	FLOAT32 m_matrixCoefficients[2 * 8];
	X3DAUDIO_HANDLE m_3DInstance;

	ueList<auSoundBank> m_banks;

	ueList<auSound> m_sounds;
	ueGenericPool m_soundsPool;

	auLibData() :
		m_stackAllocator(NULL),
		m_xaudio2(NULL),
		m_engine(NULL),
		m_settingsFile(NULL)
	{}
};

static auLibData* s_data = NULL;

void __stdcall auLib_XACTNotificationCallback(const XACT_NOTIFICATION* notification)
{
	UE_ASSERT(s_data);

	if (notification->type == XACTNOTIFICATIONTYPE_CUEDESTROYED)
	{
		ueMutexLock lock(s_data->m_mutex);
		
		for (auSound* sound = s_data->m_sounds.Front(); sound; sound = sound->Next())
			if (sound->m_cue == notification->cue.pCue)
			{
				sound->m_cue = NULL;
				return;
			}
	}
}

ueBool auLib_StartupXAudio2();
void auLib_ShutdownXAudio2();

ueBool auLib_StartupXACT();
void auLib_ShutdownXACT();

class auLib_XAudio2EngineCallback : public IXAudio2EngineCallback
{
public:
    void _stdcall OnProcessingPassEnd () {}
    void _stdcall OnProcessingPassStart() {}
    void _stdcall OnCriticalError (HRESULT Error)
	{
		auLib_ShutdownXACT();
		auLib_ShutdownXAudio2();
	}
};

static auLib_XAudio2EngineCallback s_xaudio2Callback;

void auLib_Startup(auLibStartupParams* params)
{
	UE_ASSERT(!s_data);

	s_data = new(params->m_stackAllocator) auLibData();
	UE_ASSERT(s_data);
	s_data->m_stackAllocator = params->m_stackAllocator;
	s_data->m_freqAllocator = params->m_freqAllocator;

	UE_ASSERT_FUNC(s_data->m_soundsPool.Init(s_data->m_stackAllocator, sizeof(auSound), params->m_maxSounds));
	ueStrCpyS(s_data->m_settingsFileName, params->m_settingsFileName);

#if defined(UE_WIN32)
	HRESULT hr = CoInitialize(NULL);
	UE_ASSERT(SUCCEEDED(hr));
#endif

	if (!auLib_StartupXAudio2())
		return;

	if (!auLib_StartupXACT())
	{
		auLib_ShutdownXACT();
		auLib_ShutdownXAudio2();
		return;
	}
}

ueBool auLib_StartupXAudio2()
{
	HRESULT hr;

	hr = XAudio2Create(&s_data->m_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		ueLogE("XAudio2Create failed, hr = 0x%x", hr);
		return UE_FALSE;
	}

	s_data->m_xaudio2->RegisterForCallbacks(&s_xaudio2Callback);
	return UE_TRUE;
}

ueBool auLib_StartupXACT()
{
	HRESULT hr;

	// Create XACT engine

	DWORD creationFlags = 0;
	//creationFlags |= XACT_FLAG_API_AUDITION_MODE;
#if defined(UE_DEBUG)
	creationFlags |= XACT_FLAG_API_DEBUG_MODE;
#endif
	hr = XACT3CreateEngine(creationFlags, &s_data->m_engine);
	if (FAILED(hr))
	{
		ueLogE("XACT3CreateEngine failed, hr = 0x%x", hr);
		return UE_FALSE;
	}

	// Check available renderers

	XACTINDEX numRenderers = 0;
	s_data->m_engine->GetRendererCount(&numRenderers);
	if (numRenderers == 0)
	{
		ueLogW("No valid uedio renderers found.");
		return UE_FALSE;
	}

	ueLogI("Audio renderers [%u]:", (u32) numRenderers);
	for (XACTINDEX i = 0; i < numRenderers; i++)
	{
		XACT_RENDERER_DETAILS renderer;
		if (FAILED(s_data->m_engine->GetRendererDetails(i, &renderer)))
			ueLogI("  [%u] <Invalid>");
		else
		{
			if (renderer.defaultDevice)
				ueMemCpy(&s_data->m_renderer, &renderer, sizeof(XACT_RENDERER_DETAILS));

			ueLogI("  [%u] Default = %s, Name = %S",
				(u32) i,
				renderer.defaultDevice ? "YES" : "NO",
				renderer.displayName);
		}
	}

	// Load global settings file

	uePath settingsPath;
	ueAssets_GetAssetPath(settingsPath, s_data->m_settingsFileName, "xgs", AU_CONFIG_NAME);

	ueSize settingsSize = 0;
	if (!ioFile_Load(settingsPath, s_data->m_settingsFile, settingsSize, 0, s_data->m_stackAllocator))
	{
		ueLogE("Failed to load XACT settings file (path = '%s')", settingsPath);
		return UE_FALSE;
	}

	// Create misc. uedio settings

	const D3DVECTOR unitZVec = {0, 0, 1};
	const D3DVECTOR unitYVec = {0, 1, 0};
	const D3DVECTOR zeroVec = {0, 0, 0};

	ZeroMemory(&s_data->m_listener, sizeof(X3DAUDIO_LISTENER));
	s_data->m_listener.OrientFront = unitZVec;
	s_data->m_listener.OrientTop = unitYVec;
	s_data->m_listener.Position = zeroVec;
	s_data->m_listener.Velocity = zeroVec;

	ZeroMemory(&s_data->m_emitter, sizeof(X3DAUDIO_EMITTER));
	s_data->m_emitter.pCone = NULL;
	s_data->m_emitter.OrientFront = unitZVec;
	s_data->m_emitter.OrientTop = unitYVec;
	s_data->m_emitter.Position = zeroVec;
	s_data->m_emitter.Velocity = zeroVec;
	s_data->m_emitter.ChannelCount = 2;
	s_data->m_emitter.ChannelRadius = 1.0f;
	s_data->m_emitter.pChannelAzimuths = NULL;
	s_data->m_emitter.pVolumeCurve = NULL;
	s_data->m_emitter.pLFECurve = NULL;
	s_data->m_emitter.pLPFDirectCurve = NULL;
	s_data->m_emitter.pLPFReverbCurve = NULL;
	s_data->m_emitter.pReverbCurve = NULL;
	s_data->m_emitter.CurveDistanceScaler = 1.0f;
	s_data->m_emitter.DopplerScaler = NULL;

	s_data->m_delayTimes[0] = 0.0f;
	s_data->m_delayTimes[1] = 0.0f;

	ZeroMemory(&s_data->m_matrixCoefficients, sizeof(FLOAT32) * 8 * 2);

	// Initialize & create the XACT runtime

	XACT_RUNTIME_PARAMETERS xrParams = {0};
	xrParams.pGlobalSettingsBuffer = s_data->m_settingsFile;
	xrParams.globalSettingsBufferSize = (DWORD) settingsSize;
	xrParams.globalSettingsFlags = 0;
	xrParams.fnNotificationCallback = auLib_XACTNotificationCallback;
	xrParams.lookAheadTime = 250;
	xrParams.pXAudio2 = s_data->m_xaudio2;
	hr = s_data->m_engine->Initialize(&xrParams);
	if (FAILED(hr))
	{
		ueLogE("XACTEngine->Initialize failed, hr = 0x%x", hr);
		return UE_FALSE;
	}

	// Query number of channels on the final mix

	WAVEFORMATEXTENSIBLE wfxFinalMixFormat;
	hr = s_data->m_engine->GetFinalMixFormat(&wfxFinalMixFormat);
	if (FAILED(hr))
	{
		ueLogE("XACTEngine->GetFinalMixFormat failed, hr = 0x%x", hr);
		return UE_FALSE;
	}

	// Initialize 3D settings

	ZeroMemory(&s_data->m_dspSettings, sizeof(X3DAUDIO_DSP_SETTINGS));
	s_data->m_dspSettings.pMatrixCoefficients = s_data->m_matrixCoefficients;
	s_data->m_dspSettings.pDelayTimes = s_data->m_delayTimes;
	s_data->m_dspSettings.SrcChannelCount = 2;
	s_data->m_dspSettings.DstChannelCount = wfxFinalMixFormat.Format.nChannels;

	hr = XACT3DInitialize(s_data->m_engine, s_data->m_3DInstance);
	if (FAILED(hr))
	{
		ueLogE("XACT3DInitialize failed, hr = 0x%x", hr);
		return UE_FALSE;
	}

	// Register for XACT notifications

	XACT_NOTIFICATION_DESCRIPTION notificationDesc = {0};
	notificationDesc.flags = XACT_FLAG_NOTIFICATION_PERSIST;
	notificationDesc.type = XACTNOTIFICATIONTYPE_CUEDESTROYED;
	notificationDesc.cueIndex = XACTINDEX_INVALID;
	notificationDesc.pSoundBank = NULL;
	hr = s_data->m_engine->RegisterNotification(&notificationDesc);
	if (FAILED(hr))
	{
		ueLogE("XACTEngine->RegisterNotification failed, hr = 0x%x", hr);
		return UE_FALSE;
	}

	return UE_TRUE;
}

void auLib_ShutdownXACT()
{
	ueMutexLock lock(s_data->m_mutex);

	// Nullify all sounds

	for (auSound* sound = s_data->m_sounds.Front(); sound; sound = sound->Next())
		sound->m_cue = NULL;

	// Unload all sound banks

	auSoundBank_UnloadAll();

	// Shutdown XACT engine

	if (s_data->m_engine)
	{
		s_data->m_engine->ShutDown();
		s_data->m_engine->Release();
		s_data->m_engine = NULL;
	}

	// Unload XACT settings file

	if (s_data->m_settingsFile)
	{
		s_data->m_stackAllocator->Free(s_data->m_settingsFile);
		s_data->m_settingsFile = NULL;
	}
}

void auLib_ShutdownXAudio2()
{
	if (!s_data->m_xaudio2)
		return;

	s_data->m_xaudio2->Release();
	s_data->m_xaudio2 = NULL;
}

void auLib_Shutdown()
{
	UE_ASSERT(s_data);

	auLib_Update(0.0f); // Handle deletion of stopped sounds
	auLib_ShutdownXACT();
	auLib_ShutdownXAudio2();

#if defined(UE_WIN32)
	CoUninitialize();
#endif

	s_data->m_soundsPool.Deinit();
	ueDelete(s_data, s_data->m_stackAllocator);
	s_data = NULL;
}

void auSoundBank_UnloadAll()
{
	while (auSoundBank* bank = s_data->m_banks.Front())
		auSoundBank_Unload(bank);
}

void auLib_Pause(ueBool pause, const char* categoryName)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return; 

	XACTCATEGORY category = s_data->m_engine->GetCategory(categoryName);
	if (category == XACTCATEGORY_INVALID)
		return;

	s_data->m_engine->Pause(category, pause ? TRUE : FALSE);
}

ueBool auLib_IsValid()
{
	UE_ASSERT(s_data);
	return s_data->m_engine != NULL;
}

auSoundBank* auSoundBank_Load(const char* name)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return NULL;

#if defined(UE_DEBUG)

	// Verify bank isn't already loaded

	for (auSoundBank* bank = s_data->m_banks.Front(); bank; bank = bank->Next())
		UE_ASSERT(ueStrCmp(bank->m_name, name));

#endif

	// Determine size of wave & sound banks

	HRESULT hr;

	uePath waveBankPath;
	ueAssets_GetAssetPath(waveBankPath, name, "xwb", AU_CONFIG_NAME);

	ueSize waveBankSize;
	if (!ioFile_GetSize(waveBankPath, waveBankSize))
	{
		ueLogE("Failed to load XWB wave bank, reason: file doesn't exist (path = '%s')", waveBankPath);
		return NULL;
	}

	uePath soundBankPath;
	ueAssets_GetAssetPath(soundBankPath, name, "xsb", AU_CONFIG_NAME);

	ueSize soundBankSize;
	if (!ioFile_GetSize(soundBankPath, soundBankSize))
	{
		ueLogE("Failed to load XSB sound bank, reason: file doesn't exist (path = '%s')", soundBankPath);
		return NULL;
	}

	// Allocate memory

	const u32 nameLength = ueStrLen(name);

	const ueSize memorySize = sizeof(auSoundBank) + nameLength + 1 + (u32) waveBankSize + (u32) soundBankSize;
	u8* memory = (u8*) s_data->m_freqAllocator->Alloc(memorySize);
	if (!memory)
	{
		ueLogE("Failed to create sound bank (name = '%s'), reason: not enough memory (needed %u bytes)", name, (u32) memorySize);
		return NULL;
	}

	auSoundBank* bank = new(memory) auSoundBank();
	bank->m_waveBank = NULL;
	bank->m_soundBank = NULL;
	memory += sizeof(auSoundBank);

	bank->m_name = (char*) memory;
	ueStrCpy(bank->m_name, nameLength + 1, name);
	memory += nameLength + 1;

	bank->m_waveBankFile = memory;
	memory += waveBankSize;

	bank->m_soundBankFile = memory;

	XACTINDEX numSounds;

	// Load wave and sound banks

	if (!ioFile_Load(waveBankPath, bank->m_waveBankFile, waveBankSize))
	{
		ueLogE("Failed to load XWB wave bank, reason: can't load file (path = '%s')", waveBankPath);
		goto Failure;
	}

	if (!ioFile_Load(soundBankPath, bank->m_soundBankFile, soundBankSize))
	{
		ueLogE("Failed to load XSB sound bank, reason: can't load file (path = '%s')", soundBankPath);
		goto Failure;
	}

	// Create wave and sound bank objects

	hr = s_data->m_engine->CreateInMemoryWaveBank(bank->m_waveBankFile, (DWORD) waveBankSize, 0, 0, &bank->m_waveBank);
	if (FAILED(hr))
	{
		ueLogE("CreateInMemoryWaveBank failed, hr = 0x%x", hr);
		goto Failure;
	}

	hr = s_data->m_engine->CreateSoundBank(bank->m_soundBankFile, (DWORD) soundBankSize, 0, 0, &bank->m_soundBank);
	if (FAILED(hr))
	{
		ueLogE("CreateSoundBank failed, hr = 0x%x", hr);
		goto Failure;
	}

	// Initialize sound ids

	hr = bank->m_soundBank->GetNumCues(&numSounds);
	if (FAILED(hr))
	{
		ueLogE("GetNumCues failed, hr = 0x%x", hr);
		goto Failure;
	}
	bank->m_numSounds = numSounds;
	const u32 soundIdsMemorySize = sizeof(auSoundId) * (u32) numSounds;
	bank->m_soundIds = (auSoundId*) s_data->m_freqAllocator->Alloc(soundIdsMemorySize);
	if (!bank->m_soundIds)
	{
		ueLogE("Failed to create sound bank (name = '%s'), reason: not enough memory for sound ids (needed = %u bytes)", name, soundIdsMemorySize);
		goto Failure;
	}
	for (u32 i = 0; i < (u32) numSounds; i++)
	{
		bank->m_soundIds[i].m_bank = bank;
		bank->m_soundIds[i].m_cueIndex = (XACTINDEX) i;
	}

	s_data->m_banks.PushFront(bank);
	return bank;

Failure:
	if (bank->m_waveBank)
		bank->m_waveBank->Destroy();
	if (bank->m_soundBank)
		bank->m_soundBank->Destroy();
	s_data->m_freqAllocator->Free(bank);
	return NULL;
}

void auSoundBank_Unload(auSoundBank* bank)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return;

	bank->m_waveBank->Destroy();
	bank->m_soundBank->Destroy();
	s_data->m_banks.Remove(bank);
	s_data->m_freqAllocator->Free(bank->m_soundIds);
	s_data->m_freqAllocator->Free(bank);
}

auSoundId* auSoundBank_GetSoundId(const char* name, auSoundBank* bank)
{
	UE_ASSERT(s_data);

	ueMutexLock lock(s_data->m_mutex);

	XACTINDEX cueIndex = XACTINDEX_INVALID;
	if (bank)
		cueIndex = bank->m_soundBank->GetCueIndex(name);
	else
		for (bank = s_data->m_banks.Front(); bank; bank = bank->Next())
			if ((cueIndex = bank->m_soundBank->GetCueIndex(name)) != XACTINDEX_INVALID)
				break;

	return cueIndex == XACTINDEX_INVALID ? NULL : &bank->m_soundIds[cueIndex];
}

void auLib_SetListenerDesc(const auListenerDesc& listener)
{
	UE_ASSERT(s_data);
	ueMemSet(&s_data->m_listener, 0, sizeof(s_data->m_listener));

	ueMemCpy(&s_data->m_listener.Position, &listener.m_position, sizeof(f32) * 3);
	ueMemCpy(&s_data->m_listener.Velocity, &listener.m_velocity, sizeof(f32) * 3);

	ueMat44 rotation;
	ueMat44_SetRotation(rotation, listener.m_orientation);

	ueVec3 frontVec, upVec;
	ueMat44_GetFrontVec(frontVec, rotation);
	ueMat44_GetUpVec(upVec, rotation);

	ueVec3_GetPtr(&s_data->m_listener.OrientFront.x, frontVec);
	ueVec3_GetPtr(&s_data->m_listener.OrientTop.x, upVec);
}

void auLib_Update(f32 dt)
{
	UE_ASSERT(s_data);

	UE_PROF_SCOPE("auLib_Update");

	if (!s_data->m_engine)
		return;

	// Remove finished sounds
	{
		ueMutexLock lock(s_data->m_mutex);

		ueList<auSound> soundsCopy;
		soundsCopy.CopyFrom(s_data->m_sounds);

		while (auSound* sound = soundsCopy.PopBack())
		{
			if (!sound->m_cue)
			{
				if (sound->m_destroyOnFinish)
				{
					if (sound->m_handlePtr)
						sound->m_handlePtr->_SetHandle(NULL);
					s_data->m_soundsPool.Free(sound);
					continue;
				}
			}
			else
			{
				DWORD cueState = 0;
				sound->m_cue->GetState(&cueState);

				if ((sound->m_toBeDestroyed || sound->m_destroyOnFinish) && (cueState & XACT_CUESTATE_STOPPED))
				{
					if (sound->m_handlePtr)
						sound->m_handlePtr->_SetHandle(NULL);
					sound->m_cue->Destroy();
					s_data->m_soundsPool.Free(sound);
					continue;
				}

				if (sound->m_is3D)
				{
					ueMemCpy(&s_data->m_emitter.Position, &sound->m_position, sizeof(f32) * 3);
					XACT3DCalculate(s_data->m_3DInstance, &s_data->m_listener, &s_data->m_emitter, &s_data->m_dspSettings);
					XACT3DApply(&s_data->m_dspSettings, sound->m_cue);
				}
			}

			s_data->m_sounds.PushFront(sound);
		}
	}

	// Update XACT / XAudio2

	s_data->m_engine->DoWork();
}

// Converts volume to decibels
f32 auLib_VolumeToDB(f32 volume)
{
	volume = ueClamp(volume, 0.0f, AU_MAX_VOLUME);
	if (volume >= 1.0f)
		return (AU_MAX_VOLUME - volume) / (AU_MAX_VOLUME - 1.0f) * 6.0f;
	return volume * 96.0f - 96.0f;
}

void auLib_SetVolume(const char* categoryName, f32 volume)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return;

	XACTCATEGORY category = s_data->m_engine->GetCategory(categoryName);
	if (category == XACTCATEGORY_INVALID)
		return;

	s_data->m_engine->SetVolume(category, auLib_VolumeToDB(volume));
}

void auLib_Stop(const char* categoryName, ueBool immediately)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return;

	XACTCATEGORY category = s_data->m_engine->GetCategory(categoryName);
	if (category == XACTCATEGORY_INVALID)
		return;

	s_data->m_engine->Stop(category, immediately ? XACT_FLAG_ENGINE_STOP_IMMEDIATE : 0);
}

// Sound

auSound* auSound_CreateAndPlay(auSoundId* soundId)
{
	auSoundParams soundParams;
	soundParams.m_id = soundId;
	return auSound_Create(&soundParams);
}

auSound* auSound_Create(auSoundParams* params)
{
	UE_ASSERT(s_data);
	UE_ASSERT(params->m_id);

	if (!s_data->m_engine)
		return NULL;

	ueMutexLock lock(s_data->m_mutex);
	auSoundId* id = params->m_id;

	// Allocate new slot for the sound

	auSound* sound = new(s_data->m_soundsPool) auSound();
	if (!sound)
		return NULL;
	s_data->m_sounds.PushBack(sound);

	sound->m_toBeDestroyed = UE_FALSE;
	sound->m_destroyOnFinish = params->m_destroyOnFinish;
	sound->m_handlePtr = params->m_handlePtr;
	if (sound->m_handlePtr)
		sound->m_handlePtr->_SetHandle(sound);

	// Precache the sound

	id->m_bank->m_soundBank->Prepare(id->m_cueIndex, 0, 0, &sound->m_cue);

	// Apply 3D settings if needed

	sound->m_is3D = params->m_position != NULL;
	if (sound->m_is3D)
	{
		sound->m_position = *params->m_position;

		ueMemCpy(&s_data->m_emitter.Position, params->m_position, sizeof(f32) * 3);
		XACT3DCalculate(s_data->m_3DInstance, &s_data->m_listener, &s_data->m_emitter, &s_data->m_dspSettings);
		XACT3DApply(&s_data->m_dspSettings, sound->m_cue);
	}

	// Set volume

	if (params->m_volume != 1.0f)
		auSound_SetVolume(sound, params->m_volume);

	// Start playing

	if (params->m_start)
		sound->m_cue->Play();

	return sound;
}

void auSound_Destroy(auSound* sound, ueBool immediately)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return;

	ueMutexLock lock(s_data->m_mutex);

	if (sound->m_handlePtr)
	{
		sound->m_handlePtr->_SetHandle(NULL);
		sound->m_handlePtr = NULL;
	}

	if (immediately)
	{
		if (sound->m_cue)
			sound->m_cue->Destroy();
		s_data->m_sounds.Remove(sound);
		s_data->m_soundsPool.Free(sound);
	}
	else if (sound->m_cue)
	{
		sound->m_toBeDestroyed = UE_TRUE;

		// Stop and wait until stopped, then destroy

		sound->m_cue->Stop(0);
	}
}

void auSound_Play(auSound* sound)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return;

	if (sound->m_cue)
		sound->m_cue->Play();
}

void auSound_Stop(auSound* sound)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return;

	if (sound->m_cue)
		sound->m_cue->Stop(0);
}

ueBool auSound_IsPlaying(auSound* sound)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return UE_FALSE;

	if (!sound->m_cue)
		return UE_FALSE;
	
	DWORD state = 0;
	sound->m_cue->GetState(&state);
	return state == XACT_CUESTATE_PLAYING;
}

void auSound_Pause(auSound* sound, ueBool pause)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return;

	if (sound->m_cue)
		sound->m_cue->Pause(pause ? 1 : 0);
}

void auSound_SetVolume(auSound* sound, f32 volume)
{
	UE_ASSERT(s_data);

	if (!s_data->m_engine)
		return;

	if (!sound->m_cue)
		return;

	XACTVARIABLEINDEX idx = sound->m_cue->GetVariableIndex("Volume");
	if (idx)
		sound->m_cue->SetVariable(idx, auLib_VolumeToDB(volume));
}

void auSound_SetPosition(auSound* sound, ueVec3 position)
{
	sound->m_position = position;
}

const char* auSound_GetName(auSound* sound)
{
	if (!auSound_IsPlaying(sound))
		return NULL;

	XACT_CUE_INSTANCE_PROPERTIES* props;
	sound->m_cue->GetProperties(&props);

	return props->cueProperties.friendlyName;
}

// auDynamicSound

auDynamicSound* auDynamicSound_Create(auDynamicSoundParams& params)
{
	if (!s_data->m_engine)
		return NULL;

	// Get total ring buffer size

	const ueSize ringBufferSizeTotal = ueRingBuffer::CalcMemReq(params.m_maxBufferChunks, params.m_ringBufferSize);

	// Allocate memory

	u8* memory = (u8*) params.m_allocator->Alloc(sizeof(auDynamicSound) + ringBufferSizeTotal);
	if (!memory)
		return NULL;

	// Create and init sound

	auDynamicSound* sound = new(memory) auDynamicSound();
	memory += sizeof(auDynamicSound);

	sound->m_allocator = params.m_allocator;

	sound->m_ringBuffer.InitMem(memory, ringBufferSizeTotal, params.m_maxBufferChunks, params.m_ringBufferSize);
	memory += ringBufferSizeTotal;
	sound->m_ringBuffer.EnableWrapping(UE_FALSE);

	sound->m_numSubmittedBuffers = 0;

	// Create XAudio2 source voice

	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = params.m_numChannels;
	waveFormat.nSamplesPerSec = params.m_samplesPerSec;
	waveFormat.nAvgBytesPerSec = params.m_avgBytesPerSec;
	waveFormat.nBlockAlign = params.m_blockAlign;
	waveFormat.wBitsPerSample = params.m_bitsPerSample;
	waveFormat.cbSize = 0;

	if (FAILED(s_data->m_xaudio2->CreateSourceVoice(&sound->m_srcVoice, &waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, sound)))
	{
		sound->m_allocator->Free(sound);
		return NULL;
	}
	if (FAILED(sound->m_srcVoice->Start(0)))
	{
		sound->m_srcVoice->DestroyVoice();
		sound->m_allocator->Free(sound);
		return NULL;
	}

	return sound;

}

void auDynamicSound_Destroy(auDynamicSound* sound)
{
	sound->m_srcVoice->DestroyVoice();
	sound->m_allocator->Free(sound);
}

void auDynamicSound_Update(auDynamicSound* sound)
{
	// Get voice state

	XAUDIO2_VOICE_STATE voiceState;
	sound->m_srcVoice->GetState(&voiceState);

	// Push awaiting buffers for processing

	ueMutexLock lock(sound->m_mutex);

	while (sound->m_ringBuffer.Count() > sound->m_numSubmittedBuffers && voiceState.BuffersQueued < XAUDIO2_MAX_QUEUED_BUFFERS)
	{
		u32 bufferSize;
		void* buffer;
		sound->m_ringBuffer.GetFrontSize(sound->m_numSubmittedBuffers, bufferSize);
		sound->m_ringBuffer.GetFrontData(sound->m_numSubmittedBuffers, &buffer);

		XAUDIO2_BUFFER xAudio2Buffer;
		memset(&xAudio2Buffer, 0, sizeof(XAUDIO2_BUFFER));
		xAudio2Buffer.pAudioData = (const BYTE*) buffer;
		xAudio2Buffer.AudioBytes = bufferSize;
		xAudio2Buffer.pContext = buffer;

		HRESULT hr = sound->m_srcVoice->SubmitSourceBuffer(&xAudio2Buffer);
		if (FAILED(hr))
			ueLogE("Failed to submit buffer for dynamic sound (%d bytes).", bufferSize);
		else
		{
			sound->m_numSubmittedBuffers++;
			voiceState.BuffersQueued++;
		}
	}
}

void auDynamicSound_FlushBuffers(auDynamicSound* sound)
{
	sound->m_srcVoice->Stop();
	sound->m_srcVoice->FlushSourceBuffers();
	sound->m_srcVoice->Start(0);
}

void auDynamicSound_SubmitBuffer(auDynamicSound* sound, const void* buffer, u32 bufferSize)
{
	auDynamicSound_Update(sound);
	{
		ueMutexLock lock(sound->m_mutex);
		UE_ASSERT_FUNC( sound->m_ringBuffer.PushBack(buffer, bufferSize, UE_TRUE) );
	}
	auDynamicSound_Update(sound);
}

u32 auDynamicSound_GetBufferedSize(auDynamicSound* sound)
{
	ueMutexLock lock(sound->m_mutex);
	return sound->m_ringBuffer.GetBufferedDataSize();
}

u32 auDynamicSound_GetMaxBytesThatCanSubmit(auDynamicSound* sound)
{
	ueMutexLock lock(sound->m_mutex);
	return sound->m_ringBuffer.GetMaxBytesThatCanBuffer();
}

void auDynamicSound::OnBufferEnd(void* pBufferContext)
{
	ueMutexLock lock(m_mutex);
	UE_ASSERT(m_numSubmittedBuffers > 0);
	m_numSubmittedBuffers--;
	m_ringBuffer.PopFront();
}

void auDynamicSound_SetVolume(auDynamicSound* sound, f32 volume)
{
	sound->m_srcVoice->SetVolume(auLib_VolumeToDB(volume));
}