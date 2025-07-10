#include "../include/g_weapon.h"
#include "../include/g_player.h"
#include "../include/g_level.h"
#include "../include/g_entity.h"
#include "../include/g_raycaster.h"
#include "../../engine/include/i_system.h"
#include "../../engine/include/i_audio.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WEAPON_BOB_AMOUNT 0.03f
#define WEAPON_BOB_SPEED 8.0f
#define WEAPON_SWAY_AMOUNT 0.001f
#define WEAPON_RECOIL_AMOUNT 0.1f
#define WEAPON_RECOIL_RECOVERY 5.0f

static float weaponBobTime = 0.0f;
static float weaponRecoil = 0.0f;
static vec2_t weaponSway = {0, 0};

weapon_manager_t* G_CreateWeaponManager(void) {
    weapon_manager_t* manager = (weapon_manager_t*)I_Calloc(1, sizeof(weapon_manager_t));
    I_Log("Weapon manager created");
    return manager;
}

void G_DestroyWeaponManager(weapon_manager_t* manager) {
    if (manager) {
        I_Free(manager);
        I_Log("Weapon manager destroyed");
    }
}

bool G_LoadWeapons(weapon_manager_t* manager, const char* dataPath) {
    if (!manager || !dataPath) return false;
    
    manager->weaponCount = 0;
    
    for (int i = 0; i < MAX_WEAPONS; i++) {
        char weaponFile[1024];
        snprintf(weaponFile, sizeof(weaponFile), "%s/Assets_Weapons/Weapon_%d_Data.ini", dataPath, i);
        
        if (!I_FileExists(weaponFile)) continue;
        
        ini_file_t config = I_LoadINI(weaponFile);
        if (!config) continue;
        
        weapon_t* weapon = &manager->weapons[i];
        if (G_LoadWeaponData(weapon, config)) {
            manager->weaponCount++;
            I_Log("Loaded weapon %d: %s", i, weapon->name);
        }
        
        I_UnloadINI(config);
    }
    
    manager->loaded = manager->weaponCount > 0;
    I_Log("Loaded %d weapons", manager->weaponCount);
    
    return manager->loaded;
}

bool G_LoadWeaponData(weapon_t* weapon, ini_file_t config) {
    if (!weapon || !config) return false;
    
    memset(weapon, 0, sizeof(weapon_t));
    
    I_INIGetString(config, "Weapon", "Name", weapon->name, sizeof(weapon->name), "Unknown");
    
    I_INIGetInt(config, "Stats", "Damage", &weapon->damage, 10);
    I_INIGetInt(config, "Stats", "AmmoPerShot", &weapon->ammoPerShot, 1);
    I_INIGetFloat(config, "Stats", "FireRate", &weapon->fireRate, 0.5f);
    I_INIGetFloat(config, "Stats", "ReloadTime", &weapon->reloadTime, 2.0f);
    I_INIGetFloat(config, "Stats", "SwitchTime", &weapon->switchTime, 0.5f);
    I_INIGetFloat(config, "Stats", "Range", &weapon->range, 1000.0f);
    I_INIGetFloat(config, "Stats", "Spread", &weapon->spread, 0.02f);
    
    I_INIGetInt(config, "Ammo", "ClipSize", &weapon->clipSize, 30);
    weapon->currentClip = weapon->clipSize;
    
    I_INIGetBool(config, "Behavior", "Automatic", &weapon->automatic, false);
    I_INIGetBool(config, "Behavior", "Hitscan", &weapon->hitscan, true);
    I_INIGetFloat(config, "Behavior", "ProjectileSpeed", &weapon->projectileSpeed, 500.0f);
    
    weapon->state = WS_READY;
    
    return true;
}

weapon_t* G_GetWeapon(weapon_manager_t* manager, int weaponId) {
    if (!manager || weaponId < 0 || weaponId >= MAX_WEAPONS) return NULL;
    return &manager->weapons[weaponId];
}

weapon_t* G_GetPlayerWeapon(weapon_manager_t* manager, player_t* player) {
    if (!manager || !player) return NULL;
    return G_GetWeapon(manager, player->currentWeapon);
}

