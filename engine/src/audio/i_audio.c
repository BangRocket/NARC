#include "../../include/i_audio.h"
#include "../../include/i_system.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct audio_sound_s {
    Sound sound;
    float baseVolume;
    float basePitch;
} audio_sound_impl_t;

typedef struct audio_music_s {
    Music music;
    float baseVolume;
    float basePitch;
} audio_music_impl_t;

static bool audioInitialized = false;
static float masterVolume = 1.0f;
static Vector3 listenerPosition = { 0, 0, 0 };
static Vector3 listenerForward = { 0, 0, -1 };
static Vector3 listenerUp = { 0, 1, 0 };

bool I_InitAudio(const audio_config_t* config) {
    if (audioInitialized) return true;
    
    InitAudioDevice();
    
    if (!IsAudioDeviceReady()) {
        I_Warning("Failed to initialize audio device");
        return false;
    }
    
    SetMasterVolume(masterVolume);
    
    I_Log("Audio initialized: %d Hz, %d channels", 
          config ? config->sampleRate : 44100,
          config ? config->channels : 2);
    
    audioInitialized = true;
    return true;
}

void I_ShutdownAudio(void) {
    if (!audioInitialized) return;
    
    CloseAudioDevice();
    audioInitialized = false;
    
    I_Log("Audio shutdown");
}

audio_sound_t I_LoadSound(const char* filename) {
    if (!audioInitialized || !filename) return NULL;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)I_Malloc(sizeof(audio_sound_impl_t));
    soundImpl->sound = LoadSound(filename);
    soundImpl->baseVolume = 1.0f;
    soundImpl->basePitch = 1.0f;
    
    if (!IsSoundReady(soundImpl->sound)) {
        I_Warning("Failed to load sound: %s", filename);
        I_Free(soundImpl);
        return NULL;
    }
    
    return (audio_sound_t)soundImpl;
}

void I_UnloadSound(audio_sound_t sound) {
    if (!sound) return;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    UnloadSound(soundImpl->sound);
    I_Free(soundImpl);
}

void I_PlaySound(audio_sound_t sound) {
    if (!sound || !audioInitialized) return;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    PlaySound(soundImpl->sound);
}

void I_PlaySoundMulti(audio_sound_t sound) {
    if (!sound || !audioInitialized) return;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    PlaySoundMulti(soundImpl->sound);
}

void I_StopSound(audio_sound_t sound) {
    if (!sound || !audioInitialized) return;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    StopSound(soundImpl->sound);
}

void I_PauseSound(audio_sound_t sound) {
    if (!sound || !audioInitialized) return;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    PauseSound(soundImpl->sound);
}

void I_ResumeSound(audio_sound_t sound) {
    if (!sound || !audioInitialized) return;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    ResumeSound(soundImpl->sound);
}

bool I_IsSoundPlaying(audio_sound_t sound) {
    if (!sound || !audioInitialized) return false;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    return IsSoundPlaying(soundImpl->sound);
}

void I_SetSoundVolume(audio_sound_t sound, float volume) {
    if (!sound || !audioInitialized) return;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    soundImpl->baseVolume = volume;
    SetSoundVolume(soundImpl->sound, volume * masterVolume);
}

void I_SetSoundPitch(audio_sound_t sound, float pitch) {
    if (!sound || !audioInitialized) return;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    soundImpl->basePitch = pitch;
    SetSoundPitch(soundImpl->sound, pitch);
}

void I_SetSoundPan(audio_sound_t sound, float pan) {
    if (!sound || !audioInitialized) return;
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    SetSoundPan(soundImpl->sound, pan);
}

audio_music_t I_LoadMusicStream(const char* filename) {
    if (!audioInitialized || !filename) return NULL;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)I_Malloc(sizeof(audio_music_impl_t));
    musicImpl->music = LoadMusicStream(filename);
    musicImpl->baseVolume = 1.0f;
    musicImpl->basePitch = 1.0f;
    
    if (!IsMusicReady(musicImpl->music)) {
        I_Warning("Failed to load music: %s", filename);
        I_Free(musicImpl);
        return NULL;
    }
    
    return (audio_music_t)musicImpl;
}

