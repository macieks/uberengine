#include "Audio/auLib.h"
#include "Containers/ueList.h"
#include "IO/ioXml.h"
#include "IO/ioFile.h"
#include "Utils/utRand.h"
#include "stb_vorbis.h"

#define AU_CONFIG "marmalade-raw"

// Audio library

struct auSound :  public ueList<auSound>::Node
{
	auSoundId* m_id;
	s32 m_channelId;
	f32 m_volume;
	ueBool m_destroyOnFinish;
	auSoundHandle* m_handlePtr;
	u32 m_loopCount;
};

struct auSample
{
	f32 m_probability; //!< Value within 0..1 indicating how frequently to choose this sample (from all the samples in auSoundId)
	u32 m_size;
	void* m_data;
};

struct auSoundId : public ueList<auSoundId>::Node
{
	char* m_name;
	char* m_categoryName;
	auSoundBank* m_bank;
	ueBool m_loop;
	ueBool m_isMP3; // Otherwise raw uncompressed data
	u32 m_numSamples;
	auSample* m_samples;
};

struct auSoundBank : public ueList<auSoundBank>::Node
{
	char* m_name;
	u32 m_numSounds;
	ueList<auSoundId> m_sounds;
	u32 m_usedMemorySize;

	auSoundBank() : m_usedMemorySize(0) {}
};

struct auLibData
{
	ueAllocator* m_allocator;
	ueList<auSoundBank> m_banks;
	ueList<auSound> m_sounds;

	auSound* m_currentMP3;
};

static auLibData* s_data = NULL;

int32 auSound_AudioStopCallback(void* systemData, void* userData);

void auLib_Startup(auLibStartupParams* params)
{
	ueAssert(!s_data);
	s_data = new(params->m_freqAllocator) auLibData();
	ueAssert(s_data);
	s_data->m_allocator = params->m_freqAllocator;

	s_data->m_currentMP3 = NULL;

	s3eSoundSetInt(S3E_SOUND_DEFAULT_FREQ, 22050);
	s3eAudioRegister(S3E_AUDIO_STOP, auSound_AudioStopCallback, NULL);

	const ueBool isMP3Supported = s3eAudioIsCodecSupported(S3E_AUDIO_CODEC_MP3) ? UE_TRUE : UE_FALSE;
}

