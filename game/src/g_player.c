#include "../include/g_player.h"
#include "../../engine/include/i_system.h"
#include "../../engine/include/i_input.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PLAYER_ACCELERATION 10.0f
#define PLAYER_FRICTION 8.0f
#define PLAYER_MAX_PITCH 0.5f
#define PLAYER_BOB_AMOUNT 0.02f
#define PLAYER_BOB_SPEED 5.0f
#define COLLISION_MARGIN 0.1f

player_t* G_CreatePlayer(void) {
    player_t* player = (player_t*)I_Calloc(1, sizeof(player_t));
    
    player->direction.x = 1.0f;
    player->direction.y = 0.0f;
    player->plane.x = 0.0f;
    player->plane.y = 0.66f;
    
    player->height = PLAYER_HEIGHT;
    player->health = 100;
    player->maxHealth = 100;
    player->armor = 0;
    player->maxArmor = 100;
    
    player->moveSpeed = PLAYER_SPEED;
    player->turnSpeed = PLAYER_TURN_SPEED;
    
    for (int i = 0; i < MAX_WEAPONS; i++) {
        player->maxAmmo[i] = 200;
    }
    
    I_Log("Player created");
    
    return player;
}

void G_DestroyPlayer(player_t* player) {
    if (player) {
        I_Free(player);
        I_Log("Player destroyed");
    }
}

void G_InitPlayer(player_t* player, vec2_t position, float angle) {
    if (!player) return;
    
    player->position = position;
    player->direction.x = cosf(angle);
    player->direction.y = sinf(angle);
    
    float planeAngle = angle - M_PI / 2.0f;
    player->plane.x = cosf(planeAngle) * 0.66f;
    player->plane.y = sinf(planeAngle) * 0.66f;
    
    player->pitch = 0.0f;
    player->bob = 0.0f;
    player->bobTime = 0.0f;
    
    player->health = player->maxHealth;
    player->armor = 0;
    player->currentWeapon = 0;
    player->score = 0;
    player->secrets = 0;
    
    player->isMoving = false;
    player->isShooting = false;
    player->isDead = false;
    
    memset(player->keys, 0, sizeof(player->keys));
    memset(player->hasWeapon, 0, sizeof(player->hasWeapon));
    memset(player->ammo, 0, sizeof(player->ammo));
    
    player->hasWeapon[0] = true;
    player->ammo[0] = 50;
    
    I_Log("Player initialized at (%.2f, %.2f) angle %.2f", position.x, position.y, angle);
}

void G_ResetPlayer(player_t* player) {
    if (!player) return;
    
    vec2_t pos = player->position;
    float angle = atan2f(player->direction.y, player->direction.x);
    G_InitPlayer(player, pos, angle);
}

void G_UpdatePlayer(player_t* player, level_t* level, float deltaTime) {
    if (!player || !level || player->isDead) return;
    
    G_ProcessPlayerInput(player, deltaTime);
    
    G_UpdatePlayerBob(player, deltaTime);
    
    if (player->health <= 0 && !player->isDead) {
        player->isDead = true;
        player->health = 0;
        I_Log("Player died!");
    }
}

void G_ProcessPlayerInput(player_t* player, float deltaTime) {
    if (!player || player->isDead) return;
    
    float forward = 0.0f;
    float strafe = 0.0f;
    
    if (I_IsKeyDown(KEY_W) || I_IsKeyDown(KEY_UP)) forward += 1.0f;
    if (I_IsKeyDown(KEY_S) || I_IsKeyDown(KEY_DOWN)) forward -= 1.0f;
    if (I_IsKeyDown(KEY_A)) strafe -= 1.0f;
    if (I_IsKeyDown(KEY_D)) strafe += 1.0f;
    
    if (I_IsKeyDown(KEY_LEFT_SHIFT)) {
        forward *= 1.5f;
        strafe *= 1.5f;
    }
    
    player->isMoving = (forward != 0.0f || strafe != 0.0f);
    
    if (I_IsKeyDown(KEY_LEFT)) {
        G_RotatePlayer(player, player->turnSpeed * deltaTime, 0);
    }
    if (I_IsKeyDown(KEY_RIGHT)) {
        G_RotatePlayer(player, -player->turnSpeed * deltaTime, 0);
    }
    
    int mouseX, mouseY;
    I_GetMouseDelta(&mouseX, &mouseY);
    if (mouseX != 0 || mouseY != 0) {
        G_LookPlayer(player, mouseX, mouseY, 1.0f);
    }
    
    if (I_IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || I_IsKeyPressed(KEY_SPACE)) {
        player->isShooting = true;
    }
    
    for (int i = 0; i < 9; i++) {
        if (I_IsKeyPressed(KEY_ONE + i)) {
            G_SwitchPlayerWeapon(player, i);
        }
    }
}