void I_UnloadMusicStream(audio_music_t music) {
    if (!music) return;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    UnloadMusicStream(musicImpl->music);
    I_Free(musicImpl);
}

void I_PlayMusicStream(audio_music_t music) {
    if (!music || !audioInitialized) return;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    PlayMusicStream(musicImpl->music);
}

void I_UpdateMusicStream(audio_music_t music) {
    if (!music || !audioInitialized) return;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    UpdateMusicStream(musicImpl->music);
}

void I_StopMusicStream(audio_music_t music) {
    if (!music || !audioInitialized) return;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    StopMusicStream(musicImpl->music);
}

void I_PauseMusicStream(audio_music_t music) {
    if (!music || !audioInitialized) return;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    PauseMusicStream(musicImpl->music);
}

void I_ResumeMusicStream(audio_music_t music) {
    if (!music || !audioInitialized) return;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    ResumeMusicStream(musicImpl->music);
}

void I_SeekMusicStream(audio_music_t music, float position) {
    if (!music || !audioInitialized) return;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    SeekMusicStream(musicImpl->music, position);
}

bool I_IsMusicStreamPlaying(audio_music_t music) {
    if (!music || !audioInitialized) return false;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    return IsMusicStreamPlaying(musicImpl->music);
}

void I_SetMusicVolume(audio_music_t music, float volume) {
    if (!music || !audioInitialized) return;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    musicImpl->baseVolume = volume;
    SetMusicVolume(musicImpl->music, volume * masterVolume);
}

void I_SetMusicPitch(audio_music_t music, float pitch) {
    if (!music || !audioInitialized) return;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    musicImpl->basePitch = pitch;
    SetMusicPitch(musicImpl->music, pitch);
}

void I_SetMusicPan(audio_music_t music, float pan) {
    if (!music || !audioInitialized) return;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    SetMusicPan(musicImpl->music, pan);
}

float I_GetMusicTimeLength(audio_music_t music) {
    if (!music || !audioInitialized) return 0.0f;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    return GetMusicTimeLength(musicImpl->music);
}

float I_GetMusicTimePlayed(audio_music_t music) {
    if (!music || !audioInitialized) return 0.0f;
    
    audio_music_impl_t* musicImpl = (audio_music_impl_t*)music;
    return GetMusicTimePlayed(musicImpl->music);
}

void I_SetMasterVolume(float volume) {
    masterVolume = volume;
    if (audioInitialized) {
        SetMasterVolume(volume);
    }
}

void I_PlaySound3D(audio_sound_t sound, float x, float y, float z) {
    if (!sound || !audioInitialized) return;
    
    float dx = x - listenerPosition.x;
    float dy = y - listenerPosition.y;
    float dz = z - listenerPosition.z;
    float distance = sqrtf(dx*dx + dy*dy + dz*dz);
    
    float maxDistance = 50.0f;
    float volume = 1.0f - (distance / maxDistance);
    if (volume < 0.0f) volume = 0.0f;
    
    float pan = 0.0f;
    if (distance > 0.0f) {
        Vector3 toSound = { dx / distance, dy / distance, dz / distance };
        Vector3 right = {
            listenerUp.y * listenerForward.z - listenerUp.z * listenerForward.y,
            listenerUp.z * listenerForward.x - listenerUp.x * listenerForward.z,
            listenerUp.x * listenerForward.y - listenerUp.y * listenerForward.x
        };
        pan = toSound.x * right.x + toSound.y * right.y + toSound.z * right.z;
    }
    
    audio_sound_impl_t* soundImpl = (audio_sound_impl_t*)sound;
    SetSoundVolume(soundImpl->sound, volume * soundImpl->baseVolume * masterVolume);
    SetSoundPan(soundImpl->sound, pan);
    PlaySound(soundImpl->sound);
}

void I_SetListenerPosition(float x, float y, float z) {
    listenerPosition.x = x;
    listenerPosition.y = y;
    listenerPosition.z = z;
}

void I_SetListenerOrientation(float forwardX, float forwardY, float forwardZ,
                              float upX, float upY, float upZ) {
    listenerForward.x = forwardX;
    listenerForward.y = forwardY;
    listenerForward.z = forwardZ;
    
    listenerUp.x = upX;
    listenerUp.y = upY;
    listenerUp.z = upZ;
}