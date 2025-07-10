#include "../include/g_level.h"
#include "../include/g_player.h"
#include "../../engine/include/i_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LINE_LENGTH 1024

static bool parseMapLine(const char* line, uint8_t* tiles, int width);
static bool parseTextureLine(const char* line, uint8_t* textures, int width);

level_t* G_CreateLevel(void) {
    level_t* level = (level_t*)I_Calloc(1, sizeof(level_t));
    
    level->ambientLight = 0.3f;
    level->fogColor = (rgba_t){0, 0, 0, 255};
    level->fogDensity = 0.0f;
    
    I_Log("Level created");
    return level;
}

void G_DestroyLevel(level_t* level) {
    if (!level) return;
    
    G_UnloadLevel(level);
    I_Free(level);
    
    I_Log("Level destroyed");
}

bool G_LoadLevel(level_t* level, const char* levelPath) {
    if (!level || !levelPath) return false;
    
    G_UnloadLevel(level);
    
    char configPath[1024];
    snprintf(configPath, sizeof(configPath), "%s/LevelData/Config.ini", levelPath);
    
    ini_file_t config = I_LoadINI(configPath);
    if (!config) {
        I_Warning("Failed to load level config: %s", configPath);
        return false;
    }
    
    if (!G_LoadMapConfig(level, config)) {
        I_UnloadINI(config);
        return false;
    }
    
    char mapPath[1024];
    I_INIGetString(config, "Map", "File", mapPath, sizeof(mapPath), "Level.map");
    
    char fullMapPath[1024];
    snprintf(fullMapPath, sizeof(fullMapPath), "%s/LevelData/%s", levelPath, mapPath);
    
    if (!G_LoadMapData(level, fullMapPath)) {
        I_UnloadINI(config);
        return false;
    }
    
    snprintf(configPath, sizeof(configPath), "%s/LevelData/TexturesData.conf", levelPath);
    if (I_FileExists(configPath)) {
        G_LoadTextureData(level, configPath);
    }
    
    snprintf(configPath, sizeof(configPath), "%s/LevelData/StaticLightsData.conf", levelPath);
    if (I_FileExists(configPath)) {
        G_LoadLightData(level, configPath);
    }
    
    snprintf(configPath, sizeof(configPath), "%s/EntityData", levelPath);
    if (I_FileExists(configPath)) {
        G_LoadEntityData(level, configPath);
    }
    
    snprintf(configPath, sizeof(configPath), "%s/LevelData/MapDoorData.conf", levelPath);
    if (I_FileExists(configPath)) {
        G_LoadDoorData(level, configPath);
    }
    
    I_UnloadINI(config);
    
    I_Log("Level loaded: %s (%dx%d)", level->name, level->width, level->height);
    return true;
}

bool G_LoadLevelFromNumber(level_t* level, int levelNumber, const char* dataPath) {
    char levelPath[1024];
    snprintf(levelPath, sizeof(levelPath), "%s/Levels/%d", dataPath, levelNumber);
    return G_LoadLevel(level, levelPath);
}

void G_UnloadLevel(level_t* level) {
    if (!level) return;
    
    I_Free(level->tiles);
    I_Free(level->floorTextures);
    I_Free(level->ceilingTextures);
    for (int i = 0; i < 4; i++) {
        I_Free(level->wallTextures[i]);
    }
    I_Free(level->lightMap);
    I_Free(level->entities);
    I_Free(level->doors);
    I_Free(level->lights);
    
    memset(level, 0, sizeof(level_t));
}

