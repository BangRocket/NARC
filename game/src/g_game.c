#include "../include/g_game.h"
#include "../../engine/include/i_system.h"
#include "../../engine/include/i_video.h"
#include "../../engine/include/i_input.h"
#include "../../engine/include/i_audio.h"
#include "../../engine/include/i_config.h"
#include "../../engine/include/r_renderer.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    gamestate_t state;
    gamestate_t previousState;
    bool paused;
    
    int currentLevel;
    int currentSkill;
    
    float deltaTime;
    float accumulator;
    float fixedTimeStep;
    
    float soundVolume;
    float musicVolume;
    float mouseSensitivity;
    
    char dataPath[512];
    int screenWidth;
    int screenHeight;
    
    ini_file_t gameConfig;
    ini_file_t inputConfig;
    
    bool initialized;
} game_state_t;

static game_state_t gameState = {
    .state = GS_MENU,
    .previousState = GS_MENU,
    .paused = false,
    .currentLevel = 1,
    .currentSkill = 1,
    .deltaTime = 0.0f,
    .accumulator = 0.0f,
    .fixedTimeStep = 1.0f / 60.0f,
    .soundVolume = 1.0f,
    .musicVolume = 0.7f,
    .mouseSensitivity = 1.0f,
    .initialized = false
};

static void G_LoadConfiguration(void);
static void G_SaveConfiguration(void);
static void G_InitializeGame(void);
static void G_ShutdownGame(void);

bool G_Init(const game_config_t* config) {
    if (gameState.initialized) return true;
    
    if (!config) {
        I_Error("G_Init: NULL config");
        return false;
    }
    
    strncpy(gameState.dataPath, config->dataPath, sizeof(gameState.dataPath) - 1);
    gameState.dataPath[sizeof(gameState.dataPath) - 1] = '\0';
    
    gameState.screenWidth = config->screenWidth;
    gameState.screenHeight = config->screenHeight;
    
    R_Init();
    
    G_LoadConfiguration();
    
    G_InitializeGame();
    
    I_DisableCursor();
    
    I_Log("Game initialized successfully");
    gameState.initialized = true;
    return true;
}

void G_Shutdown(void) {
    if (!gameState.initialized) return;
    
    G_SaveConfiguration();
    
    G_ShutdownGame();
    
    if (gameState.gameConfig) {
        I_UnloadINI(gameState.gameConfig);
        gameState.gameConfig = NULL;
    }
    
    if (gameState.inputConfig) {
        I_UnloadINI(gameState.inputConfig);
        gameState.inputConfig = NULL;
    }
    
    R_Shutdown();
    
    I_Log("Game shutdown");
    gameState.initialized = false;
}

void G_RunFrame(void) {
    if (!gameState.initialized) return;
    
    float frameTime = I_GetDeltaTime();
    
    frameTime = frameTime > 0.25f ? 0.25f : frameTime;
    
    gameState.accumulator += frameTime;
    
    while (gameState.accumulator >= gameState.fixedTimeStep) {
        G_UpdateGame(gameState.fixedTimeStep);
        gameState.accumulator -= gameState.fixedTimeStep;
    }
    
    gameState.deltaTime = frameTime;
}

void G_Draw(void) {
    if (!gameState.initialized) return;
    
    uint32_t* framebuffer = I_GetFramebuffer();
    if (!framebuffer) return;
    
    int width, height;
    I_GetScreenSize(&width, &height);
    
    memset(framebuffer, 0, width * height * sizeof(uint32_t));
    
    switch (gameState.state) {
        case GS_MENU:
            break;
            
        case GS_PLAYING:
            break;
            
        case GS_PAUSED:
            break;
            
        case GS_LOADING:
            {
                const char* loadingText = "LOADING...";
                int textWidth = R_MeasureText(loadingText, 40);
                R_DrawText(loadingText, (width - textWidth) / 2, height / 2 - 20, 40, COLOR_WHITE);
            }
            break;
            
        case GS_INTERMISSION:
            break;
            
        case GS_FINALE:
            break;
    }
    
    if (gameState.state != GS_MENU && gameState.state != GS_LOADING) {
        char fpsText[32];
        snprintf(fpsText, sizeof(fpsText), "FPS: %d", I_GetFPS());
        R_DrawText(fpsText, 10, 10, 20, COLOR_GREEN);
    }
}

void G_NewGame(int skill, int level) {
    I_Log("Starting new game - Skill: %d, Level: %d", skill, level);
    
    gameState.currentSkill = skill;
    gameState.currentLevel = level;
    
    G_SetGameState(GS_LOADING);
    
    G_SetGameState(GS_PLAYING);
}

void G_LoadGame(const char* filename) {
    if (!filename) return;
    
    I_Log("Loading game from: %s", filename);
    
    G_SetGameState(GS_LOADING);
    
    G_SetGameState(GS_PLAYING);
}

