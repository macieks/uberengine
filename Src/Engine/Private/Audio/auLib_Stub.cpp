#include "Audio/auLib.h"

void		auLib_Startup(auLibStartupParams* params) {}
void		auLib_Shutdown() {}
ueBool		auLib_IsValid() { return UE_FALSE; }
void		auLib_Update(f32 dt) {}
void		auLib_Pause(ueBool pause, const char* categoryName) {}
void		auLib_SetListenerDesc(const auListenerDesc& listener) {}
void		auLib_SetVolume(const char* categoryName, f32 volume) {}
void		auLib_Stop(const char* categoryName, ueBool immediately) {}

auSoundBank*auSoundBank_Load(const char* name) { return (auSoundBank*) 0x1; }
void		auSoundBank_Unload(auSoundBank* bank) {}
void		auSoundBank_UnloadAll() {}
auSoundId*	auSoundBank_GetSoundId(const char* name, auSoundBank* bank) { return (auSoundId*) 0x1; }

auSound*	auSound_Create(auSoundParams* params) { return (auSound*) 0x1; }
auSound*	auSound_CreateAndPlay(auSoundId* soundId) { return (auSound*) 0x1; }
void		auSound_Destroy(auSound* sound, ueBool immediately) {}
void		auSound_Play(auSound* sound) {}
void		auSound_Stop(auSound* sound) {}
ueBool		auSound_IsPlaying(auSound* sound) { return UE_FALSE; }
void		auSound_Pause(auSound* sound, ueBool pause) {}
void		auSound_SetVolume(auSound* sound, f32 volume) {}
void		auSound_SetPosition(auSound* sound, const ueVec3& position) {}
const char* auSound_GetName(auSound* sound) { return NULL; }

auDynamicSound* auDynamicSound_Create(auDynamicSoundParams& params) { return NULL; }
void			auDynamicSound_Destroy(auDynamicSound* sound) {}
void			auDynamicSound_Update(auDynamicSound* sound) {}
void			auDynamicSound_SubmitBuffer(auDynamicSound* sound, const void* buffer, u32 bufferSize) {}
void			auDynamicSound_FlushBuffers(auDynamicSound* sound) {}
u32				auDynamicSound_GetBufferedSize(auDynamicSound* sound) { return 0; }
u32				auDynamicSound_GetMaxBytesThatCanSubmit(auDynamicSound* sound) { return 0; }
void			auDynamicSound_SetVolume(auDynamicSound* sound, f32 volume) {}