bool G_LoadMapData(level_t* level, const char* mapFile) {
    if (!level || !mapFile) return false;
    
    char* buffer;
    size_t size;
    if (!I_LoadConfigFile(mapFile, &buffer, &size)) {
        return false;
    }
    
    int lineCount = 0;
    char* line = strtok(buffer, "\n");
    
    while (line && lineCount == 0) {
        if (strlen(line) > 0 && line[0] != '#') {
            char* comma = strchr(line, ',');
            if (comma) {
                level->width = 0;
                while (*line && *line != '\n') {
                    if (*line == ',') level->width++;
                    line++;
                }
                level->width++;
                lineCount = 1;
            }
        }
        line = strtok(NULL, "\n");
    }
    
    I_Free(buffer);
    
    if (level->width == 0) {
        I_Warning("Invalid map format");
        return false;
    }
    
    if (!I_LoadConfigFile(mapFile, &buffer, &size)) {
        return false;
    }
    
    int maxHeight = MAX_MAP_HEIGHT;
    uint8_t* tempTiles = (uint8_t*)I_Malloc(level->width * maxHeight);
    
    level->height = 0;
    const char* bufferPtr = buffer;
    
    while (level->height < maxHeight) {
        const char* line = I_GetConfigLine(&bufferPtr);
        if (!line) break;
        
        if (strlen(line) > 0 && line[0] != '#') {
            if (parseMapLine(line, &tempTiles[level->height * level->width], level->width)) {
                level->height++;
            }
        }
        I_Free((void*)line);
    }
    
    I_Free(buffer);
    
    if (level->height == 0) {
        I_Free(tempTiles);
        I_Warning("No valid map data found");
        return false;
    }
    
    level->tiles = (uint8_t*)I_Malloc(level->width * level->height);
    memcpy(level->tiles, tempTiles, level->width * level->height);
    I_Free(tempTiles);
    
    level->floorTextures = (uint8_t*)I_Calloc(level->width * level->height, 1);
    level->ceilingTextures = (uint8_t*)I_Calloc(level->width * level->height, 1);
    for (int i = 0; i < 4; i++) {
        level->wallTextures[i] = (uint8_t*)I_Calloc(level->width * level->height, 1);
    }
    level->lightMap = (uint8_t*)I_Calloc(level->width * level->height, 1);
    
    return true;
}

bool G_LoadMapConfig(level_t* level, ini_file_t config) {
    if (!level || !config) return false;
    
    I_INIGetString(config, "Level", "Name", level->name, sizeof(level->name), "Unnamed Level");
    I_INIGetString(config, "Level", "NextLevel", level->nextLevel, sizeof(level->nextLevel), "");
    I_INIGetString(config, "Level", "Music", level->music, sizeof(level->music), "");
    
    I_INIGetFloat(config, "Player", "StartX", &level->playerStart.x, 1.5f);
    I_INIGetFloat(config, "Player", "StartY", &level->playerStart.y, 1.5f);
    I_INIGetFloat(config, "Player", "StartAngle", &level->playerStartAngle, 0.0f);
    
    level->playerStart.x *= TILE_SIZE;
    level->playerStart.y *= TILE_SIZE;
    
    I_INIGetFloat(config, "Lighting", "AmbientLight", &level->ambientLight, 0.3f);
    I_INIGetFloat(config, "Fog", "Density", &level->fogDensity, 0.0f);
    I_INIGetColor(config, "Fog", "Color", 
                  &level->fogColor.r, &level->fogColor.g, &level->fogColor.b, &level->fogColor.a,
                  0, 0, 0, 255);
    
    return true;
}

bool G_LoadTextureData(level_t* level, const char* textureFile) {
    if (!level || !textureFile) return false;
    
    char* buffer;
    size_t size;
    if (!I_LoadConfigFile(textureFile, &buffer, &size)) {
        return false;
    }
    
    const char* bufferPtr = buffer;
    int section = -1;
    int y = 0;
    
    while (true) {
        const char* line = I_GetConfigLine(&bufferPtr);
        if (!line) break;
        
        if (strstr(line, "[MapFloorData]")) {
            section = 0;
            y = 0;
        } else if (strstr(line, "[MapCeilingData]")) {
            section = 1;
            y = 0;
        } else if (strstr(line, "[MapWallData_North]")) {
            section = 2;
            y = 0;
        } else if (strstr(line, "[MapWallData_East]")) {
            section = 3;
            y = 0;
        } else if (strstr(line, "[MapWallData_South]")) {
            section = 4;
            y = 0;
        } else if (strstr(line, "[MapWallData_West]")) {
            section = 5;
            y = 0;
        } else if (strlen(line) > 0 && line[0] != '#' && section >= 0) {
            uint8_t* target = NULL;
            
            switch (section) {
                case 0: target = level->floorTextures; break;
                case 1: target = level->ceilingTextures; break;
                case 2: target = level->wallTextures[0]; break;
                case 3: target = level->wallTextures[1]; break;
                case 4: target = level->wallTextures[2]; break;
                case 5: target = level->wallTextures[3]; break;
            }
            
            if (target && y < level->height) {
                parseTextureLine(line, &target[y * level->width], level->width);
                y++;
            }
        }
        
        I_Free((void*)line);
    }
    
    I_Free(buffer);
    return true;
}

