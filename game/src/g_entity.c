#include "../include/g_entity.h"
#include "../include/g_player.h"
#include "../include/g_level.h"
#include "../include/g_raycaster.h"
#include "../../engine/include/i_system.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ENTITY_TURN_SPEED 3.0f
#define ENTITY_ATTACK_COOLDOWN 1.0f
#define ENTITY_PAIN_TIME 0.3f
#define ENTITY_DEATH_TIME 2.0f

static const float SOLDIER_SPEED = 100.0f;
static const float DEMON_SPEED = 150.0f;
static const float TURRET_RANGE = 512.0f;

static bool checkEntityCollision(level_t* level, vec2_t position, float radius, entity_t* ignore);
static float normalizeAngle(float angle);

void G_InitEntity(entity_t* entity, entity_type_t type, vec2_t position) {
    if (!entity) return;
    
    memset(entity, 0, sizeof(entity_t));
    
    entity->type = type;
    entity->position = position;
    entity->state = ES_IDLE;
    entity->active = true;
    entity->visible = true;
    
    switch (type) {
        case ET_ENEMY_SOLDIER:
            entity->health = 50;
            entity->maxHealth = 50;
            entity->damage = 10;
            entity->speed = SOLDIER_SPEED;
            entity->sightRange = 384.0f;
            entity->attackRange = 256.0f;
            entity->radius = 20.0f;
            entity->height = 56.0f;
            entity->solid = true;
            break;
            
        case ET_ENEMY_DEMON:
            entity->health = 150;
            entity->maxHealth = 150;
            entity->damage = 20;
            entity->speed = DEMON_SPEED;
            entity->sightRange = 512.0f;
            entity->attackRange = 64.0f;
            entity->radius = 24.0f;
            entity->height = 64.0f;
            entity->solid = true;
            break;
            
        case ET_TURRET:
            entity->health = 200;
            entity->maxHealth = 200;
            entity->damage = 15;
            entity->speed = 0.0f;
            entity->sightRange = TURRET_RANGE;
            entity->attackRange = TURRET_RANGE;
            entity->radius = 32.0f;
            entity->height = 48.0f;
            entity->solid = true;
            break;
            
        case ET_ITEM_AMMO:
            entity->radius = 16.0f;
            entity->height = 32.0f;
            entity->pickupable = true;
            entity->damage = 20;
            break;
            
        case ET_ITEM_HEALTH:
            entity->radius = 16.0f;
            entity->height = 32.0f;
            entity->pickupable = true;
            entity->damage = 25;
            break;
            
        case ET_ITEM_ARMOR:
            entity->radius = 16.0f;
            entity->height = 32.0f;
            entity->pickupable = true;
            entity->damage = 50;
            break;
            
        case ET_ITEM_KEY:
            entity->radius = 16.0f;
            entity->height = 32.0f;
            entity->pickupable = true;
            entity->damage = 1;
            break;
            
        default:
            break;
    }
}

void G_UpdateEntity(entity_t* entity, level_t* level, player_t* player, float deltaTime) {
    if (!entity || !entity->active || !level || !player) return;
    
    switch (entity->type) {
        case ET_ENEMY_SOLDIER:
            G_UpdateEnemySoldier(entity, level, player, deltaTime);
            break;
            
        case ET_ENEMY_DEMON:
            G_UpdateEnemyDemon(entity, level, player, deltaTime);
            break;
            
        case ET_TURRET:
            G_UpdateTurret(entity, level, player, deltaTime);
            break;
            
        case ET_ITEM_AMMO:
        case ET_ITEM_HEALTH:
        case ET_ITEM_ARMOR:
        case ET_ITEM_KEY:
            G_UpdatePickup(entity, level, player, deltaTime);
            break;
            
        default:
            break;
    }
    
    G_UpdateEntityAnimation(entity, deltaTime);
}

void G_UpdateAllEntities(level_t* level, player_t* player, float deltaTime) {
    if (!level || !player) return;
    
    for (int i = 0; i < level->entityCount; i++) {
        G_UpdateEntity(&level->entities[i], level, player, deltaTime);
    }
}

void G_SetEntityState(entity_t* entity, entity_state_t state) {
    if (!entity || entity->state == state) return;
    
    entity->previousState = entity->state;
    entity->state = state;
    entity->stateTime = 0.0f;
    entity->currentFrame = 0;
    entity->frameTime = 0.0f;
    
    I_Log("Entity state changed: %d -> %d", entity->previousState, entity->state);
}

