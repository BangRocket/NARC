#include "../include/g_raycaster.h"
#include "../../engine/include/i_system.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void sortSprites(raycaster_t* raycaster, int count);

raycaster_t* G_CreateRaycaster(int width, int height) {
    raycaster_t* raycaster = (raycaster_t*)I_Calloc(1, sizeof(raycaster_t));
    
    raycaster->width = width;
    raycaster->height = height;
    
    raycaster->framebuffer = (uint32_t*)I_Calloc(width * height, sizeof(uint32_t));
    raycaster->zBuffer = (float*)I_Malloc(width * sizeof(float));
    
    raycaster->rayDirX = (float*)I_Malloc(width * sizeof(float));
    raycaster->rayDirY = (float*)I_Malloc(width * sizeof(float));
    
    raycaster->spriteOrder = (int*)I_Malloc(MAX_ENTITIES * sizeof(int));
    raycaster->spriteDistance = (float*)I_Malloc(MAX_ENTITIES * sizeof(float));
    
    raycaster->initialized = true;
    
    I_Log("Raycaster created: %dx%d", width, height);
    
    return raycaster;
}

void G_DestroyRaycaster(raycaster_t* raycaster) {
    if (!raycaster) return;
    
    I_Free(raycaster->framebuffer);
    I_Free(raycaster->zBuffer);
    I_Free(raycaster->rayDirX);
    I_Free(raycaster->rayDirY);
    I_Free(raycaster->spriteOrder);
    I_Free(raycaster->spriteDistance);
    
    I_Free(raycaster);
    
    I_Log("Raycaster destroyed");
}

void G_SetRaycasterPlayer(raycaster_t* raycaster, player_t* player) {
    if (raycaster) raycaster->player = player;
}

void G_SetRaycasterLevel(raycaster_t* raycaster, level_t* level) {
    if (raycaster) raycaster->level = level;
}

void G_SetRaycasterTextures(raycaster_t* raycaster, raycaster_textures_t* textures) {
    if (raycaster) raycaster->textures = textures;
}

void G_SetRaycasterConfig(raycaster_t* raycaster, render_config_t* config) {
    if (raycaster) raycaster->config = config;
}

void G_RenderFrame(raycaster_t* raycaster) {
    if (!raycaster || !raycaster->initialized) return;
    if (!raycaster->player || !raycaster->level) return;
    
    memset(raycaster->framebuffer, 0, raycaster->width * raycaster->height * sizeof(uint32_t));
    
    for (int x = 0; x < raycaster->width; x++) {
        raycaster->zBuffer[x] = FLT_MAX;
    }
    
    float fov = raycaster->config ? raycaster->config->fov : 66;
    float fovRad = fov * M_PI / 180.0f;
    
    for (int x = 0; x < raycaster->width; x++) {
        float cameraX = 2.0f * x / (float)raycaster->width - 1.0f;
        raycaster->rayDirX[x] = raycaster->player->direction.x + raycaster->player->plane.x * cameraX;
        raycaster->rayDirY[x] = raycaster->player->direction.y + raycaster->player->plane.y * cameraX;
    }
    
    if (raycaster->config && raycaster->config->useShadows && raycaster->textures && raycaster->textures->skyboxTexture) {
        G_RenderSkybox(raycaster);
    }
    
    G_RenderWalls(raycaster);
    
    if (raycaster->config && raycaster->config->useFloorCeiling) {
        G_RenderFloorCeiling(raycaster);
    }
    
    G_RenderEntities(raycaster);
    
    G_RenderWeapon(raycaster);
}

