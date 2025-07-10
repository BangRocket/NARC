#ifndef G_PLAYER_H
#define G_PLAYER_H

#include "g_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

player_t* G_CreatePlayer(void);
void G_DestroyPlayer(player_t* player);

void G_InitPlayer(player_t* player, vec2_t position, float angle);
void G_ResetPlayer(player_t* player);

void G_UpdatePlayer(player_t* player, level_t* level, float deltaTime);
void G_ProcessPlayerInput(player_t* player, float deltaTime);

void G_MovePlayer(player_t* player, level_t* level, float forward, float strafe, float deltaTime);
void G_RotatePlayer(player_t* player, float angle, float pitch);
void G_LookPlayer(player_t* player, int deltaX, int deltaY, float sensitivity);

bool G_CheckPlayerCollision(player_t* player, level_t* level, vec2_t newPos);
bool G_IsPositionSolid(level_t* level, float x, float y, float radius);

void G_DamagePlayer(player_t* player, int damage);
void G_HealPlayer(player_t* player, int amount);
void G_GivePlayerArmor(player_t* player, int amount);

void G_GivePlayerWeapon(player_t* player, int weaponId);
void G_GivePlayerAmmo(player_t* player, int weaponId, int amount);
void G_SwitchPlayerWeapon(player_t* player, int weaponId);
void G_PlayerShoot(player_t* player, level_t* level);

void G_GivePlayerKey(player_t* player, int keyType);
bool G_PlayerHasKey(player_t* player, int keyType);

void G_AddPlayerScore(player_t* player, int points);
void G_PlayerFoundSecret(player_t* player);

void G_UpdatePlayerBob(player_t* player, float deltaTime);
vec2_t G_GetPlayerViewOffset(player_t* player);

void G_SavePlayerState(player_t* player, void* buffer);
void G_LoadPlayerState(player_t* player, const void* buffer);

#ifdef __cplusplus
}
#endif

#endif