void auLib_Shutdown()
{
	ueAssert(s_data);
	s3eAudioUnRegister(S3E_AUDIO_STOP, auSound_AudioStopCallback);
	ueAssert(s_data->m_banks.Length() == 0);
	ueAssert(s_data->m_sounds.Length() == 0);
	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

ueBool auLib_IsValid()
{
	return UE_TRUE;
}

void auLib_Update(f32 dt)
{
	auSound* sound = s_data->m_sounds.Front();
	while (sound)
	{
		auSound* next = sound->Next();

		if (((sound->m_id->m_isMP3 && s_data->m_currentMP3 != sound) || (!sound->m_id->m_isMP3 && sound->m_channelId == -1)) &&
			sound->m_destroyOnFinish)
			auSound_Destroy(sound, UE_TRUE);

		sound = next;
	}
}

s32 auLib_ToMarmaladeVolume(f32 volume)
{
	static const f32 defaultMarmaladeVolume = s3eSoundGetInt(S3E_SOUND_VOLUME_DEFAULT) / 256.0f;
	static const f32 maxMarmaladeVolume = S3E_SOUND_MAX_VOLUME / 256.0f;

	volume = ueClamp(volume, 0.0f, AU_MAX_VOLUME);

	if (volume >= 1.0f)
		return (s32) (((volume - 1.0f) / (AU_MAX_VOLUME - 1.0f) * (maxMarmaladeVolume - defaultMarmaladeVolume) + defaultMarmaladeVolume) * 256.0f);
	return (s32) (volume * defaultMarmaladeVolume * 256.0f);
}

void auLib_SetVolume(const char* categoryName, f32 volume)
{
	const s32 marmaladeVolume = auLib_ToMarmaladeVolume(volume);
	auSound* sound = s_data->m_sounds.Front();
	while (sound)
	{
		if (sound->m_channelId != -1 && !ueStrCmp(categoryName, sound->m_id->m_categoryName))
			s3eSoundChannelSetInt(sound->m_channelId, S3E_CHANNEL_VOLUME, marmaladeVolume);
		sound = sound->Next();
	}
}

void auLib_Stop(const char* categoryName, ueBool immediately)
{
	auSound* sound = s_data->m_sounds.Front();
	while (sound)
	{
		auSound* next = sound->Next();
		if (!ueStrCmp(categoryName, sound->m_id->m_categoryName))
			auSound_Destroy(sound);
		sound = next;
	}
}

void auLib_Pause(ueBool pause, const char* categoryName)
{
	auSound* sound = s_data->m_sounds.Front();
	while (sound)
	{
		if (sound->m_channelId != -1 && !ueStrCmp(categoryName, sound->m_id->m_categoryName))
		{
			if (pause)
				s3eSoundChannelPause(sound->m_channelId);
			else
				s3eSoundChannelResume(sound->m_channelId);
		}
		sound = sound->Next();
	}
}

// Sound bank

ueBool auSoundBank_LoadSamples(auSoundBank* bank, auSample* samples, const char* samplePathPrefix, ioXmlNode* node)
{
	ueBool isMP3 = UE_FALSE;

	u32 index = 0;
	for (ioXmlNode* sampleNode = ioXmlNode_GetFirstNode(node, "wave"); sampleNode; sampleNode = ioXmlNode_GetNext(sampleNode))
	{
		auSample& sample = samples[index++];

		// Get sample file name

		const char* name = ioXmlNode_GetAttrValue(sampleNode, "name");
		ueAssert(name);

		// Get sample file path

		ioPath samplePath;
		ueStrFormatS(samplePath, "%s/%s", samplePathPrefix, name);

		// Load file

		void* data = NULL;
		ueSize size = 0;
		ueAssertFunc( ioFile_Load(samplePath, data, size, 0, s_data->m_allocator) );

		// If this is ogg, load it and decode first

		if (ueStrStr(name, ".ogg", UE_FALSE))
		{
			// Decode

			int oggChannels = 0;
			short* oggData = NULL;
			const int result = stb_vorbis_decode_memory((unsigned char*) data, size, &oggChannels, (short**) &oggData);
			ueAssert(result > 0);
			sample.m_size = result * oggChannels * sizeof(short);

			// Unload file content

			s_data->m_allocator->Free(data);

			// Copy into perfect-size buffer

			sample.m_data = ueMemDup(oggData, sample.m_size, s_data->m_allocator);
			ueAssert(sample.m_data);

			// Release ogg buffer
			
			stb_vorbis_free(oggData);
		}
		else
		{
			sample.m_size = size;
			sample.m_data = data;

			if (ueStrStr(name, ".mp3", UE_FALSE))
				isMP3 = UE_TRUE;
		}

		bank->m_usedMemorySize += sample.m_size;
	}

	return isMP3;
}

auSoundBank* auSoundBank_Load(const char* name)
{
	// Open sound bank Xml

	ioPath path;
	ueApp_GetAssetPath(path, name, "sound_bank.xml", NULL, ueAssetPath_NonlocalizedOnly);
	ioXmlDoc* doc = ioXmlDoc_Load(s_data->m_allocator, path);
	ueAssert(doc);
	ioXmlDocScopedDestructor docDestructor(doc);

	// Get root Xml node

	ioXmlNode* soundBankNode = ioXmlDoc_GetFirstNode(doc, "soundBank");
	ueAssert(soundBankNode);

	// Create sound bank

	const u32 nameLength = ueStrLen(name);
	const u32 bankMemorySize = sizeof(auSoundBank) + nameLength + 1;
	u8* bankMemory = (u8*) s_data->m_allocator->Alloc(bankMemorySize);
	ueAssert(bankMemory);

	auSoundBank* bank = new(bankMemory) auSoundBank();
	bank->m_usedMemorySize += bankMemorySize;
	bankMemory += sizeof(auSoundBank);

	bank->m_name = (char*) bankMemory;
	ueMemCpy(bank->m_name, name, nameLength + 1);

	bank->m_numSounds = 0;

	s_data->m_banks.PushBack(bank);

	// Get wave path prefix (relative to sound bank)

	ioPath wavePathPrefix;
	ueStrCpyS(wavePathPrefix, name);
	*ueStrLast(wavePathPrefix, '/') = 0;

	// Load sounds

	for (ioXmlNode* soundNode = ioXmlNode_GetFirstNode(soundBankNode, "sound"); soundNode; soundNode = ioXmlNode_GetNext(soundNode, "sound"))
	{
		const char* soundName = ioXmlNode_GetAttrValue(soundNode, "name");
		ueAssert(soundName);
		const u32 soundNameLength = ueStrLen(soundName);

		const char* categoryName = ioXmlNode_GetAttrValue(soundNode, "category");
		if (!categoryName)
			categoryName = "Default";
		const u32 categoryNameLength = ueStrLen(categoryName);

		const u32 numSamples = ioXmlNode_CalcNumNodes(soundNode, "wave");

		// Create sound entry in sound bank

		const u32 soundMemorySize = sizeof(auSoundId) + soundNameLength + 1 + categoryNameLength + 1 + sizeof(auSample) * numSamples;
		u8* soundMemory = (u8*) s_data->m_allocator->Alloc(soundMemorySize);
		ueAssert(soundMemory);
		bank->m_usedMemorySize += soundMemorySize;

		auSoundId* soundId = new(soundMemory) auSoundId();
		soundMemory += sizeof(auSoundId);

		soundId->m_bank = bank;
		soundId->m_numSamples = numSamples;
		soundId->m_samples = (auSample*) soundMemory;
		soundMemory += sizeof(auSample) * numSamples;

		soundId->m_name = (char*) soundMemory;
		ueMemCpy(soundId->m_name, soundName, soundNameLength + 1);
		soundMemory += soundNameLength + 1;

		soundId->m_categoryName = (char*) soundMemory;
		ueMemCpy(soundId->m_categoryName, categoryName, categoryNameLength + 1);

		if (!ioXmlNode_GetAttrValueBool(soundNode, "loop", soundId->m_loop))
			soundId->m_loop = UE_FALSE;

		// Load all samples for this sound

		soundId->m_isMP3 = auSoundBank_LoadSamples(bank, soundId->m_samples, wavePathPrefix, soundNode);

		// Add to the list of sounds

		bank->m_sounds.PushBack(soundId);
		bank->m_numSounds++;
	}

	return bank;
}

void auSoundBank_Unload(auSoundBank* bank)
{
	// Stop all sounds using this bank first

	auSound* sound = s_data->m_sounds.Front();
	while (sound)
	{
		auSound* next = sound->Next();
		if (sound->m_id->m_bank == bank)
			auSound_Destroy(sound, UE_TRUE);
		sound = next;
	}

	// Destroy bank

	s_data->m_banks.Remove(bank);
	while (auSoundId* soundId = bank->m_sounds.PopFront())
	{
		for (u32 i = 0; i < soundId->m_numSamples; i++)
			s_data->m_allocator->Free(soundId->m_samples[i].m_data);
		s_data->m_allocator->Free(soundId);
	}
	s_data->m_allocator->Free(bank);
}

void auSoundBank_UnloadAll()
{
	while (auSoundBank* bank = s_data->m_banks.Front())
		auSoundBank_Unload(bank);
}

auSoundId* auSoundBank_GetSoundId(const char* name, auSoundBank* bank)
{
	if (bank)
	{
		auSoundId* id = bank->m_sounds.Front();
		while (id)
			if (!ueStrCmp(name, id->m_name))
				return id;
			else
				id = id->Next();
	}
	else
	{
		bank = s_data->m_banks.Front();
		while (bank)
		{
			auSoundId* id = auSoundBank_GetSoundId(name, bank);
			if (id)
				return id;
			bank = bank->Next();
		}
	}
	return NULL;
}

// Sound

int32 auLib_SoundStopCallback(void* systemData, void* userData)
{
	auSound* sound = (auSound*) userData;
	sound->m_channelId = -1; // Will be destroyed during update (we're now in a different thread possibly)
	return 0;
}

int32 auSound_AudioStopCallback(void* systemData, void* userData)
{
	s_data->m_currentMP3 = NULL;
	return 0;
}

auSound* auSound_Create(auSoundParams* params)
{
	// Create the sound

	auSound* sound = new(s_data->m_allocator) auSound();
	if (!sound)
		return NULL;

	sound->m_channelId = -1;
	sound->m_id = params->m_id;
	sound->m_destroyOnFinish = params->m_destroyOnFinish;
	sound->m_volume = params->m_volume;
	sound->m_loopCount = params->m_loopCount;
	sound->m_handlePtr = params->m_handlePtr;
	if (sound->m_handlePtr)
		sound->m_handlePtr->_SetHandle(sound);

	s_data->m_sounds.PushBack(sound);

	// Optionally start playing the sound

	if (params->m_start)
		auSound_Play(sound);

	return sound;
}

auSample* auLib_PickSample(auSoundId* sound)
{
	return &sound->m_samples[(sound->m_numSamples == 1) ? 0 : utRandG_U32(sound->m_numSamples - 1)];
}

void auSound_Play(auSound* sound)
{
	uint32 loopValue = sound->m_id->m_loop ? 0 : 1;
	if (sound->m_loopCount > 0)
		loopValue = sound->m_loopCount;

	if (sound->m_id->m_isMP3)
	{
		ueAssert(!s_data->m_currentMP3);
		s_data->m_currentMP3 = sound;
		auSound_SetVolume(sound, sound->m_volume);
		auSample* sample = auLib_PickSample(sound->m_id);
		const s3eResult result = s3eAudioPlayFromBuffer(sample->m_data, sample->m_size, loopValue);
		return;
	}

	const s32 channelId = s3eSoundGetFreeChannel();
	if (channelId == -1)
		return;

	const s3eResult result = s3eSoundChannelRegister(channelId, S3E_CHANNEL_STOP_AUDIO, auLib_SoundStopCallback, sound);
	ueAssert(result == S3E_RESULT_SUCCESS);

	sound->m_channelId = channelId;
	auSound_SetVolume(sound, sound->m_volume);

	auSample* sample = auLib_PickSample(sound->m_id);
	s3eSoundChannelPlay(channelId, (int16*) sample->m_data, sample->m_size / 2, loopValue, 0);
}

const char* auSound_GetName(auSound* sound)
{
	return sound->m_id->m_name;
}

void auSound_SetVolume(auSound* sound, f32 volume)
{
	const s32 marmaladeVolume = auLib_ToMarmaladeVolume(volume);

	if (sound->m_id->m_isMP3)
	{
		if (s_data->m_currentMP3 == sound)
			s3eAudioSetInt(S3E_AUDIO_VOLUME, marmaladeVolume);
		return;
	}

	if (sound->m_channelId != -1)
		s3eSoundChannelSetInt(sound->m_channelId, S3E_CHANNEL_VOLUME, marmaladeVolume);
}

auSound* auSound_CreateAndPlay(auSoundId* soundId)
{
	auSoundParams params;
	params.m_id = soundId;
	params.m_destroyOnFinish = UE_TRUE;
	params.m_start = UE_TRUE;

	return auSound_Create(&params);
}

ueBool auSound_IsPlaying(auSound* sound)
{
	if (sound->m_id->m_isMP3)
		return s_data->m_currentMP3 == sound;
	return sound->m_channelId != -1;
}

void auSound_Destroy(auSound* sound, ueBool immediately)
{
	if (sound->m_handlePtr)
	{
		sound->m_handlePtr->_SetHandle(NULL);
		sound->m_handlePtr = NULL;
	}

	// TODO: Allow for non-immediate sound destruction

	if (sound->m_id->m_isMP3)
	{
		if (s_data->m_currentMP3 == sound)
			s3eAudioStop();
	}
	else if (sound->m_channelId != -1)
	{
		const s3eResult result = s3eSoundChannelUnRegister(sound->m_channelId, S3E_CHANNEL_STOP_AUDIO);
		ueAssert(result == S3E_RESULT_SUCCESS);

		s3eSoundChannelStop(sound->m_channelId);
		sound->m_channelId = -1;
	}

	s_data->m_sounds.Remove(sound);
	s_data->m_allocator->Free(sound);
}