void G_RenderWalls(raycaster_t* raycaster) {
    if (!raycaster || !raycaster->level || !raycaster->player) return;
    
    for (int x = 0; x < raycaster->width; x++) {
        float perpWallDist;
        int mapX, mapY, side;
        float wallX;
        
        G_CastRay(raycaster, x, &perpWallDist, &mapX, &mapY, &side, &wallX);
        
        if (perpWallDist <= 0) continue;
        
        raycaster->zBuffer[x] = perpWallDist;
        
        int lineHeight = (int)(raycaster->height / perpWallDist);
        int pitch = (int)(raycaster->player->pitch * raycaster->height);
        
        int drawStart = -lineHeight / 2 + raycaster->height / 2 + pitch;
        if (drawStart < 0) drawStart = 0;
        
        int drawEnd = lineHeight / 2 + raycaster->height / 2 + pitch;
        if (drawEnd >= raycaster->height) drawEnd = raycaster->height - 1;
        
        uint8_t tileValue = raycaster->level->tiles[mapY * raycaster->level->width + mapX];
        
        if (tileValue == TILE_WALL && raycaster->textures && raycaster->config && raycaster->config->useTextures) {
            int texIndex = 0;
            if (raycaster->level->wallTextures[side]) {
                texIndex = raycaster->level->wallTextures[side][mapY * raycaster->level->width + mapX];
            }
            
            if (texIndex > 0 && texIndex <= raycaster->textures->wallTextureCount) {
                texture_t tex = raycaster->textures->wallTextures[texIndex - 1];
                if (tex) {
                    int texWidth, texHeight;
                    R_GetTextureSize(tex, &texWidth, &texHeight);
                    
                    int texX = (int)(wallX * texWidth);
                    if ((side == 0 && raycaster->rayDirX[x] > 0) || (side == 1 && raycaster->rayDirY[x] < 0)) {
                        texX = texWidth - texX - 1;
                    }
                    
                    for (int y = drawStart; y <= drawEnd; y++) {
                        int d = (y - pitch) * 256 - raycaster->height * 128 + lineHeight * 128;
                        int texY = ((d * texHeight) / lineHeight) / 256;
                        
                        if (texY < 0) texY = 0;
                        if (texY >= texHeight) texY = texHeight - 1;
                        
                        uint32_t color = G_SampleTexture(tex, texX / (float)texWidth, texY / (float)texHeight);
                        
                        if (raycaster->config->useLighting) {
                            G_ApplyLighting(&color, perpWallDist, raycaster->level->ambientLight);
                        }
                        
                        if (raycaster->config->useFog) {
                            G_ApplyFog(&color, perpWallDist, raycaster->level->fogColor, raycaster->level->fogDensity);
                        }
                        
                        if (side == 1) {
                            color = (color >> 1) & 0x7F7F7F7F;
                        }
                        
                        raycaster->framebuffer[y * raycaster->width + x] = color;
                    }
                }
            }
        } else {
            uint32_t wallColor = 0xFF808080;
            if (side == 1) wallColor = 0xFF606060;
            
            for (int y = drawStart; y <= drawEnd; y++) {
                raycaster->framebuffer[y * raycaster->width + x] = wallColor;
            }
        }
    }
}

