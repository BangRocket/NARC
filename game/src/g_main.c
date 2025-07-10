#include "../include/g_main.h"
#include "../../engine/include/i_system.h"
#include "../../engine/include/i_video.h"
#include "../../engine/include/i_input.h"
#include "../../engine/include/i_audio.h"
#include "../../engine/include/r_renderer.h"
#include <string.h>

game_context_t* G_CreateGameContext(void) {
    game_context_t* context = (game_context_t*)I_Calloc(1, sizeof(game_context_t));
    
    context->renderConfig.textureSize = 64;
    context->renderConfig.maxTextureSize = 256;
    context->renderConfig.renderDistance = 20;
    context->renderConfig.fov = 66;
    context->renderConfig.useTextures = true;
    context->renderConfig.useFloorCeiling = true;
    context->renderConfig.useLighting = true;
    context->renderConfig.useFog = true;
    context->renderConfig.useShadows = false;
    context->renderConfig.useMultithreading = false;
    context->renderConfig.threadCount = 1;
    context->renderConfig.resolutionScale = 1;
    
    I_Log("Game context created");
    return context;
}

void G_DestroyGameContext(game_context_t* context) {
    if (!context) return;
    
    G_ShutdownGameContext(context);
    I_Free(context);
    
    I_Log("Game context destroyed");
}

bool G_InitializeGameContext(game_context_t* context, const game_config_t* config) {
    if (!context || !config) return false;
    
    if (context->initialized) return true;
    
    int width, height;
    I_GetScreenSize(&width, &height);
    
    context->player = G_CreatePlayer();
    if (!context->player) {
        I_Error("Failed to create player");
        return false;
    }
    
    context->level = G_CreateLevel();
    if (!context->level) {
        I_Error("Failed to create level");
        return false;
    }
    
    context->weaponManager = G_CreateWeaponManager();
    if (!context->weaponManager) {
        I_Error("Failed to create weapon manager");
        return false;
    }
    
    if (!G_LoadWeapons(context->weaponManager, config->dataPath)) {
        I_Warning("Failed to load weapons");
    }
    
    context->raycaster = G_CreateRaycaster(width, height);
    if (!context->raycaster) {
        I_Error("Failed to create raycaster");
        return false;
    }
    
    G_SetRaycasterPlayer(context->raycaster, context->player);
    G_SetRaycasterLevel(context->raycaster, context->level);
    G_SetRaycasterConfig(context->raycaster, &context->renderConfig);
    
    context->textures = (raycaster_textures_t*)I_Calloc(1, sizeof(raycaster_textures_t));
    
    if (!G_LoadGameTextures(context, config->dataPath)) {
        I_Warning("Failed to load textures - using fallback colors");
    }
    
    G_SetRaycasterTextures(context->raycaster, context->textures);
    
    context->initialized = true;
    I_Log("Game context initialized");
    
    return true;
}

void G_ShutdownGameContext(game_context_t* context) {
    if (!context || !context->initialized) return;
    
    G_UnloadGameLevel(context);
    G_UnloadGameTextures(context);
    
    if (context->pathfinder) {
        G_DestroyPathfinder(context->pathfinder);
        context->pathfinder = NULL;
    }
    
    if (context->raycaster) {
        G_DestroyRaycaster(context->raycaster);
        context->raycaster = NULL;
    }
    
    if (context->textures) {
        I_Free(context->textures);
        context->textures = NULL;
    }
    
    if (context->weaponManager) {
        G_DestroyWeaponManager(context->weaponManager);
        context->weaponManager = NULL;
    }
    
    if (context->level) {
        G_DestroyLevel(context->level);
        context->level = NULL;
    }
    
    if (context->player) {
        G_DestroyPlayer(context->player);
        context->player = NULL;
    }
    
    context->initialized = false;
    I_Log("Game context shutdown");
}

void G_UpdateGameContext(game_context_t* context, float deltaTime) {
    if (!context || !context->initialized) return;
    
    G_HandleGameInput(context);
    G_ProcessGameLogic(context, deltaTime);
}

