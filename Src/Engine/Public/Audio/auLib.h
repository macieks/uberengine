#ifndef AU_AUDIO_H
#define AU_AUDIO_H

/**
 *	@defgroup au Audio
 *	@brief Audio Library with support for all basic uedio functionality.
 */

#include "Base/ueMath.h"

//! Max. uedio volume
#define AU_MAX_VOLUME 8.0f

/**
 *	@addtogroup au
 *	@{
 */

class auSoundHandle;

/**
 *	@struct auSound
 *	@brief Playable sound instance
 */
struct auSound;

/**
 *	@struct auSoundBank
 *	@brief Sound group / container loaded as a single chunk of memory from file
 */
struct auSoundBank;

/**
 *	@struct auSoundId
 *	@brief Sound resource id (look up once, then use when creating instances; speeds up sound creation)
 */
struct auSoundId;

/**
 *	@struct auDynamicSound
 *	@brief Dynamic sound instance
 */
struct auDynamicSound;

//! 3D sound listener description
struct auListenerDesc
{
	ueQuat m_orientation;	//!< Listener orientation
	ueVec3 m_position;		//!< Listener position
	ueVec3 m_velocity;		//!< Listener velocity

	auListenerDesc()
	{
		m_orientation.SetIdentity();
		m_position.Zeroe();
		m_velocity.Zeroe();
	}
};

//! Sound creation parameters
struct auSoundParams
{
	auSoundId* m_id;			//!< Sound id
	auSoundHandle* m_handlePtr;	//!< Optional pointer to the handle to be associated with this sound; when sound gets destroyed the handle will be NULL'ed
	const ueVec3* m_position;	//!< 3D sound position
	ueBool m_destroyOnFinish;	//!< Indicates whether to destroy sound when done
	ueBool m_start;				//!< Indicates whether to start playing the sound immediately
	f32 m_volume;				//!< Initial volume
	u32 m_loopCount;			//!< Loop count; 0 - as specified in cue

	auSoundParams() :
		m_id(NULL),
		m_handlePtr(NULL),
		m_position(NULL),
		m_destroyOnFinish(UE_TRUE),
		m_start(UE_TRUE),
		m_volume(1.0f),
		m_loopCount(0)
	{}
};

//! Dynamic sound creation parameters
struct auDynamicSoundParams
{
	ueAllocator* m_allocator;//!< Allocator used for sound data queue and all other sound memory

	u32 m_maxBufferChunks;	//!< Max. number of chunks that can be queued
	u32 m_ringBufferSize;	//!< Size of the data queue (ring buffer)

	// PCM sound description

	u32 m_numChannels;       //!< Number of channels (i.e. mono, stereo...)
	u32 m_samplesPerSec;     //!< Sample rate
	u32 m_avgBytesPerSec;    //!< For buffer estimation
	u32 m_blockAlign;        //!< Block size of data
	u32 m_bitsPerSample;     //!< Number of bits per sample of mono data

	auDynamicSoundParams() :
		m_allocator(NULL),
		m_maxBufferChunks(16),
		m_ringBufferSize(1 << 16),
		m_numChannels(0),
		m_samplesPerSec(0),
		m_avgBytesPerSec(0),
		m_blockAlign(0),
		m_bitsPerSample(0)
	{}
};

//! Audio library startup parameters
struct auLibStartupParams
{
	ueAllocator* m_stackAllocator;		//!< Prefarably stack allocator
	ueAllocator* m_freqAllocator;		//!< Random access allocator
	const char* m_settingsFileName;		//!< Global settings file name (loaded at startup)
	u32 m_maxSounds;					//!< Max. number of sounds

	auLibStartupParams() :
		m_stackAllocator(NULL),
		m_freqAllocator(NULL),
		m_settingsFileName(NULL),
		m_maxSounds(128)
	{}
};

// Audio library
// --------------------------------

//! Starts up uedio library
void		auLib_Startup(auLibStartupParams* params);
//! Shuts down uedio library
void		auLib_Shutdown();

//! Tells whether library is in a valid state; it can be used even when it's not, but it won't play sounds
ueBool		auLib_IsValid();

//! Updates uedio library
void		auLib_Update(f32 dt);
//! Pauses selected sound category
void		auLib_Pause(ueBool pause, const char* categoryName = NULL);

