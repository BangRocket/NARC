#ifndef G_ENTITY_H
#define G_ENTITY_H

#include "g_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

void G_InitEntity(entity_t* entity, entity_type_t type, vec2_t position);
void G_UpdateEntity(entity_t* entity, level_t* level, player_t* player, float deltaTime);
void G_UpdateAllEntities(level_t* level, player_t* player, float deltaTime);

void G_SetEntityState(entity_t* entity, entity_state_t state);
void G_UpdateEntityAnimation(entity_t* entity, float deltaTime);
int G_GetEntityFrame(entity_t* entity, player_t* player);

bool G_CanEntitySeePlayer(entity_t* entity, level_t* level, player_t* player);
bool G_IsEntityInRange(entity_t* entity, vec2_t target, float range);
void G_MoveEntityToward(entity_t* entity, level_t* level, vec2_t target, float deltaTime);
void G_RotateEntityToward(entity_t* entity, vec2_t target, float deltaTime);

void G_DamageEntity(entity_t* entity, int damage, vec2_t damageFrom);
void G_KillEntity(entity_t* entity);
bool G_IsEntityDead(entity_t* entity);

void G_EntityAttack(entity_t* entity, player_t* player, level_t* level);
void G_EntityPickup(entity_t* entity, player_t* player);

void G_UpdateEnemySoldier(entity_t* entity, level_t* level, player_t* player, float deltaTime);
void G_UpdateEnemyDemon(entity_t* entity, level_t* level, player_t* player, float deltaTime);
void G_UpdateTurret(entity_t* entity, level_t* level, player_t* player, float deltaTime);
void G_UpdatePickup(entity_t* entity, level_t* level, player_t* player, float deltaTime);

void G_SpawnBloodEffect(level_t* level, vec2_t position);
void G_SpawnExplosion(level_t* level, vec2_t position, float radius, int damage);

int G_GetEntityTextureIndex(entity_t* entity, int angle, int frame);
rgba_t G_GetEntityTint(entity_t* entity);

#ifdef __cplusplus
}
#endif

#endif