void G_RenderFloorCeiling(raycaster_t* raycaster) {
    if (!raycaster || !raycaster->level || !raycaster->player) return;
    
    int pitch = (int)(raycaster->player->pitch * raycaster->height);
    
    for (int y = 0; y < raycaster->height; y++) {
        bool isCeiling = y < raycaster->height / 2 + pitch;
        int p = isCeiling ? raycaster->height / 2 + pitch - y : y - raycaster->height / 2 - pitch;
        
        if (p == 0) continue;
        
        float rowDistance = (0.5f * raycaster->height) / p;
        
        float floorStepX = rowDistance * (raycaster->rayDirX[raycaster->width - 1] - raycaster->rayDirX[0]) / raycaster->width;
        float floorStepY = rowDistance * (raycaster->rayDirY[raycaster->width - 1] - raycaster->rayDirY[0]) / raycaster->width;
        
        float floorX = raycaster->player->position.x + rowDistance * raycaster->rayDirX[0];
        float floorY = raycaster->player->position.y + rowDistance * raycaster->rayDirY[0];
        
        for (int x = 0; x < raycaster->width; x++) {
            if (rowDistance >= raycaster->zBuffer[x]) {
                floorX += floorStepX;
                floorY += floorStepY;
                continue;
            }
            
            int cellX = (int)floorX;
            int cellY = (int)floorY;
            
            if (cellX >= 0 && cellX < raycaster->level->width && 
                cellY >= 0 && cellY < raycaster->level->height) {
                
                uint32_t color = isCeiling ? 0xFF404040 : 0xFF202020;
                
                if (raycaster->textures && raycaster->config && raycaster->config->useTextures) {
                    uint8_t* texArray = isCeiling ? raycaster->level->ceilingTextures : raycaster->level->floorTextures;
                    if (texArray) {
                        int texIndex = texArray[cellY * raycaster->level->width + cellX];
                        if (texIndex > 0) {
                            texture_t* textures = isCeiling ? raycaster->textures->ceilingTextures : raycaster->textures->floorTextures;
                            int texCount = isCeiling ? raycaster->textures->ceilingTextureCount : raycaster->textures->floorTextureCount;
                            
                            if (texIndex <= texCount && textures[texIndex - 1]) {
                                float tx = floorX - cellX;
                                float ty = floorY - cellY;
                                color = G_SampleTexture(textures[texIndex - 1], tx, ty);
                            }
                        }
                    }
                }
                
                if (raycaster->config && raycaster->config->useLighting) {
                    G_ApplyLighting(&color, rowDistance, raycaster->level->ambientLight);
                }
                
                if (raycaster->config && raycaster->config->useFog) {
                    G_ApplyFog(&color, rowDistance, raycaster->level->fogColor, raycaster->level->fogDensity);
                }
                
                raycaster->framebuffer[y * raycaster->width + x] = color;
            }
            
            floorX += floorStepX;
            floorY += floorStepY;
        }
    }
}

void G_RenderEntities(raycaster_t* raycaster) {
    if (!raycaster || !raycaster->level || !raycaster->player) return;
    
    int spriteCount = 0;
    
    for (int i = 0; i < raycaster->level->entityCount && spriteCount < MAX_ENTITIES; i++) {
        entity_t* entity = &raycaster->level->entities[i];
        if (!entity->active || !entity->visible) continue;
        
        raycaster->spriteOrder[spriteCount] = i;
        float dx = entity->position.x - raycaster->player->position.x;
        float dy = entity->position.y - raycaster->player->position.y;
        raycaster->spriteDistance[spriteCount] = dx * dx + dy * dy;
        spriteCount++;
    }
    
    sortSprites(raycaster, spriteCount);
    
    for (int i = 0; i < spriteCount; i++) {
        entity_t* entity = &raycaster->level->entities[raycaster->spriteOrder[i]];
        
        float spriteX = entity->position.x - raycaster->player->position.x;
        float spriteY = entity->position.y - raycaster->player->position.y;
        
        float invDet = 1.0f / (raycaster->player->plane.x * raycaster->player->direction.y - 
                               raycaster->player->direction.x * raycaster->player->plane.y);
        
        float transformX = invDet * (raycaster->player->direction.y * spriteX - 
                                     raycaster->player->direction.x * spriteY);
        float transformY = invDet * (-raycaster->player->plane.y * spriteX + 
                                     raycaster->player->plane.x * spriteY);
        
        if (transformY <= 0.1f) continue;
        
        int spriteScreenX = (int)((raycaster->width / 2) * (1 + transformX / transformY));
        
        int spriteHeight = abs((int)(raycaster->height / transformY));
        int spriteWidth = spriteHeight;
        
        int pitch = (int)(raycaster->player->pitch * raycaster->height);
        
        int drawStartY = -spriteHeight / 2 + raycaster->height / 2 + pitch;
        if (drawStartY < 0) drawStartY = 0;
        int drawEndY = spriteHeight / 2 + raycaster->height / 2 + pitch;
        if (drawEndY >= raycaster->height) drawEndY = raycaster->height - 1;
        
        int drawStartX = -spriteWidth / 2 + spriteScreenX;
        if (drawStartX < 0) drawStartX = 0;
        int drawEndX = spriteWidth / 2 + spriteScreenX;
        if (drawEndX >= raycaster->width) drawEndX = raycaster->width - 1;
        
        for (int stripe = drawStartX; stripe <= drawEndX; stripe++) {
            if (stripe < 0 || stripe >= raycaster->width) continue;
            if (transformY >= raycaster->zBuffer[stripe]) continue;
            
            for (int y = drawStartY; y <= drawEndY; y++) {
                raycaster->framebuffer[y * raycaster->width + stripe] = 0xFFFF0000;
            }
        }
    }
}

