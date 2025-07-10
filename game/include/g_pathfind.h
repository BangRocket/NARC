#ifndef G_PATHFIND_H
#define G_PATHFIND_H

#include "g_defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct path_node_s {
    int x, y;
    float g, h, f;
    struct path_node_s* parent;
    bool inOpenSet;
    bool inClosedSet;
} path_node_t;

typedef struct {
    vec2_t* points;
    int count;
    int capacity;
} path_t;

typedef struct {
    path_node_t* nodes;
    int width;
    int height;
    int openSetCount;
    path_node_t** openSet;
    bool initialized;
} pathfinder_t;

pathfinder_t* G_CreatePathfinder(int mapWidth, int mapHeight);
void G_DestroyPathfinder(pathfinder_t* pathfinder);

path_t* G_FindPath(pathfinder_t* pathfinder, level_t* level, 
                   vec2_t start, vec2_t end, float entityRadius);
void G_FreePath(path_t* path);

bool G_IsTileWalkable(level_t* level, int x, int y, float radius);
float G_GetMoveCost(level_t* level, int fromX, int fromY, int toX, int toY);

vec2_t G_GetNextPathPoint(path_t* path, vec2_t currentPos, float lookAheadDist);
bool G_HasReachedPathEnd(path_t* path, vec2_t currentPos, float threshold);

void G_SmoothPath(path_t* path, level_t* level, float entityRadius);
void G_SimplifyPath(path_t* path);

void G_DebugDrawPath(path_t* path, uint32_t* framebuffer, int width, int height, 
                     vec2_t offset, float scale);

#ifdef __cplusplus
}
#endif

#endif