//! Sets sound listener parameters
void		auLib_SetListenerDesc(const auListenerDesc& listener);
//! Sets volume of the sound category; 0 - no sound; 1 - default; AU_MAX_VOLUME - max value
void		auLib_SetVolume(const char* categoryName, f32 volume);
//! Stops all sounds in category
void		auLib_Stop(const char* categoryName, ueBool immediately);

// Sound bank
// --------------------------------

//! Loads sound bank
auSoundBank*auSoundBank_Load(const char* name);
//! Unloads sound bank
void		auSoundBank_Unload(auSoundBank* bank);
//! Unloads all sound banks
void		auSoundBank_UnloadAll();
//! Gets sound id from given sound bank; if bank isn't specified all banks are searched
auSoundId*	auSoundBank_GetSoundId(const char* name, auSoundBank* bank = NULL);

// Sound
// --------------------------------

//! Creates the sound
auSound*	auSound_Create(auSoundParams* params);
//! Creates and plays the sound
auSound*	auSound_CreateAndPlay(auSoundId* soundId);
//! Destroys the sound; if immediately is not set, the sound will fade out according to its settings
void		auSound_Destroy(auSound* sound, ueBool immediately = UE_FALSE);

//! Plays the sound
void		auSound_Play(auSound* sound);
//! Stops the sound
void		auSound_Stop(auSound* sound);
//! Gets whether sound is playing
ueBool		auSound_IsPlaying(auSound* sound);
//! Pauses the sound
void		auSound_Pause(auSound* sound, ueBool pause);
//! Sets sound volume; 0 - no sound; 1 - default; AU_MAX_VOLUME - max value
void		auSound_SetVolume(auSound* sound, f32 volume);

//! Sets position of the 3D sound
void		auSound_SetPosition(auSound* sound, const ueVec3& position);

//! Gets sound name (returns NULL if sound isn't playing)
const char* auSound_GetName(auSound* sound);

//! Sound handle helper class
class auSoundHandle : public ueSimpleHandle<auSound>
{
public:
	//! Destroys the sound
	UE_INLINE ~auSoundHandle() { Destroy(UE_TRUE); }
	//! Gets whether the sound is playing
	UE_INLINE ueBool IsPlaying() const
	{
		return m_object && auSound_IsPlaying(m_object);
	}
	//! Creates and starts playing the sound
	UE_INLINE void CreateAndPlay(const char* name, f32 volume = 1.0f, u32 loopCount = 1)
	{
		auSoundParams params;
		params.m_handlePtr = this;
		params.m_id = auSoundBank_GetSoundId(name);
		params.m_start = UE_TRUE;
		params.m_volume = volume;
		params.m_loopCount = loopCount;
		auSound_Create(&params);
	}
	//! Destroys the sound
	UE_INLINE void Destroy(ueBool immediately = UE_FALSE)
	{
		if (m_object)
			auSound_Destroy(m_object, immediately);
	}
	//! Gets sound name; returns NULL if no sound is created
	UE_INLINE const char* GetName() const
	{
		return m_object ? auSound_GetName(m_object) : NULL;
	}
	//! Sets sound volume; the function has no effect if no sound is created
	UE_INLINE void SetVolume(f32 volume)
	{
		if (m_object)
			auSound_SetVolume(m_object, volume);
	}
};

// Dynamic sound
// --------------------------------

//! Creates dynamic sound
auDynamicSound* auDynamicSound_Create(auDynamicSoundParams& params);
//! Destroys dynamic sound
void			auDynamicSound_Destroy(auDynamicSound* sound);

//! Updates dynamic sound
void			auDynamicSound_Update(auDynamicSound* sound);

//! Submits new uedio buffer; if there's not enough room in a queue, the oldest entry (or entries) are first removed
void			auDynamicSound_SubmitBuffer(auDynamicSound* sound, const void* buffer, u32 bufferSize);
//! Removes all queued buffers from the queue
void			auDynamicSound_FlushBuffers(auDynamicSound* sound);
//! Gets total buffered data size in a queue
u32				auDynamicSound_GetBufferedSize(auDynamicSound* sound);
//! Gets max. number of bytes that it's possible to queue for that sound
u32				auDynamicSound_GetMaxBytesThatCanSubmit(auDynamicSound* sound);

//! Sets volume for the sound; 0 - no sound; 1 - default; AU_MAX_VOLUME - max value
void			auDynamicSound_SetVolume(auDynamicSound* sound, f32 volume);

//	@}

#endif // AU_AUDIO_H