void G_UpdateEntityAnimation(entity_t* entity, float deltaTime) {
    if (!entity) return;
    
    entity->stateTime += deltaTime;
    entity->frameTime += deltaTime;
    
    float frameRate = 0.1f;
    
    switch (entity->state) {
        case ES_WALKING:
            frameRate = 0.1f;
            break;
            
        case ES_ATTACKING:
            frameRate = 0.15f;
            break;
            
        case ES_DYING:
            frameRate = 0.2f;
            break;
            
        default:
            frameRate = 0.2f;
            break;
    }
    
    if (entity->frameTime >= frameRate) {
        entity->frameTime = 0.0f;
        entity->currentFrame++;
        
        int maxFrames = 4;
        switch (entity->state) {
            case ES_ATTACKING:
                maxFrames = 2;
                break;
                
            case ES_DYING:
                maxFrames = 5;
                break;
                
            default:
                break;
        }
        
        if (entity->currentFrame >= maxFrames) {
            if (entity->state == ES_DYING) {
                G_SetEntityState(entity, ES_DEAD);
            } else if (entity->state == ES_ATTACKING) {
                G_SetEntityState(entity, ES_IDLE);
            } else {
                entity->currentFrame = 0;
            }
        }
    }
}

int G_GetEntityFrame(entity_t* entity, player_t* player) {
    if (!entity || !player) return 0;
    
    if (entity->state == ES_DEAD) return 0;
    
    float dx = entity->position.x - player->position.x;
    float dy = entity->position.y - player->position.y;
    float angle = atan2f(dy, dx) - entity->angle;
    
    angle = normalizeAngle(angle);
    
    int spriteAngle = (int)((angle + M_PI) * 8.0f / (2.0f * M_PI) + 0.5f) & 7;
    
    return spriteAngle * 10 + entity->currentFrame;
}

bool G_CanEntitySeePlayer(entity_t* entity, level_t* level, player_t* player) {
    if (!entity || !level || !player) return false;
    
    float dx = player->position.x - entity->position.x;
    float dy = player->position.y - entity->position.y;
    float distSq = dx * dx + dy * dy;
    
    if (distSq > entity->sightRange * entity->sightRange) return false;
    
    float angle = atan2f(dy, dx);
    float angleDiff = normalizeAngle(angle - entity->angle);
    
    if (entity->type != ET_TURRET && fabsf(angleDiff) > M_PI * 0.75f) return false;
    
    return G_CheckLineOfSight(level, entity->position, player->position);
}

bool G_IsEntityInRange(entity_t* entity, vec2_t target, float range) {
    if (!entity) return false;
    
    float dx = target.x - entity->position.x;
    float dy = target.y - entity->position.y;
    float distSq = dx * dx + dy * dy;
    
    return distSq <= range * range;
}

void G_MoveEntityToward(entity_t* entity, level_t* level, vec2_t target, float deltaTime) {
    if (!entity || !level || entity->speed <= 0) return;
    
    float dx = target.x - entity->position.x;
    float dy = target.y - entity->position.y;
    float dist = sqrtf(dx * dx + dy * dy);
    
    if (dist < 1.0f) return;
    
    dx /= dist;
    dy /= dist;
    
    float moveX = dx * entity->speed * deltaTime;
    float moveY = dy * entity->speed * deltaTime;
    
    vec2_t newPos = entity->position;
    
    newPos.x += moveX;
    if (!checkEntityCollision(level, newPos, entity->radius, entity)) {
        entity->position.x = newPos.x;
    } else {
        newPos.x = entity->position.x;
    }
    
    newPos.y += moveY;
    if (!checkEntityCollision(level, newPos, entity->radius, entity)) {
        entity->position.y = newPos.y;
    }
    
    entity->velocity.x = (entity->position.x - entity->position.x) / deltaTime;
    entity->velocity.y = (entity->position.y - entity->position.y) / deltaTime;
}

void G_RotateEntityToward(entity_t* entity, vec2_t target, float deltaTime) {
    if (!entity) return;
    
    float dx = target.x - entity->position.x;
    float dy = target.y - entity->position.y;
    float targetAngle = atan2f(dy, dx);
    
    float angleDiff = normalizeAngle(targetAngle - entity->angle);
    
    if (fabsf(angleDiff) < 0.1f) {
        entity->angle = targetAngle;
        return;
    }
    
    float turnSpeed = ENTITY_TURN_SPEED * deltaTime;
    
    if (angleDiff > 0) {
        entity->angle += fminf(turnSpeed, angleDiff);
    } else {
        entity->angle -= fminf(turnSpeed, -angleDiff);
    }
    
    entity->angle = normalizeAngle(entity->angle);
}

