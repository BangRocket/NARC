#ifndef G_RAYCASTER_H
#define G_RAYCASTER_H

#include "g_defs.h"
#include "../../engine/include/r_renderer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    texture_t wallTextures[MAX_TEXTURES];
    texture_t floorTextures[MAX_TEXTURES];
    texture_t ceilingTextures[MAX_TEXTURES];
    texture_t entityTextures[MAX_TEXTURES];
    texture_t weaponTextures[MAX_WEAPONS];
    texture_t skyboxTexture;
    
    int wallTextureCount;
    int floorTextureCount;
    int ceilingTextureCount;
    int entityTextureCount;
    int weaponTextureCount;
} raycaster_textures_t;

typedef struct {
    uint32_t* framebuffer;
    float* zBuffer;
    int width;
    int height;
    
    player_t* player;
    level_t* level;
    raycaster_textures_t* textures;
    render_config_t* config;
    
    float* rayDirX;
    float* rayDirY;
    
    int* spriteOrder;
    float* spriteDistance;
    
    bool initialized;
} raycaster_t;

raycaster_t* G_CreateRaycaster(int width, int height);
void G_DestroyRaycaster(raycaster_t* raycaster);

void G_SetRaycasterPlayer(raycaster_t* raycaster, player_t* player);
void G_SetRaycasterLevel(raycaster_t* raycaster, level_t* level);
void G_SetRaycasterTextures(raycaster_t* raycaster, raycaster_textures_t* textures);
void G_SetRaycasterConfig(raycaster_t* raycaster, render_config_t* config);

void G_RenderFrame(raycaster_t* raycaster);
void G_RenderWalls(raycaster_t* raycaster);
void G_RenderFloorCeiling(raycaster_t* raycaster);
void G_RenderEntities(raycaster_t* raycaster);
void G_RenderWeapon(raycaster_t* raycaster);
void G_RenderSkybox(raycaster_t* raycaster);

void G_CastRay(raycaster_t* raycaster, int x, float* perpWallDist, 
               int* mapX, int* mapY, int* side, float* wallX);

bool G_CheckLineOfSight(level_t* level, vec2_t start, vec2_t end);
float G_GetDistance(vec2_t a, vec2_t b);
float G_GetAngle(vec2_t from, vec2_t to);

void G_ApplyLighting(uint32_t* color, float distance, float ambientLight);
void G_ApplyFog(uint32_t* color, float distance, rgba_t fogColor, float fogDensity);

uint32_t G_SampleTexture(texture_t texture, float u, float v);
uint32_t G_BlendColors(uint32_t a, uint32_t b, float alpha);

#ifdef __cplusplus
}
#endif

#endif