void G_SaveGame(const char* filename) {
    if (!filename) return;
    
    I_Log("Saving game to: %s", filename);
}

void G_SetGameState(gamestate_t state) {
    if (state == gameState.state) return;
    
    I_Log("Game state change: %d -> %d", gameState.state, state);
    
    gameState.previousState = gameState.state;
    gameState.state = state;
    
    switch (state) {
        case GS_MENU:
            I_EnableCursor();
            break;
            
        case GS_PLAYING:
            I_DisableCursor();
            gameState.paused = false;
            break;
            
        case GS_PAUSED:
            I_EnableCursor();
            gameState.paused = true;
            break;
            
        default:
            break;
    }
}

gamestate_t G_GetGameState(void) {
    return gameState.state;
}

void G_PauseGame(void) {
    if (gameState.state == GS_PLAYING) {
        G_SetGameState(GS_PAUSED);
    }
}

void G_ResumeGame(void) {
    if (gameState.state == GS_PAUSED) {
        G_SetGameState(GS_PLAYING);
    }
}

bool G_IsPaused(void) {
    return gameState.paused;
}

void G_ProcessInput(void) {
    if (!gameState.initialized) return;
    
    if (I_IsKeyPressed(KEY_ESCAPE)) {
        switch (gameState.state) {
            case GS_PLAYING:
                G_PauseGame();
                break;
            case GS_PAUSED:
                G_ResumeGame();
                break;
            default:
                break;
        }
    }
    
    if (gameState.state == GS_PLAYING) {
        if (I_IsKeyPressed(KEY_F1)) {
        }
    }
}

void G_UpdateGame(float deltaTime) {
    if (!gameState.initialized) return;
    
    if (gameState.paused) return;
    
    switch (gameState.state) {
        case GS_MENU:
            break;
            
        case GS_PLAYING:
            break;
            
        case GS_LOADING:
            break;
            
        case GS_INTERMISSION:
            break;
            
        case GS_FINALE:
            break;
            
        default:
            break;
    }
}

void G_PlayerDied(void) {
    I_Log("Player died!");
}

void G_LevelComplete(void) {
    I_Log("Level complete!");
    G_SetGameState(GS_INTERMISSION);
}

void G_SetMouseSensitivity(float sensitivity) {
    gameState.mouseSensitivity = sensitivity;
    G_SaveConfiguration();
}

float G_GetMouseSensitivity(void) {
    return gameState.mouseSensitivity;
}

void G_SetSoundVolume(float volume) {
    gameState.soundVolume = volume;
    I_SetMasterVolume(volume);
    G_SaveConfiguration();
}

float G_GetSoundVolume(void) {
    return gameState.soundVolume;
}

void G_SetMusicVolume(float volume) {
    gameState.musicVolume = volume;
    G_SaveConfiguration();
}

float G_GetMusicVolume(void) {
    return gameState.musicVolume;
}

static void G_LoadConfiguration(void) {
    char configPath[1024];
    
    snprintf(configPath, sizeof(configPath), "%s/GameConfig/GameConfig.ini", gameState.dataPath);
    gameState.gameConfig = I_LoadINI(configPath);
    
    if (gameState.gameConfig) {
        I_INIGetFloat(gameState.gameConfig, "Audio", "MasterVolume", &gameState.soundVolume, 1.0f);
        I_INIGetFloat(gameState.gameConfig, "Audio", "MusicVolume", &gameState.musicVolume, 0.7f);
    }
    
    snprintf(configPath, sizeof(configPath), "%s/GameConfig/InputConfig.ini", gameState.dataPath);
    gameState.inputConfig = I_LoadINI(configPath);
    
    if (gameState.inputConfig) {
        I_INIGetFloat(gameState.inputConfig, "Mouse", "Sensitivity", &gameState.mouseSensitivity, 1.0f);
    }
    
    I_SetMasterVolume(gameState.soundVolume);
}

static void G_SaveConfiguration(void) {
    if (gameState.gameConfig) {
        I_INISetFloat(gameState.gameConfig, "Audio", "MasterVolume", gameState.soundVolume);
        I_INISetFloat(gameState.gameConfig, "Audio", "MusicVolume", gameState.musicVolume);
        
        char configPath[1024];
        snprintf(configPath, sizeof(configPath), "%s/GameConfig/GameConfig.ini", gameState.dataPath);
        I_INISave(gameState.gameConfig, configPath);
    }
    
    if (gameState.inputConfig) {
        I_INISetFloat(gameState.inputConfig, "Mouse", "Sensitivity", gameState.mouseSensitivity);
        
        char configPath[1024];
        snprintf(configPath, sizeof(configPath), "%s/GameConfig/InputConfig.ini", gameState.dataPath);
        I_INISave(gameState.inputConfig, configPath);
    }
}

static void G_InitializeGame(void) {
    I_Log("Initializing game systems...");
}

static void G_ShutdownGame(void) {
    I_Log("Shutting down game systems...");
}