void G_RenderGameContext(game_context_t* context) {
    if (!context || !context->initialized) return;
    
    G_RenderFrame(context->raycaster);
    
    uint32_t* framebuffer = I_GetFramebuffer();
    if (framebuffer && context->raycaster->framebuffer) {
        int width, height;
        I_GetScreenSize(&width, &height);
        memcpy(framebuffer, context->raycaster->framebuffer, width * height * sizeof(uint32_t));
    }
}

bool G_LoadGameLevel(game_context_t* context, int levelNumber) {
    if (!context || !context->level) return false;
    
    char dataPath[512];
    const char* basePath = I_GetBasePath();
    snprintf(dataPath, sizeof(dataPath), "%sDATA", basePath);
    
    if (!G_LoadLevelFromNumber(context->level, levelNumber, dataPath)) {
        I_Error("Failed to load level %d", levelNumber);
        return false;
    }
    
    G_InitPlayer(context->player, context->level->playerStart, context->level->playerStartAngle);
    
    if (context->pathfinder) {
        G_DestroyPathfinder(context->pathfinder);
    }
    context->pathfinder = G_CreatePathfinder(context->level->width, context->level->height);
    
    G_SetRaycasterLevel(context->raycaster, context->level);
    
    I_Log("Loaded level %d: %s", levelNumber, context->level->name);
    return true;
}

void G_UnloadGameLevel(game_context_t* context) {
    if (!context || !context->level) return;
    
    G_UnloadLevel(context->level);
    
    if (context->pathfinder) {
        G_DestroyPathfinder(context->pathfinder);
        context->pathfinder = NULL;
    }
}

bool G_LoadGameTextures(game_context_t* context, const char* dataPath) {
    if (!context || !context->textures || !dataPath) return false;
    
    char texturePath[1024];
    bool success = true;
    
    for (int i = 0; i < 16; i++) {
        snprintf(texturePath, sizeof(texturePath), "%s/GFX/LevelTextures/64/Wall%d.png", dataPath, i + 1);
        if (I_FileExists(texturePath)) {
            context->textures->wallTextures[i] = R_LoadTexture(texturePath);
            if (context->textures->wallTextures[i]) {
                context->textures->wallTextureCount++;
            }
        }
    }
    
    for (int i = 0; i < 8; i++) {
        snprintf(texturePath, sizeof(texturePath), "%s/GFX/LevelTextures/64/Floor%d.png", dataPath, i + 1);
        if (I_FileExists(texturePath)) {
            context->textures->floorTextures[i] = R_LoadTexture(texturePath);
            if (context->textures->floorTextures[i]) {
                context->textures->floorTextureCount++;
            }
        }
    }
    
    for (int i = 0; i < 8; i++) {
        snprintf(texturePath, sizeof(texturePath), "%s/GFX/LevelTextures/64/Ceiling%d.png", dataPath, i + 1);
        if (I_FileExists(texturePath)) {
            context->textures->ceilingTextures[i] = R_LoadTexture(texturePath);
            if (context->textures->ceilingTextures[i]) {
                context->textures->ceilingTextureCount++;
            }
        }
    }
    
    for (int i = 0; i < MAX_WEAPONS; i++) {
        snprintf(texturePath, sizeof(texturePath), "%s/GFX/PlayerAssets/Weapons/%d/Weapon.png", dataPath, i);
        if (I_FileExists(texturePath)) {
            context->textures->weaponTextures[i] = R_LoadTexture(texturePath);
            if (context->textures->weaponTextures[i]) {
                context->textures->weaponTextureCount++;
            }
        }
    }
    
    snprintf(texturePath, sizeof(texturePath), "%s/GFX/SkyBox/Sky.png", dataPath);
    if (I_FileExists(texturePath)) {
        context->textures->skyboxTexture = R_LoadTexture(texturePath);
    }
    
    I_Log("Loaded textures - Walls: %d, Floors: %d, Ceilings: %d, Weapons: %d",
          context->textures->wallTextureCount,
          context->textures->floorTextureCount,
          context->textures->ceilingTextureCount,
          context->textures->weaponTextureCount);
    
    return success;
}