void G_UpdateWeapon(weapon_t* weapon, player_t* player, float deltaTime) {
    if (!weapon || !player) return;
    
    weapon->stateTime += deltaTime;
    
    switch (weapon->state) {
        case WS_READY:
            if (player->isShooting && G_CanFireWeapon(weapon, player)) {
                G_SetWeaponState(weapon, WS_FIRING);
                player->isShooting = false;
            }
            break;
            
        case WS_FIRING:
            if (weapon->stateTime >= weapon->fireRate) {
                if (weapon->automatic && player->isShooting && G_CanFireWeapon(weapon, player)) {
                    weapon->stateTime = 0.0f;
                } else {
                    G_SetWeaponState(weapon, WS_READY);
                }
            }
            break;
            
        case WS_RELOADING:
            if (weapon->stateTime >= weapon->reloadTime) {
                int ammoNeeded = weapon->clipSize - weapon->currentClip;
                int ammoAvailable = G_GetWeaponAmmo(player, weapon);
                int ammoToLoad = ammoNeeded < ammoAvailable ? ammoNeeded : ammoAvailable;
                
                weapon->currentClip += ammoToLoad;
                player->ammo[player->currentWeapon] -= ammoToLoad;
                
                G_SetWeaponState(weapon, WS_READY);
                I_Log("Weapon reloaded: %d rounds", weapon->currentClip);
            }
            break;
            
        case WS_SWITCHING:
            if (weapon->stateTime >= weapon->switchTime) {
                G_SetWeaponState(weapon, WS_READY);
            }
            break;
    }
    
    weaponRecoil *= powf(0.1f, deltaTime * WEAPON_RECOIL_RECOVERY);
    
    weaponSway.x *= 0.9f;
    weaponSway.y *= 0.9f;
}

void G_UpdateAllWeapons(weapon_manager_t* manager, player_t* player, float deltaTime) {
    if (!manager || !player) return;
    
    weapon_t* currentWeapon = G_GetPlayerWeapon(manager, player);
    if (currentWeapon) {
        G_UpdateWeapon(currentWeapon, player, deltaTime);
    }
    
    if (player->isMoving) {
        weaponBobTime += deltaTime * WEAPON_BOB_SPEED;
    } else {
        weaponBobTime *= 0.9f;
    }
}

bool G_CanFireWeapon(weapon_t* weapon, player_t* player) {
    if (!weapon || !player) return false;
    
    if (weapon->state != WS_READY) return false;
    if (weapon->currentClip < weapon->ammoPerShot) {
        if (G_GetWeaponAmmo(player, weapon) > 0) {
            G_ReloadWeapon(weapon, player);
        }
        return false;
    }
    
    return true;
}

void G_FireWeapon(weapon_t* weapon, player_t* player, level_t* level) {
    if (!weapon || !player || !level) return;
    
    if (!G_CanFireWeapon(weapon, player)) return;
    
    weapon->currentClip -= weapon->ammoPerShot;
    weaponRecoil += WEAPON_RECOIL_AMOUNT;
    
    I_Log("Weapon fired: %s (Clip: %d/%d)", weapon->name, weapon->currentClip, weapon->clipSize);
    
    if (weapon->hitscan) {
        G_PerformHitscan(player, level, weapon);
    } else {
        G_SpawnProjectile(player, level, weapon);
    }
    
    G_SetWeaponState(weapon, WS_FIRING);
}

void G_ReloadWeapon(weapon_t* weapon, player_t* player) {
    if (!weapon || !player) return;
    
    if (weapon->state != WS_READY) return;
    if (weapon->currentClip >= weapon->clipSize) return;
    if (G_GetWeaponAmmo(player, weapon) <= 0) return;
    
    G_SetWeaponState(weapon, WS_RELOADING);
    I_Log("Reloading weapon: %s", weapon->name);
}

void G_SetWeaponState(weapon_t* weapon, weapon_state_t state) {
    if (!weapon || weapon->state == state) return;
    
    weapon->state = state;
    weapon->stateTime = 0.0f;
}

float G_GetWeaponBob(weapon_t* weapon, player_t* player) {
    if (!weapon || !player) return 0.0f;
    
    float bob = sinf(weaponBobTime) * WEAPON_BOB_AMOUNT;
    if (player->isMoving) {
        bob *= 1.5f;
    }
    
    return bob;
}

vec2_t G_GetWeaponOffset(weapon_t* weapon, player_t* player) {
    vec2_t offset = {0, 0};
    
    if (!weapon || !player) return offset;
    
    offset.x = weaponSway.x * 100.0f;
    offset.y = weaponSway.y * 100.0f + G_GetWeaponBob(weapon, player) * 50.0f;
    
    offset.y -= weaponRecoil * 100.0f;
    
    switch (weapon->state) {
        case WS_FIRING:
            offset.y -= 20.0f * (1.0f - weapon->stateTime / weapon->fireRate);
            break;
            
        case WS_RELOADING:
            offset.y += 100.0f * sinf(weapon->stateTime / weapon->reloadTime * M_PI);
            break;
            
        case WS_SWITCHING:
            offset.y += 200.0f * (1.0f - weapon->stateTime / weapon->switchTime);
            break;
            
        default:
            break;
    }
    
    return offset;
}