void G_RenderWeapon(raycaster_t* raycaster) {
    if (!raycaster || !raycaster->player) return;
}

void G_RenderSkybox(raycaster_t* raycaster) {
    if (!raycaster || !raycaster->textures || !raycaster->textures->skyboxTexture) return;
}

void G_CastRay(raycaster_t* raycaster, int x, float* perpWallDist,
               int* mapX, int* mapY, int* side, float* wallX) {
    if (!raycaster || !raycaster->level || !raycaster->player) {
        *perpWallDist = -1;
        return;
    }
    
    float rayDirX = raycaster->rayDirX[x];
    float rayDirY = raycaster->rayDirY[x];
    
    *mapX = (int)raycaster->player->position.x;
    *mapY = (int)raycaster->player->position.y;
    
    float sideDistX, sideDistY;
    float deltaDistX = fabsf(1.0f / rayDirX);
    float deltaDistY = fabsf(1.0f / rayDirY);
    
    int stepX, stepY;
    
    if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (raycaster->player->position.x - *mapX) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (*mapX + 1.0f - raycaster->player->position.x) * deltaDistX;
    }
    
    if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (raycaster->player->position.y - *mapY) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = (*mapY + 1.0f - raycaster->player->position.y) * deltaDistY;
    }
    
    bool hit = false;
    int maxSteps = 100;
    int steps = 0;
    
    while (!hit && steps < maxSteps) {
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            *mapX += stepX;
            *side = 0;
        } else {
            sideDistY += deltaDistY;
            *mapY += stepY;
            *side = 1;
        }
        
        if (*mapX < 0 || *mapX >= raycaster->level->width ||
            *mapY < 0 || *mapY >= raycaster->level->height) {
            break;
        }
        
        uint8_t tile = raycaster->level->tiles[*mapY * raycaster->level->width + *mapX];
        if (tile == TILE_WALL) {
            hit = true;
        }
        
        steps++;
    }
    
    if (!hit) {
        *perpWallDist = -1;
        return;
    }
    
    if (*side == 0) {
        *perpWallDist = (*mapX - raycaster->player->position.x + (1 - stepX) / 2) / rayDirX;
    } else {
        *perpWallDist = (*mapY - raycaster->player->position.y + (1 - stepY) / 2) / rayDirY;
    }
    
    if (*side == 0) {
        *wallX = raycaster->player->position.y + *perpWallDist * rayDirY;
    } else {
        *wallX = raycaster->player->position.x + *perpWallDist * rayDirX;
    }
    *wallX -= floorf(*wallX);
}

bool G_CheckLineOfSight(level_t* level, vec2_t start, vec2_t end) {
    if (!level) return false;
    
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float distance = sqrtf(dx * dx + dy * dy);
    
    if (distance < 0.01f) return true;
    
    dx /= distance;
    dy /= distance;
    
    float x = start.x;
    float y = start.y;
    
    for (float t = 0; t < distance; t += 0.1f) {
        int mapX = (int)x;
        int mapY = (int)y;
        
        if (mapX < 0 || mapX >= level->width || mapY < 0 || mapY >= level->height) {
            return false;
        }
        
        if (level->tiles[mapY * level->width + mapX] == TILE_WALL) {
            return false;
        }
        
        x += dx * 0.1f;
        y += dy * 0.1f;
    }
    
    return true;
}

float G_GetDistance(vec2_t a, vec2_t b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx * dx + dy * dy);
}

float G_GetAngle(vec2_t from, vec2_t to) {
    return atan2f(to.y - from.y, to.x - from.x);
}