bool G_LoadLightData(level_t* level, const char* lightFile) {
    if (!level || !lightFile) return false;
    
    char* buffer;
    size_t size;
    if (!I_LoadConfigFile(lightFile, &buffer, &size)) {
        return false;
    }
    
    level->lightCount = 0;
    const char* bufferPtr = buffer;
    
    while (true) {
        const char* line = I_GetConfigLine(&bufferPtr);
        if (!line) break;
        
        if (strlen(line) > 0 && line[0] != '#') {
            level->lightCount++;
        }
        I_Free((void*)line);
    }
    
    if (level->lightCount > 0) {
        level->lights = (light_t*)I_Calloc(level->lightCount, sizeof(light_t));
        
        I_Free(buffer);
        if (!I_LoadConfigFile(lightFile, &buffer, &size)) {
            return false;
        }
        
        bufferPtr = buffer;
        int lightIndex = 0;
        
        while (lightIndex < level->lightCount) {
            const char* line = I_GetConfigLine(&bufferPtr);
            if (!line) break;
            
            if (strlen(line) > 0 && line[0] != '#') {
                light_t* light = &level->lights[lightIndex];
                int r, g, b, flicker;
                
                if (sscanf(line, "%f,%f,%f,%d,%d,%d,%f,%f,%d",
                          &light->position.x, &light->position.y, &light->position.z,
                          &r, &g, &b,
                          &light->intensity, &light->radius, &flicker) == 9) {
                    light->color.r = r;
                    light->color.g = g;
                    light->color.b = b;
                    light->color.a = 255;
                    light->flicker = flicker != 0;
                    light->flickerSpeed = 5.0f;
                    
                    light->position.x *= TILE_SIZE;
                    light->position.y *= TILE_SIZE;
                    
                    lightIndex++;
                }
            }
            I_Free((void*)line);
        }
    }
    
    I_Free(buffer);
    return true;
}

