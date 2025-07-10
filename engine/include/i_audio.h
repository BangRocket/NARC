#ifndef I_AUDIO_H
#define I_AUDIO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audio_sound_s* audio_sound_t;
typedef struct audio_music_s* audio_music_t;

typedef struct {
    int sampleRate;
    int channels;
    bool spatialAudio;
} audio_config_t;

bool I_InitAudio(const audio_config_t* config);
void I_ShutdownAudio(void);

audio_sound_t I_LoadSound(const char* filename);
void I_UnloadSound(audio_sound_t sound);
void I_PlaySound(audio_sound_t sound);
void I_PlaySoundMulti(audio_sound_t sound);
void I_StopSound(audio_sound_t sound);
void I_PauseSound(audio_sound_t sound);
void I_ResumeSound(audio_sound_t sound);
bool I_IsSoundPlaying(audio_sound_t sound);
void I_SetSoundVolume(audio_sound_t sound, float volume);
void I_SetSoundPitch(audio_sound_t sound, float pitch);
void I_SetSoundPan(audio_sound_t sound, float pan);

audio_music_t I_LoadMusicStream(const char* filename);
void I_UnloadMusicStream(audio_music_t music);
void I_PlayMusicStream(audio_music_t music);
void I_UpdateMusicStream(audio_music_t music);
void I_StopMusicStream(audio_music_t music);
void I_PauseMusicStream(audio_music_t music);
void I_ResumeMusicStream(audio_music_t music);
void I_SeekMusicStream(audio_music_t music, float position);
bool I_IsMusicStreamPlaying(audio_music_t music);
void I_SetMusicVolume(audio_music_t music, float volume);
void I_SetMusicPitch(audio_music_t music, float pitch);
void I_SetMusicPan(audio_music_t music, float pan);
float I_GetMusicTimeLength(audio_music_t music);
float I_GetMusicTimePlayed(audio_music_t music);

void I_SetMasterVolume(float volume);

void I_PlaySound3D(audio_sound_t sound, float x, float y, float z);
void I_SetListenerPosition(float x, float y, float z);
void I_SetListenerOrientation(float forwardX, float forwardY, float forwardZ, 
                               float upX, float upY, float upZ);

#ifdef __cplusplus
}
#endif

#endif