void G_PerformHitscan(player_t* player, level_t* level, weapon_t* weapon) {
    if (!player || !level || !weapon) return;
    
    vec2_t rayStart = player->position;
    float rayAngle = atan2f(player->direction.y, player->direction.x);
    
    float spread = ((float)rand() / RAND_MAX - 0.5f) * weapon->spread;
    rayAngle += spread;
    
    vec2_t rayDir = {cosf(rayAngle), sinf(rayAngle)};
    
    float closestDist = weapon->range;
    entity_t* hitEntity = NULL;
    vec2_t hitPoint = rayStart;
    
    for (int i = 0; i < level->entityCount; i++) {
        entity_t* entity = &level->entities[i];
        if (!entity->active || !entity->solid) continue;
        
        vec2_t toEntity = {entity->position.x - rayStart.x, entity->position.y - rayStart.y};
        float projLength = toEntity.x * rayDir.x + toEntity.y * rayDir.y;
        
        if (projLength < 0 || projLength > closestDist) continue;
        
        vec2_t closestPoint = {
            rayStart.x + rayDir.x * projLength,
            rayStart.y + rayDir.y * projLength
        };
        
        float dist = G_GetDistance(closestPoint, entity->position);
        if (dist <= entity->radius) {
            closestDist = projLength;
            hitEntity = entity;
            hitPoint = closestPoint;
        }
    }
    
    for (float t = 0; t < closestDist; t += 10.0f) {
        vec2_t checkPos = {
            rayStart.x + rayDir.x * t,
            rayStart.y + rayDir.y * t
        };
        
        int tileX = (int)(checkPos.x / TILE_SIZE);
        int tileY = (int)(checkPos.y / TILE_SIZE);
        
        if (G_GetTile(level, tileX, tileY) == TILE_WALL) {
            closestDist = t;
            hitEntity = NULL;
            hitPoint = checkPos;
            break;
        }
    }
    
    if (hitEntity) {
        G_DamageEntity(hitEntity, weapon->damage, rayStart);
        G_SpawnBloodEffect(level, hitPoint);
        I_Log("Hit entity at distance %.2f for %d damage", closestDist, weapon->damage);
    } else if (closestDist < weapon->range) {
        I_Log("Hit wall at distance %.2f", closestDist);
    }
}

void G_SpawnProjectile(player_t* player, level_t* level, weapon_t* weapon) {
    if (!player || !level || !weapon) return;
    
    vec2_t spawnPos = player->position;
    spawnPos.x += player->direction.x * 32.0f;
    spawnPos.y += player->direction.y * 32.0f;
    
    entity_t* projectile = G_SpawnEntity(level, ET_NONE, spawnPos);
    if (!projectile) return;
    
    float angle = atan2f(player->direction.y, player->direction.x);
    float spread = ((float)rand() / RAND_MAX - 0.5f) * weapon->spread;
    angle += spread;
    
    projectile->velocity.x = cosf(angle) * weapon->projectileSpeed;
    projectile->velocity.y = sinf(angle) * weapon->projectileSpeed;
    projectile->damage = weapon->damage;
    projectile->radius = 4.0f;
    projectile->height = 8.0f;
    
    I_Log("Spawned projectile with velocity (%.2f, %.2f)", projectile->velocity.x, projectile->velocity.y);
}

int G_GetWeaponFrame(weapon_t* weapon) {
    if (!weapon) return 0;
    
    switch (weapon->state) {
        case WS_FIRING:
            return 1 + (int)(weapon->stateTime / weapon->fireRate * 3.0f);
            
        case WS_RELOADING:
            return 4 + (int)(weapon->stateTime / weapon->reloadTime * 4.0f);
            
        default:
            return 0;
    }
}

bool G_IsWeaponReady(weapon_t* weapon) {
    return weapon && weapon->state == WS_READY;
}

void G_GiveWeaponAmmo(player_t* player, int weaponId, int amount) {
    if (!player || weaponId < 0 || weaponId >= MAX_WEAPONS) return;
    G_GivePlayerAmmo(player, weaponId, amount);
}

int G_GetWeaponAmmo(player_t* player, weapon_t* weapon) {
    if (!player || !weapon) return 0;
    return player->ammo[player->currentWeapon];
}