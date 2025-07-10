#ifndef G_MAIN_H
#define G_MAIN_H

#include "g_defs.h"
#include "g_game.h"
#include "g_player.h"
#include "g_level.h"
#include "g_entity.h"
#include "g_weapon.h"
#include "g_raycaster.h"
#include "g_pathfind.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    player_t* player;
    level_t* level;
    weapon_manager_t* weaponManager;
    raycaster_t* raycaster;
    raycaster_textures_t* textures;
    pathfinder_t* pathfinder;
    render_config_t renderConfig;
    
    bool initialized;
} game_context_t;

game_context_t* G_CreateGameContext(void);
void G_DestroyGameContext(game_context_t* context);

bool G_InitializeGameContext(game_context_t* context, const game_config_t* config);
void G_ShutdownGameContext(game_context_t* context);

void G_UpdateGameContext(game_context_t* context, float deltaTime);
void G_RenderGameContext(game_context_t* context);

bool G_LoadGameLevel(game_context_t* context, int levelNumber);
void G_UnloadGameLevel(game_context_t* context);

bool G_LoadGameTextures(game_context_t* context, const char* dataPath);
void G_UnloadGameTextures(game_context_t* context);

void G_HandleGameInput(game_context_t* context);
void G_ProcessGameLogic(game_context_t* context, float deltaTime);

#ifdef __cplusplus
}
#endif

#endif