bool G_LoadEntityData(level_t* level, const char* entityPath) {
    if (!level || !entityPath) return false;
    
    level->entityCount = 0;
    
    for (int i = 0; i < 1000; i++) {
        char entityFile[1024];
        snprintf(entityFile, sizeof(entityFile), "%s/%d.ini", entityPath, i);
        
        if (!I_FileExists(entityFile)) continue;
        
        level->entityCount++;
    }
    
    if (level->entityCount == 0) return true;
    
    level->entities = (entity_t*)I_Calloc(level->entityCount, sizeof(entity_t));
    int entityIndex = 0;
    
    for (int i = 0; i < 1000 && entityIndex < level->entityCount; i++) {
        char entityFile[1024];
        snprintf(entityFile, sizeof(entityFile), "%s/%d.ini", entityPath, i);
        
        if (!I_FileExists(entityFile)) continue;
        
        ini_file_t entityConfig = I_LoadINI(entityFile);
        if (!entityConfig) continue;
        
        entity_t* entity = &level->entities[entityIndex];
        
        char typeStr[64];
        I_INIGetString(entityConfig, "Entity", "Type", typeStr, sizeof(typeStr), "");
        
        if (strcmp(typeStr, "Soldier") == 0) entity->type = ET_ENEMY_SOLDIER;
        else if (strcmp(typeStr, "Demon") == 0) entity->type = ET_ENEMY_DEMON;
        else if (strcmp(typeStr, "AmmoBox") == 0) entity->type = ET_ITEM_AMMO;
        else if (strcmp(typeStr, "Turret") == 0) entity->type = ET_TURRET;
        
        I_INIGetFloat(entityConfig, "Entity", "X", &entity->position.x, 0);
        I_INIGetFloat(entityConfig, "Entity", "Y", &entity->position.y, 0);
        I_INIGetFloat(entityConfig, "Entity", "Angle", &entity->angle, 0);
        
        entity->position.x *= TILE_SIZE;
        entity->position.y *= TILE_SIZE;
        
        I_INIGetInt(entityConfig, "Entity", "Health", &entity->health, 100);
        entity->maxHealth = entity->health;
        
        I_INIGetFloat(entityConfig, "Entity", "Speed", &entity->speed, 100.0f);
        I_INIGetFloat(entityConfig, "Entity", "SightRange", &entity->sightRange, 512.0f);
        I_INIGetFloat(entityConfig, "Entity", "AttackRange", &entity->attackRange, 128.0f);
        I_INIGetInt(entityConfig, "Entity", "Damage", &entity->damage, 10);
        
        entity->radius = PLAYER_RADIUS;
        entity->height = PLAYER_HEIGHT;
        entity->active = true;
        entity->visible = true;
        entity->solid = (entity->type == ET_ENEMY_SOLDIER || 
                        entity->type == ET_ENEMY_DEMON || 
                        entity->type == ET_TURRET);
        entity->pickupable = (entity->type == ET_ITEM_AMMO || 
                             entity->type == ET_ITEM_HEALTH || 
                             entity->type == ET_ITEM_ARMOR);
        
        I_UnloadINI(entityConfig);
        entityIndex++;
    }
    
    return true;
}

bool G_LoadDoorData(level_t* level, const char* doorFile) {
    if (!level || !doorFile) return false;
    
    char* buffer;
    size_t size;
    if (!I_LoadConfigFile(doorFile, &buffer, &size)) {
        return false;
    }
    
    level->doorCount = 0;
    const char* bufferPtr = buffer;
    
    while (true) {
        const char* line = I_GetConfigLine(&bufferPtr);
        if (!line) break;
        
        if (strlen(line) > 0 && line[0] != '#') {
            level->doorCount++;
        }
        I_Free((void*)line);
    }
    
    if (level->doorCount > 0) {
        level->doors = (door_t*)I_Calloc(level->doorCount, sizeof(door_t));
        
        I_Free(buffer);
        if (!I_LoadConfigFile(doorFile, &buffer, &size)) {
            return false;
        }
        
        bufferPtr = buffer;
        int doorIndex = 0;
        
        while (doorIndex < level->doorCount) {
            const char* line = I_GetConfigLine(&bufferPtr);
            if (!line) break;
            
            if (strlen(line) > 0 && line[0] != '#') {
                door_t* door = &level->doors[doorIndex];
                int locked, secret;
                
                if (sscanf(line, "%d,%d,%d,%d,%d,%d,%f",
                          &door->position.x, &door->position.y,
                          &door->textureIndex, &door->keyRequired,
                          &locked, &secret, &door->openTime) >= 5) {
                    door->locked = locked != 0;
                    door->secret = secret != 0;
                    door->state = DS_CLOSED;
                    door->openAmount = 0.0f;
                    
                    if (door->openTime <= 0) door->openTime = 3.0f;
                    
                    if (door->position.x >= 0 && door->position.x < level->width &&
                        door->position.y >= 0 && door->position.y < level->height) {
                        level->tiles[door->position.y * level->width + door->position.x] = TILE_DOOR;
                    }
                    
                    doorIndex++;
                }
            }
            I_Free((void*)line);
        }
    }
    
    I_Free(buffer);
    return true;
}

uint8_t G_GetTile(level_t* level, int x, int y) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height) {
        return TILE_WALL;
    }
    return level->tiles[y * level->width + x];
}

