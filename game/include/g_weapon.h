#ifndef G_WEAPON_H
#define G_WEAPON_H

#include "g_defs.h"
#include "../../engine/include/i_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    weapon_t weapons[MAX_WEAPONS];
    int weaponCount;
    bool loaded;
} weapon_manager_t;

weapon_manager_t* G_CreateWeaponManager(void);
void G_DestroyWeaponManager(weapon_manager_t* manager);

bool G_LoadWeapons(weapon_manager_t* manager, const char* dataPath);
bool G_LoadWeaponData(weapon_t* weapon, ini_file_t config);

weapon_t* G_GetWeapon(weapon_manager_t* manager, int weaponId);
weapon_t* G_GetPlayerWeapon(weapon_manager_t* manager, player_t* player);

void G_UpdateWeapon(weapon_t* weapon, player_t* player, float deltaTime);
void G_UpdateAllWeapons(weapon_manager_t* manager, player_t* player, float deltaTime);

bool G_CanFireWeapon(weapon_t* weapon, player_t* player);
void G_FireWeapon(weapon_t* weapon, player_t* player, level_t* level);
void G_ReloadWeapon(weapon_t* weapon, player_t* player);

void G_SetWeaponState(weapon_t* weapon, weapon_state_t state);
float G_GetWeaponBob(weapon_t* weapon, player_t* player);
vec2_t G_GetWeaponOffset(weapon_t* weapon, player_t* player);

void G_PerformHitscan(player_t* player, level_t* level, weapon_t* weapon);
void G_SpawnProjectile(player_t* player, level_t* level, weapon_t* weapon);

int G_GetWeaponFrame(weapon_t* weapon);
bool G_IsWeaponReady(weapon_t* weapon);

void G_GiveWeaponAmmo(player_t* player, int weaponId, int amount);
int G_GetWeaponAmmo(player_t* player, weapon_t* weapon);

#ifdef __cplusplus
}
#endif

#endif