void G_MovePlayer(player_t* player, level_t* level, float forward, float strafe, float deltaTime) {
    if (!player || !level) return;
    
    float moveX = (player->direction.x * forward - player->direction.y * strafe) * player->moveSpeed * deltaTime;
    float moveY = (player->direction.y * forward + player->direction.x * strafe) * player->moveSpeed * deltaTime;
    
    vec2_t newPos = player->position;
    
    newPos.x += moveX;
    if (!G_CheckPlayerCollision(player, level, newPos)) {
        player->position.x = newPos.x;
    } else {
        newPos.x = player->position.x;
    }
    
    newPos.y += moveY;
    if (!G_CheckPlayerCollision(player, level, newPos)) {
        player->position.y = newPos.y;
    }
}

void G_RotatePlayer(player_t* player, float angle, float pitch) {
    if (!player) return;
    
    float oldDirX = player->direction.x;
    player->direction.x = player->direction.x * cosf(angle) - player->direction.y * sinf(angle);
    player->direction.y = oldDirX * sinf(angle) + player->direction.y * cosf(angle);
    
    float oldPlaneX = player->plane.x;
    player->plane.x = player->plane.x * cosf(angle) - player->plane.y * sinf(angle);
    player->plane.y = oldPlaneX * sinf(angle) + player->plane.y * cosf(angle);
    
    player->pitch += pitch;
    if (player->pitch > PLAYER_MAX_PITCH) player->pitch = PLAYER_MAX_PITCH;
    if (player->pitch < -PLAYER_MAX_PITCH) player->pitch = -PLAYER_MAX_PITCH;
}

void G_LookPlayer(player_t* player, int deltaX, int deltaY, float sensitivity) {
    if (!player) return;
    
    float rotateAngle = -deltaX * sensitivity * 0.002f;
    float pitchAngle = deltaY * sensitivity * 0.002f;
    
    G_RotatePlayer(player, rotateAngle, pitchAngle);
}

bool G_CheckPlayerCollision(player_t* player, level_t* level, vec2_t newPos) {
    if (!player || !level) return true;
    
    return G_IsPositionSolid(level, newPos.x, newPos.y, PLAYER_RADIUS);
}

bool G_IsPositionSolid(level_t* level, float x, float y, float radius) {
    if (!level) return true;
    
    int minX = (int)((x - radius - COLLISION_MARGIN) / TILE_SIZE);
    int maxX = (int)((x + radius + COLLISION_MARGIN) / TILE_SIZE);
    int minY = (int)((y - radius - COLLISION_MARGIN) / TILE_SIZE);
    int maxY = (int)((y + radius + COLLISION_MARGIN) / TILE_SIZE);
    
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= level->width) maxX = level->width - 1;
    if (maxY >= level->height) maxY = level->height - 1;
    
    for (int checkY = minY; checkY <= maxY; checkY++) {
        for (int checkX = minX; checkX <= maxX; checkX++) {
            uint8_t tile = level->tiles[checkY * level->width + checkX];
            
            if (tile == TILE_WALL || tile == TILE_DOOR) {
                float tileLeft = checkX * TILE_SIZE;
                float tileRight = (checkX + 1) * TILE_SIZE;
                float tileTop = checkY * TILE_SIZE;
                float tileBottom = (checkY + 1) * TILE_SIZE;
                
                float closestX = x;
                if (x < tileLeft) closestX = tileLeft;
                else if (x > tileRight) closestX = tileRight;
                
                float closestY = y;
                if (y < tileTop) closestY = tileTop;
                else if (y > tileBottom) closestY = tileBottom;
                
                float distX = x - closestX;
                float distY = y - closestY;
                float distSq = distX * distX + distY * distY;
                
                if (distSq < radius * radius) {
                    return true;
                }
            }
        }
    }
    
    for (int i = 0; i < level->entityCount; i++) {
        entity_t* entity = &level->entities[i];
        if (!entity->active || !entity->solid) continue;
        
        float dx = x - entity->position.x;
        float dy = y - entity->position.y;
        float distSq = dx * dx + dy * dy;
        float minDist = radius + entity->radius;
        
        if (distSq < minDist * minDist) {
            return true;
        }
    }
    
    return false;
}