void G_SetTile(level_t* level, int x, int y, uint8_t tile) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height) {
        return;
    }
    level->tiles[y * level->width + x] = tile;
}

uint8_t G_GetFloorTexture(level_t* level, int x, int y) {
    if (!level || !level->floorTextures || x < 0 || x >= level->width || y < 0 || y >= level->height) {
        return 0;
    }
    return level->floorTextures[y * level->width + x];
}

uint8_t G_GetCeilingTexture(level_t* level, int x, int y) {
    if (!level || !level->ceilingTextures || x < 0 || x >= level->width || y < 0 || y >= level->height) {
        return 0;
    }
    return level->ceilingTextures[y * level->width + x];
}

uint8_t G_GetWallTexture(level_t* level, int x, int y, int side) {
    if (!level || side < 0 || side >= 4 || !level->wallTextures[side] || 
        x < 0 || x >= level->width || y < 0 || y >= level->height) {
        return 0;
    }
    return level->wallTextures[side][y * level->width + x];
}

entity_t* G_SpawnEntity(level_t* level, entity_type_t type, vec2_t position) {
    if (!level) return NULL;
    
    entity_t* newEntities = (entity_t*)I_Realloc(level->entities, 
                                                  (level->entityCount + 1) * sizeof(entity_t));
    if (!newEntities) return NULL;
    
    level->entities = newEntities;
    entity_t* entity = &level->entities[level->entityCount];
    level->entityCount++;
    
    memset(entity, 0, sizeof(entity_t));
    entity->type = type;
    entity->position = position;
    entity->active = true;
    entity->visible = true;
    
    return entity;
}

void G_RemoveEntity(level_t* level, entity_t* entity) {
    if (!level || !entity) return;
    
    entity->active = false;
    entity->visible = false;
}

entity_t* G_FindEntityAt(level_t* level, vec2_t position, float radius) {
    if (!level) return NULL;
    
    for (int i = 0; i < level->entityCount; i++) {
        entity_t* entity = &level->entities[i];
        if (!entity->active) continue;
        
        float dx = entity->position.x - position.x;
        float dy = entity->position.y - position.y;
        float distSq = dx * dx + dy * dy;
        float minDist = radius + entity->radius;
        
        if (distSq <= minDist * minDist) {
            return entity;
        }
    }
    
    return NULL;
}

door_t* G_GetDoorAt(level_t* level, int x, int y) {
    if (!level) return NULL;
    
    for (int i = 0; i < level->doorCount; i++) {
        door_t* door = &level->doors[i];
        if (door->position.x == x && door->position.y == y) {
            return door;
        }
    }
    
    return NULL;
}

bool G_OpenDoor(level_t* level, door_t* door, player_t* player) {
    if (!level || !door || !player) return false;
    
    if (door->state != DS_CLOSED) return false;
    
    if (door->locked) {
        if (door->keyRequired < 0 || door->keyRequired >= 4) return false;
        if (!G_PlayerHasKey(player, door->keyRequired)) return false;
    }
    
    door->state = DS_OPENING;
    door->stateTime = 0.0f;
    
    I_Log("Door opened at (%d, %d)", door->position.x, door->position.y);
    return true;
}

void G_UpdateDoors(level_t* level, float deltaTime) {
    if (!level) return;
    
    for (int i = 0; i < level->doorCount; i++) {
        door_t* door = &level->doors[i];
        
        switch (door->state) {
            case DS_OPENING:
                door->openAmount += deltaTime / 1.0f;
                if (door->openAmount >= 1.0f) {
                    door->openAmount = 1.0f;
                    door->state = DS_OPEN;
                    door->stateTime = 0.0f;
                }
                break;
                
            case DS_OPEN:
                door->stateTime += deltaTime;
                if (door->stateTime >= door->openTime) {
                    door->state = DS_CLOSING;
                }
                break;
                
            case DS_CLOSING:
                door->openAmount -= deltaTime / 1.0f;
                if (door->openAmount <= 0.0f) {
                    door->openAmount = 0.0f;
                    door->state = DS_CLOSED;
                }
                break;
                
            default:
                break;
        }
    }
}

