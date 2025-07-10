#ifndef G_LEVEL_H
#define G_LEVEL_H

#include "g_defs.h"
#include "../../engine/include/i_config.h"

#ifdef __cplusplus
extern "C" {
#endif

level_t* G_CreateLevel(void);
void G_DestroyLevel(level_t* level);

bool G_LoadLevel(level_t* level, const char* levelPath);
bool G_LoadLevelFromNumber(level_t* level, int levelNumber, const char* dataPath);
void G_UnloadLevel(level_t* level);

bool G_LoadMapData(level_t* level, const char* mapFile);
bool G_LoadMapConfig(level_t* level, ini_file_t config);
bool G_LoadTextureData(level_t* level, const char* textureFile);
bool G_LoadLightData(level_t* level, const char* lightFile);
bool G_LoadEntityData(level_t* level, const char* entityPath);
bool G_LoadDoorData(level_t* level, const char* doorFile);

uint8_t G_GetTile(level_t* level, int x, int y);
void G_SetTile(level_t* level, int x, int y, uint8_t tile);

uint8_t G_GetFloorTexture(level_t* level, int x, int y);
uint8_t G_GetCeilingTexture(level_t* level, int x, int y);
uint8_t G_GetWallTexture(level_t* level, int x, int y, int side);

entity_t* G_SpawnEntity(level_t* level, entity_type_t type, vec2_t position);
void G_RemoveEntity(level_t* level, entity_t* entity);
entity_t* G_FindEntityAt(level_t* level, vec2_t position, float radius);

door_t* G_GetDoorAt(level_t* level, int x, int y);
bool G_OpenDoor(level_t* level, door_t* door, player_t* player);
void G_UpdateDoors(level_t* level, float deltaTime);

light_t* G_AddLight(level_t* level, vec3_t position, rgba_t color, float intensity, float radius);
void G_RemoveLight(level_t* level, light_t* light);
rgba_t G_CalculateLightingAt(level_t* level, vec2_t position);

bool G_CheckLevelComplete(level_t* level);
int G_CountSecrets(level_t* level);
int G_CountEnemies(level_t* level);

void G_SaveLevelState(level_t* level, void* buffer);
void G_LoadLevelState(level_t* level, const void* buffer);

#ifdef __cplusplus
}
#endif

#endif