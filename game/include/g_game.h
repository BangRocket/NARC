#ifndef G_GAME_H
#define G_GAME_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GS_MENU,
    GS_PLAYING,
    GS_PAUSED,
    GS_LOADING,
    GS_INTERMISSION,
    GS_FINALE
} gamestate_t;

typedef struct {
    int screenWidth;
    int screenHeight;
    bool fullscreen;
    bool vsync;
    int targetFPS;
    const char* windowTitle;
    const char* dataPath;
} game_config_t;

bool G_Init(const game_config_t* config);
void G_Shutdown(void);

void G_RunFrame(void);
void G_Draw(void);

void G_NewGame(int skill, int level);
void G_LoadGame(const char* filename);
void G_SaveGame(const char* filename);

void G_SetGameState(gamestate_t state);
gamestate_t G_GetGameState(void);

void G_PauseGame(void);
void G_ResumeGame(void);
bool G_IsPaused(void);

void G_ProcessInput(void);
void G_UpdateGame(float deltaTime);

void G_PlayerDied(void);
void G_LevelComplete(void);

void G_SetMouseSensitivity(float sensitivity);
float G_GetMouseSensitivity(void);

void G_SetSoundVolume(float volume);
float G_GetSoundVolume(void);

void G_SetMusicVolume(float volume);
float G_GetMusicVolume(void);

#ifdef __cplusplus
}
#endif

#endif