void G_DamagePlayer(player_t* player, int damage) {
    if (!player || player->isDead) return;
    
    if (player->armor > 0) {
        int armorAbsorb = damage / 2;
        if (armorAbsorb > player->armor) {
            armorAbsorb = player->armor;
        }
        player->armor -= armorAbsorb;
        damage -= armorAbsorb;
    }
    
    player->health -= damage;
    if (player->health < 0) player->health = 0;
    
    I_Log("Player damaged: %d (Health: %d, Armor: %d)", damage, player->health, player->armor);
}

void G_HealPlayer(player_t* player, int amount) {
    if (!player || player->isDead) return;
    
    player->health += amount;
    if (player->health > player->maxHealth) {
        player->health = player->maxHealth;
    }
    
    I_Log("Player healed: %d (Health: %d)", amount, player->health);
}

void G_GivePlayerArmor(player_t* player, int amount) {
    if (!player) return;
    
    player->armor += amount;
    if (player->armor > player->maxArmor) {
        player->armor = player->maxArmor;
    }
    
    I_Log("Player got armor: %d (Armor: %d)", amount, player->armor);
}

void G_GivePlayerWeapon(player_t* player, int weaponId) {
    if (!player || weaponId < 0 || weaponId >= MAX_WEAPONS) return;
    
    player->hasWeapon[weaponId] = true;
    
    if (player->ammo[weaponId] < player->maxAmmo[weaponId] / 4) {
        player->ammo[weaponId] = player->maxAmmo[weaponId] / 4;
    }
    
    I_Log("Player got weapon: %d", weaponId);
}

void G_GivePlayerAmmo(player_t* player, int weaponId, int amount) {
    if (!player || weaponId < 0 || weaponId >= MAX_WEAPONS) return;
    
    player->ammo[weaponId] += amount;
    if (player->ammo[weaponId] > player->maxAmmo[weaponId]) {
        player->ammo[weaponId] = player->maxAmmo[weaponId];
    }
    
    I_Log("Player got ammo: %d for weapon %d (Total: %d)", amount, weaponId, player->ammo[weaponId]);
}

void G_SwitchPlayerWeapon(player_t* player, int weaponId) {
    if (!player || weaponId < 0 || weaponId >= MAX_WEAPONS) return;
    if (!player->hasWeapon[weaponId]) return;
    if (player->currentWeapon == weaponId) return;
    
    player->currentWeapon = weaponId;
    I_Log("Player switched to weapon: %d", weaponId);
}

void G_PlayerShoot(player_t* player, level_t* level) {
    if (!player || !level || player->isDead) return;
    
    if (player->ammo[player->currentWeapon] <= 0) {
        I_Log("No ammo for weapon %d", player->currentWeapon);
        return;
    }
    
    player->ammo[player->currentWeapon]--;
    player->isShooting = false;
    
    I_Log("Player shot weapon %d (Ammo: %d)", player->currentWeapon, player->ammo[player->currentWeapon]);
}

void G_GivePlayerKey(player_t* player, int keyType) {
    if (!player || keyType < 0 || keyType >= 4) return;
    
    player->keys[keyType] = true;
    I_Log("Player got key: %d", keyType);
}

bool G_PlayerHasKey(player_t* player, int keyType) {
    if (!player || keyType < 0 || keyType >= 4) return false;
    return player->keys[keyType];
}

void G_AddPlayerScore(player_t* player, int points) {
    if (!player) return;
    
    player->score += points;
    I_Log("Player score: %d (+%d)", player->score, points);
}

void G_PlayerFoundSecret(player_t* player) {
    if (!player) return;
    
    player->secrets++;
    G_AddPlayerScore(player, 100);
    I_Log("Player found secret #%d", player->secrets);
}

void G_UpdatePlayerBob(player_t* player, float deltaTime) {
    if (!player) return;
    
    if (player->isMoving && !player->isDead) {
        player->bobTime += deltaTime * PLAYER_BOB_SPEED;
        player->bob = sinf(player->bobTime) * PLAYER_BOB_AMOUNT;
    } else {
        player->bobTime *= 0.9f;
        player->bob *= 0.9f;
    }
}

vec2_t G_GetPlayerViewOffset(player_t* player) {
    vec2_t offset = {0, 0};
    if (!player) return offset;
    
    offset.y = player->bob;
    return offset;
}

void G_SavePlayerState(player_t* player, void* buffer) {
    if (!player || !buffer) return;
    memcpy(buffer, player, sizeof(player_t));
}

void G_LoadPlayerState(player_t* player, const void* buffer) {
    if (!player || !buffer) return;
    memcpy(player, buffer, sizeof(player_t));
}