light_t* G_AddLight(level_t* level, vec3_t position, rgba_t color, float intensity, float radius) {
    if (!level) return NULL;
    
    light_t* newLights = (light_t*)I_Realloc(level->lights, 
                                              (level->lightCount + 1) * sizeof(light_t));
    if (!newLights) return NULL;
    
    level->lights = newLights;
    light_t* light = &level->lights[level->lightCount];
    level->lightCount++;
    
    light->position = position;
    light->color = color;
    light->intensity = intensity;
    light->radius = radius;
    light->flicker = false;
    light->flickerSpeed = 5.0f;
    
    return light;
}

void G_RemoveLight(level_t* level, light_t* light) {
    if (!level || !light) return;
}

rgba_t G_CalculateLightingAt(level_t* level, vec2_t position) {
    rgba_t result = {0, 0, 0, 255};
    
    if (!level) return result;
    
    float totalR = level->ambientLight * 255;
    float totalG = level->ambientLight * 255;
    float totalB = level->ambientLight * 255;
    
    for (int i = 0; i < level->lightCount; i++) {
        light_t* light = &level->lights[i];
        
        float dx = light->position.x - position.x;
        float dy = light->position.y - position.y;
        float distSq = dx * dx + dy * dy;
        
        if (distSq < light->radius * light->radius) {
            float dist = sqrtf(distSq);
            float attenuation = 1.0f - (dist / light->radius);
            
            float intensity = light->intensity;
            if (light->flicker) {
                intensity *= 0.8f + 0.2f * sinf(I_GetTime() * 0.001f * light->flickerSpeed);
            }
            
            totalR += light->color.r * attenuation * intensity;
            totalG += light->color.g * attenuation * intensity;
            totalB += light->color.b * attenuation * intensity;
        }
    }
    
    result.r = totalR > 255 ? 255 : (uint8_t)totalR;
    result.g = totalG > 255 ? 255 : (uint8_t)totalG;
    result.b = totalB > 255 ? 255 : (uint8_t)totalB;
    
    return result;
}

bool G_CheckLevelComplete(level_t* level) {
    if (!level) return false;
    
    for (int i = 0; i < level->entityCount; i++) {
        entity_t* entity = &level->entities[i];
        if (entity->active && (entity->type == ET_ENEMY_SOLDIER || 
                               entity->type == ET_ENEMY_DEMON)) {
            return false;
        }
    }
    
    return true;
}

int G_CountSecrets(level_t* level) {
    if (!level) return 0;
    
    int count = 0;
    for (int i = 0; i < level->doorCount; i++) {
        if (level->doors[i].secret) count++;
    }
    
    return count;
}

int G_CountEnemies(level_t* level) {
    if (!level) return 0;
    
    int count = 0;
    for (int i = 0; i < level->entityCount; i++) {
        entity_t* entity = &level->entities[i];
        if (entity->active && (entity->type == ET_ENEMY_SOLDIER || 
                               entity->type == ET_ENEMY_DEMON)) {
            count++;
        }
    }
    
    return count;
}

void G_SaveLevelState(level_t* level, void* buffer) {
    if (!level || !buffer) return;
}

void G_LoadLevelState(level_t* level, const void* buffer) {
    if (!level || !buffer) return;
}

static bool parseMapLine(const char* line, uint8_t* tiles, int width) {
    if (!line || !tiles) return false;
    
    int x = 0;
    const char* ptr = line;
    
    while (*ptr && x < width) {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        
        if (*ptr == '\0' || *ptr == '\n' || *ptr == '\r') break;
        
        char* end;
        long value = strtol(ptr, &end, 10);
        
        if (end == ptr) break;
        
        tiles[x] = (uint8_t)value;
        x++;
        
        ptr = end;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == ',') ptr++;
    }
    
    return x == width;
}

static bool parseTextureLine(const char* line, uint8_t* textures, int width) {
    return parseMapLine(line, textures, width);
}