void G_UnloadGameTextures(game_context_t* context) {
    if (!context || !context->textures) return;
    
    for (int i = 0; i < MAX_TEXTURES; i++) {
        if (context->textures->wallTextures[i]) {
            R_UnloadTexture(context->textures->wallTextures[i]);
            context->textures->wallTextures[i] = NULL;
        }
        if (context->textures->floorTextures[i]) {
            R_UnloadTexture(context->textures->floorTextures[i]);
            context->textures->floorTextures[i] = NULL;
        }
        if (context->textures->ceilingTextures[i]) {
            R_UnloadTexture(context->textures->ceilingTextures[i]);
            context->textures->ceilingTextures[i] = NULL;
        }
        if (context->textures->entityTextures[i]) {
            R_UnloadTexture(context->textures->entityTextures[i]);
            context->textures->entityTextures[i] = NULL;
        }
    }
    
    for (int i = 0; i < MAX_WEAPONS; i++) {
        if (context->textures->weaponTextures[i]) {
            R_UnloadTexture(context->textures->weaponTextures[i]);
            context->textures->weaponTextures[i] = NULL;
        }
    }
    
    if (context->textures->skyboxTexture) {
        R_UnloadTexture(context->textures->skyboxTexture);
        context->textures->skyboxTexture = NULL;
    }
    
    context->textures->wallTextureCount = 0;
    context->textures->floorTextureCount = 0;
    context->textures->ceilingTextureCount = 0;
    context->textures->entityTextureCount = 0;
    context->textures->weaponTextureCount = 0;
}

void G_HandleGameInput(game_context_t* context) {
    if (!context || !context->player || !context->level) return;
    
    float deltaTime = I_GetDeltaTime();
    
    G_ProcessPlayerInput(context->player, deltaTime);
    
    float forward = 0.0f;
    float strafe = 0.0f;
    
    if (I_IsKeyDown(KEY_W) || I_IsKeyDown(KEY_UP)) forward += 1.0f;
    if (I_IsKeyDown(KEY_S) || I_IsKeyDown(KEY_DOWN)) forward -= 1.0f;
    if (I_IsKeyDown(KEY_A)) strafe -= 1.0f;
    if (I_IsKeyDown(KEY_D)) strafe += 1.0f;
    
    G_MovePlayer(context->player, context->level, forward, strafe, deltaTime);
    
    if (context->player->isShooting) {
        weapon_t* weapon = G_GetPlayerWeapon(context->weaponManager, context->player);
        if (weapon) {
            G_FireWeapon(weapon, context->player, context->level);
        }
        context->player->isShooting = false;
    }
    
    if (I_IsKeyPressed(KEY_R)) {
        weapon_t* weapon = G_GetPlayerWeapon(context->weaponManager, context->player);
        if (weapon) {
            G_ReloadWeapon(weapon, context->player);
        }
    }
    
    if (I_IsKeyPressed(KEY_E)) {
        int tileX = (int)(context->player->position.x / TILE_SIZE);
        int tileY = (int)(context->player->position.y / TILE_SIZE);
        
        int dx[] = {0, 1, 0, -1};
        int dy[] = {-1, 0, 1, 0};
        
        for (int i = 0; i < 4; i++) {
            int checkX = tileX + dx[i];
            int checkY = tileY + dy[i];
            
            door_t* door = G_GetDoorAt(context->level, checkX, checkY);
            if (door) {
                G_OpenDoor(context->level, door, context->player);
                break;
            }
        }
    }
}

void G_ProcessGameLogic(game_context_t* context, float deltaTime) {
    if (!context) return;
    
    G_UpdatePlayer(context->player, context->level, deltaTime);
    G_UpdateAllWeapons(context->weaponManager, context->player, deltaTime);
    G_UpdateAllEntities(context->level, context->player, deltaTime);
    G_UpdateDoors(context->level, deltaTime);
    
    if (context->player->health <= 0) {
        G_PlayerDied();
    }
    
    if (G_CheckLevelComplete(context->level)) {
        G_LevelComplete();
    }
}