void G_DamageEntity(entity_t* entity, int damage, vec2_t damageFrom) {
    if (!entity || !entity->active || entity->state == ES_DYING || entity->state == ES_DEAD) return;
    
    entity->health -= damage;
    
    if (entity->health <= 0) {
        G_KillEntity(entity);
    } else {
        G_SetEntityState(entity, ES_HURT);
        entity->target = (void*)1;
        entity->lastKnownTargetPos = damageFrom;
        entity->lastSeenTime = 0.0f;
        
        I_Log("Entity damaged: %d HP remaining", entity->health);
    }
}

void G_KillEntity(entity_t* entity) {
    if (!entity || entity->state == ES_DEAD) return;
    
    entity->health = 0;
    entity->solid = false;
    G_SetEntityState(entity, ES_DYING);
    
    I_Log("Entity killed");
}

bool G_IsEntityDead(entity_t* entity) {
    return !entity || entity->state == ES_DEAD || entity->health <= 0;
}

void G_EntityAttack(entity_t* entity, player_t* player, level_t* level) {
    if (!entity || !player || !level || entity->state != ES_IDLE) return;
    
    G_SetEntityState(entity, ES_ATTACKING);
    
    if (G_IsEntityInRange(entity, player->position, entity->attackRange)) {
        if (entity->type == ET_ENEMY_DEMON) {
            if (G_IsEntityInRange(entity, player->position, 64.0f)) {
                G_DamagePlayer(player, entity->damage);
            }
        } else {
            if (G_CheckLineOfSight(level, entity->position, player->position)) {
                G_DamagePlayer(player, entity->damage);
            }
        }
    }
}

void G_EntityPickup(entity_t* entity, player_t* player) {
    if (!entity || !player || !entity->pickupable) return;
    
    switch (entity->type) {
        case ET_ITEM_AMMO:
            G_GivePlayerAmmo(player, player->currentWeapon, entity->damage);
            break;
            
        case ET_ITEM_HEALTH:
            G_HealPlayer(player, entity->damage);
            break;
            
        case ET_ITEM_ARMOR:
            G_GivePlayerArmor(player, entity->damage);
            break;
            
        case ET_ITEM_KEY:
            G_GivePlayerKey(player, entity->damage - 1);
            break;
            
        default:
            break;
    }
    
    entity->active = false;
    entity->visible = false;
}

void G_UpdateEnemySoldier(entity_t* entity, level_t* level, player_t* player, float deltaTime) {
    if (!entity || !level || !player) return;
    
    switch (entity->state) {
        case ES_IDLE:
            if (G_CanEntitySeePlayer(entity, level, player)) {
                entity->target = player;
                entity->lastKnownTargetPos = player->position;
                entity->lastSeenTime = 0.0f;
                G_SetEntityState(entity, ES_WALKING);
            }
            break;
            
        case ES_WALKING:
            if (entity->target) {
                if (G_CanEntitySeePlayer(entity, level, player)) {
                    entity->lastKnownTargetPos = player->position;
                    entity->lastSeenTime = 0.0f;
                    
                    if (G_IsEntityInRange(entity, player->position, entity->attackRange)) {
                        G_EntityAttack(entity, player, level);
                    } else {
                        G_RotateEntityToward(entity, player->position, deltaTime);
                        G_MoveEntityToward(entity, level, player->position, deltaTime);
                    }
                } else {
                    entity->lastSeenTime += deltaTime;
                    
                    if (entity->lastSeenTime < 3.0f) {
                        G_RotateEntityToward(entity, entity->lastKnownTargetPos, deltaTime);
                        G_MoveEntityToward(entity, level, entity->lastKnownTargetPos, deltaTime);
                    } else {
                        entity->target = NULL;
                        G_SetEntityState(entity, ES_IDLE);
                    }
                }
            }
            break;
            
        case ES_ATTACKING:
            if (entity->stateTime >= ENTITY_ATTACK_COOLDOWN) {
                G_SetEntityState(entity, ES_WALKING);
            }
            break;
            
        case ES_HURT:
            if (entity->stateTime >= ENTITY_PAIN_TIME) {
                G_SetEntityState(entity, ES_WALKING);
            }
            break;
            
        case ES_DYING:
            break;
            
        case ES_DEAD:
            break;
    }
}