void G_ApplyLighting(uint32_t* color, float distance, float ambientLight) {
    if (!color) return;
    
    float attenuation = 1.0f / (1.0f + distance * 0.1f);
    float light = ambientLight + attenuation * (1.0f - ambientLight);
    
    if (light > 1.0f) light = 1.0f;
    if (light < 0.0f) light = 0.0f;
    
    uint8_t r = (*color >> 16) & 0xFF;
    uint8_t g = (*color >> 8) & 0xFF;
    uint8_t b = *color & 0xFF;
    uint8_t a = (*color >> 24) & 0xFF;
    
    r = (uint8_t)(r * light);
    g = (uint8_t)(g * light);
    b = (uint8_t)(b * light);
    
    *color = (a << 24) | (r << 16) | (g << 8) | b;
}

void G_ApplyFog(uint32_t* color, float distance, rgba_t fogColor, float fogDensity) {
    if (!color || fogDensity <= 0) return;
    
    float fogFactor = expf(-distance * fogDensity);
    if (fogFactor > 1.0f) fogFactor = 1.0f;
    if (fogFactor < 0.0f) fogFactor = 0.0f;
    
    uint8_t r = (*color >> 16) & 0xFF;
    uint8_t g = (*color >> 8) & 0xFF;
    uint8_t b = *color & 0xFF;
    uint8_t a = (*color >> 24) & 0xFF;
    
    r = (uint8_t)(r * fogFactor + fogColor.r * (1.0f - fogFactor));
    g = (uint8_t)(g * fogFactor + fogColor.g * (1.0f - fogFactor));
    b = (uint8_t)(b * fogFactor + fogColor.b * (1.0f - fogFactor));
    
    *color = (a << 24) | (r << 16) | (g << 8) | b;
}

uint32_t G_SampleTexture(texture_t texture, float u, float v) {
    if (!texture) return 0xFF000000;
    
    int width, height;
    R_GetTextureSize(texture, &width, &height);
    
    if (u < 0) u = 0;
    if (u > 1) u = 1;
    if (v < 0) v = 0;
    if (v > 1) v = 1;
    
    int x = (int)(u * width);
    int y = (int)(v * height);
    
    if (x >= width) x = width - 1;
    if (y >= height) y = height - 1;
    
    void* data = R_GetTextureData(texture);
    if (!data) return 0xFF000000;
    
    uint32_t* pixels = (uint32_t*)data;
    return pixels[y * width + x];
}

uint32_t G_BlendColors(uint32_t a, uint32_t b, float alpha) {
    if (alpha <= 0) return a;
    if (alpha >= 1) return b;
    
    uint8_t ar = (a >> 16) & 0xFF;
    uint8_t ag = (a >> 8) & 0xFF;
    uint8_t ab = a & 0xFF;
    uint8_t aa = (a >> 24) & 0xFF;
    
    uint8_t br = (b >> 16) & 0xFF;
    uint8_t bg = (b >> 8) & 0xFF;
    uint8_t bb = b & 0xFF;
    uint8_t ba = (b >> 24) & 0xFF;
    
    uint8_t r = (uint8_t)(ar * (1 - alpha) + br * alpha);
    uint8_t g = (uint8_t)(ag * (1 - alpha) + bg * alpha);
    uint8_t bl = (uint8_t)(ab * (1 - alpha) + bb * alpha);
    uint8_t al = (uint8_t)(aa * (1 - alpha) + ba * alpha);
    
    return (al << 24) | (r << 16) | (g << 8) | bl;
}

static void sortSprites(raycaster_t* raycaster, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (raycaster->spriteDistance[i] < raycaster->spriteDistance[j]) {
                float tempDist = raycaster->spriteDistance[i];
                raycaster->spriteDistance[i] = raycaster->spriteDistance[j];
                raycaster->spriteDistance[j] = tempDist;
                
                int tempOrder = raycaster->spriteOrder[i];
                raycaster->spriteOrder[i] = raycaster->spriteOrder[j];
                raycaster->spriteOrder[j] = tempOrder;
            }
        }
    }
}