void G_UpdateEnemyDemon(entity_t* entity, level_t* level, player_t* player, float deltaTime) {
    if (!entity || !level || !player) return;
    
    switch (entity->state) {
        case ES_IDLE:
            if (G_CanEntitySeePlayer(entity, level, player)) {
                entity->target = player;
                entity->lastKnownTargetPos = player->position;
                entity->lastSeenTime = 0.0f;
                G_SetEntityState(entity, ES_WALKING);
            }
            break;
            
        case ES_WALKING:
            if (entity->target) {
                if (G_CanEntitySeePlayer(entity, level, player)) {
                    entity->lastKnownTargetPos = player->position;
                    entity->lastSeenTime = 0.0f;
                    
                    if (G_IsEntityInRange(entity, player->position, entity->attackRange)) {
                        G_EntityAttack(entity, player, level);
                    } else {
                        G_RotateEntityToward(entity, player->position, deltaTime);
                        G_MoveEntityToward(entity, level, player->position, deltaTime);
                    }
                } else {
                    entity->lastSeenTime += deltaTime;
                    
                    if (entity->lastSeenTime < 5.0f) {
                        G_RotateEntityToward(entity, entity->lastKnownTargetPos, deltaTime);
                        G_MoveEntityToward(entity, level, entity->lastKnownTargetPos, deltaTime);
                    } else {
                        entity->target = NULL;
                        G_SetEntityState(entity, ES_IDLE);
                    }
                }
            }
            break;
            
        case ES_ATTACKING:
            if (entity->stateTime >= ENTITY_ATTACK_COOLDOWN * 0.5f) {
                G_SetEntityState(entity, ES_WALKING);
            }
            break;
            
        case ES_HURT:
            if (entity->stateTime >= ENTITY_PAIN_TIME) {
                G_SetEntityState(entity, ES_WALKING);
            }
            break;
            
        case ES_DYING:
            break;
            
        case ES_DEAD:
            break;
    }
}

void G_UpdateTurret(entity_t* entity, level_t* level, player_t* player, float deltaTime) {
    if (!entity || !level || !player) return;
    
    switch (entity->state) {
        case ES_IDLE:
            if (G_CanEntitySeePlayer(entity, level, player)) {
                entity->target = player;
                G_SetEntityState(entity, ES_ATTACKING);
            }
            break;
            
        case ES_ATTACKING:
            if (G_CanEntitySeePlayer(entity, level, player)) {
                G_RotateEntityToward(entity, player->position, deltaTime * 2.0f);
                
                if (entity->stateTime >= ENTITY_ATTACK_COOLDOWN * 1.5f) {
                    if (G_CheckLineOfSight(level, entity->position, player->position)) {
                        G_DamagePlayer(player, entity->damage);
                    }
                    entity->stateTime = 0.0f;
                }
            } else {
                entity->target = NULL;
                G_SetEntityState(entity, ES_IDLE);
            }
            break;
            
        case ES_HURT:
            if (entity->stateTime >= ENTITY_PAIN_TIME) {
                G_SetEntityState(entity, ES_ATTACKING);
            }
            break;
            
        case ES_DYING:
            break;
            
        case ES_DEAD:
            break;
            
        default:
            break;
    }
}

void G_UpdatePickup(entity_t* entity, level_t* level, player_t* player, float deltaTime) {
    if (!entity || !level || !player || !entity->pickupable) return;
    
    if (G_IsEntityInRange(entity, player->position, entity->radius + PLAYER_RADIUS)) {
        G_EntityPickup(entity, player);
    }
    
    entity->angle += deltaTime * 2.0f;
    entity->angle = normalizeAngle(entity->angle);
}

void G_SpawnBloodEffect(level_t* level, vec2_t position) {
    if (!level) return;
    
    I_Log("Blood effect at (%.2f, %.2f)", position.x, position.y);
}

void G_SpawnExplosion(level_t* level, vec2_t position, float radius, int damage) {
    if (!level) return;
    
    I_Log("Explosion at (%.2f, %.2f) radius %.2f damage %d", position.x, position.y, radius, damage);
}

int G_GetEntityTextureIndex(entity_t* entity, int angle, int frame) {
    if (!entity) return 0;
    
    return (entity->type * 100) + (angle * 10) + frame;
}

rgba_t G_GetEntityTint(entity_t* entity) {
    rgba_t tint = {255, 255, 255, 255};
    
    if (!entity) return tint;
    
    if (entity->state == ES_HURT && entity->stateTime < 0.1f) {
        tint.r = 255;
        tint.g = 128;
        tint.b = 128;
    }
    
    return tint;
}

static bool checkEntityCollision(level_t* level, vec2_t position, float radius, entity_t* ignore) {
    if (!level) return true;
    
    if (G_IsPositionSolid(level, position.x, position.y, radius)) {
        return true;
    }
    
    for (int i = 0; i < level->entityCount; i++) {
        entity_t* other = &level->entities[i];
        if (other == ignore || !other->active || !other->solid) continue;
        
        float dx = position.x - other->position.x;
        float dy = position.y - other->position.y;
        float distSq = dx * dx + dy * dy;
        float minDist = radius + other->radius;
        
        if (distSq < minDist * minDist) {
            return true;
        }
    }
    
    return false;
}

static float normalizeAngle(float angle) {
    while (angle > M_PI) angle -= 2.0f * M_PI;
    while (angle < -M_PI) angle += 2.0f * M_PI;
    return angle;
}