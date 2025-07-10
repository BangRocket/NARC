#include "../include/g_pathfind.h"
#include "../include/g_level.h"
#include "../../engine/include/i_system.h"
#include <math.h>
#include <string.h>
#include <float.h>

#define PATH_INITIAL_CAPACITY 64
#define DIAGONAL_COST 1.414f

static void resetPathfinder(pathfinder_t* pathfinder);
static path_node_t* getNode(pathfinder_t* pathfinder, int x, int y);
static void addToOpenSet(pathfinder_t* pathfinder, path_node_t* node);
static path_node_t* getLowestFNode(pathfinder_t* pathfinder);
static void removeFromOpenSet(pathfinder_t* pathfinder, path_node_t* node);
static float heuristic(int x1, int y1, int x2, int y2);
static path_t* reconstructPath(path_node_t* endNode);

pathfinder_t* G_CreatePathfinder(int mapWidth, int mapHeight) {
    pathfinder_t* pathfinder = (pathfinder_t*)I_Calloc(1, sizeof(pathfinder_t));
    
    pathfinder->width = mapWidth;
    pathfinder->height = mapHeight;
    pathfinder->nodes = (path_node_t*)I_Calloc(mapWidth * mapHeight, sizeof(path_node_t));
    pathfinder->openSet = (path_node_t**)I_Malloc(mapWidth * mapHeight * sizeof(path_node_t*));
    
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            path_node_t* node = &pathfinder->nodes[y * mapWidth + x];
            node->x = x;
            node->y = y;
        }
    }
    
    pathfinder->initialized = true;
    I_Log("Pathfinder created for %dx%d map", mapWidth, mapHeight);
    
    return pathfinder;
}

void G_DestroyPathfinder(pathfinder_t* pathfinder) {
    if (!pathfinder) return;
    
    I_Free(pathfinder->nodes);
    I_Free(pathfinder->openSet);
    I_Free(pathfinder);
    
    I_Log("Pathfinder destroyed");
}

path_t* G_FindPath(pathfinder_t* pathfinder, level_t* level, 
                   vec2_t start, vec2_t end, float entityRadius) {
    if (!pathfinder || !level || !pathfinder->initialized) return NULL;
    
    int startX = (int)(start.x / TILE_SIZE);
    int startY = (int)(start.y / TILE_SIZE);
    int endX = (int)(end.x / TILE_SIZE);
    int endY = (int)(end.y / TILE_SIZE);
    
    if (startX < 0 || startX >= pathfinder->width || 
        startY < 0 || startY >= pathfinder->height ||
        endX < 0 || endX >= pathfinder->width || 
        endY < 0 || endY >= pathfinder->height) {
        return NULL;
    }
    
    if (!G_IsTileWalkable(level, endX, endY, entityRadius)) {
        return NULL;
    }
    
    resetPathfinder(pathfinder);
    
    path_node_t* startNode = getNode(pathfinder, startX, startY);
    path_node_t* endNode = getNode(pathfinder, endX, endY);
    
    startNode->g = 0;
    startNode->h = heuristic(startX, startY, endX, endY);
    startNode->f = startNode->h;
    
    addToOpenSet(pathfinder, startNode);
    
    while (pathfinder->openSetCount > 0) {
        path_node_t* current = getLowestFNode(pathfinder);
        
        if (current == endNode) {
            path_t* path = reconstructPath(endNode);
            if (path) {
                G_SimplifyPath(path);
                G_SmoothPath(path, level, entityRadius);
            }
            return path;
        }
        
        removeFromOpenSet(pathfinder, current);
        current->inClosedSet = true;
        
        int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        
        for (int i = 0; i < 8; i++) {
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];
            
            if (nx < 0 || nx >= pathfinder->width || 
                ny < 0 || ny >= pathfinder->height) {
                continue;
            }
            
            path_node_t* neighbor = getNode(pathfinder, nx, ny);
            
            if (neighbor->inClosedSet) continue;
            
            if (!G_IsTileWalkable(level, nx, ny, entityRadius)) continue;
            
            bool isDiagonal = (dx[i] != 0 && dy[i] != 0);
            if (isDiagonal) {
                if (!G_IsTileWalkable(level, current->x + dx[i], current->y, entityRadius) ||
                    !G_IsTileWalkable(level, current->x, current->y + dy[i], entityRadius)) {
                    continue;
                }
            }
            
            float moveCost = isDiagonal ? DIAGONAL_COST : 1.0f;
            float tentativeG = current->g + moveCost * G_GetMoveCost(level, current->x, current->y, nx, ny);
            
            if (!neighbor->inOpenSet) {
                neighbor->g = tentativeG;
                neighbor->h = heuristic(nx, ny, endX, endY);
                neighbor->f = neighbor->g + neighbor->h;
                neighbor->parent = current;
                addToOpenSet(pathfinder, neighbor);
            } else if (tentativeG < neighbor->g) {
                neighbor->g = tentativeG;
                neighbor->f = neighbor->g + neighbor->h;
                neighbor->parent = current;
            }
        }
    }
    
    return NULL;
}

void G_FreePath(path_t* path) {
    if (!path) return;
    
    I_Free(path->points);
    I_Free(path);
}

bool G_IsTileWalkable(level_t* level, int x, int y, float radius) {
    if (!level) return false;
    
    int margin = (int)ceilf(radius / TILE_SIZE);
    
    for (int dy = -margin; dy <= margin; dy++) {
        for (int dx = -margin; dx <= margin; dx++) {
            int checkX = x + dx;
            int checkY = y + dy;
            
            if (checkX < 0 || checkX >= level->width || 
                checkY < 0 || checkY >= level->height) {
                return false;
            }
            
            uint8_t tile = G_GetTile(level, checkX, checkY);
            if (tile == TILE_WALL || tile == TILE_DOOR) {
                float tileCenterX = (checkX + 0.5f) * TILE_SIZE;
                float tileCenterY = (checkY + 0.5f) * TILE_SIZE;
                float nodeCenterX = (x + 0.5f) * TILE_SIZE;
                float nodeCenterY = (y + 0.5f) * TILE_SIZE;
                
                float dx = tileCenterX - nodeCenterX;
                float dy = tileCenterY - nodeCenterY;
                float dist = sqrtf(dx * dx + dy * dy);
                
                if (dist < radius + TILE_SIZE * 0.7f) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

float G_GetMoveCost(level_t* level, int fromX, int fromY, int toX, int toY) {
    if (!level) return 1.0f;
    
    return 1.0f;
}

vec2_t G_GetNextPathPoint(path_t* path, vec2_t currentPos, float lookAheadDist) {
    if (!path || path->count == 0) return currentPos;
    
    int closestIndex = 0;
    float closestDist = FLT_MAX;
    
    for (int i = 0; i < path->count; i++) {
        float dx = path->points[i].x - currentPos.x;
        float dy = path->points[i].y - currentPos.y;
        float dist = sqrtf(dx * dx + dy * dy);
        
        if (dist < closestDist) {
            closestDist = dist;
            closestIndex = i;
        }
    }
    
    int targetIndex = closestIndex;
    float accumulatedDist = 0.0f;
    
    for (int i = closestIndex + 1; i < path->count; i++) {
        float dx = path->points[i].x - path->points[i-1].x;
        float dy = path->points[i].y - path->points[i-1].y;
        float segmentDist = sqrtf(dx * dx + dy * dy);
        
        if (accumulatedDist + segmentDist > lookAheadDist) {
            break;
        }
        
        accumulatedDist += segmentDist;
        targetIndex = i;
    }
    
    return path->points[targetIndex];
}

bool G_HasReachedPathEnd(path_t* path, vec2_t currentPos, float threshold) {
    if (!path || path->count == 0) return true;
    
    vec2_t endPoint = path->points[path->count - 1];
    float dx = endPoint.x - currentPos.x;
    float dy = endPoint.y - currentPos.y;
    float dist = sqrtf(dx * dx + dy * dy);
    
    return dist < threshold;
}

void G_SmoothPath(path_t* path, level_t* level, float entityRadius) {
    if (!path || !level || path->count < 3) return;
    
    vec2_t* smoothed = (vec2_t*)I_Malloc(path->count * sizeof(vec2_t));
    int smoothedCount = 0;
    
    smoothed[smoothedCount++] = path->points[0];
    
    int currentIndex = 0;
    while (currentIndex < path->count - 1) {
        int furthestVisible = currentIndex + 1;
        
        for (int i = currentIndex + 2; i < path->count; i++) {
            if (G_CheckLineOfSight(level, path->points[currentIndex], path->points[i])) {
                furthestVisible = i;
            } else {
                break;
            }
        }
        
        smoothed[smoothedCount++] = path->points[furthestVisible];
        currentIndex = furthestVisible;
    }
    
    I_Free(path->points);
    path->points = smoothed;
    path->count = smoothedCount;
}

void G_SimplifyPath(path_t* path) {
    if (!path || path->count < 3) return;
    
    vec2_t* simplified = (vec2_t*)I_Malloc(path->count * sizeof(vec2_t));
    int simplifiedCount = 0;
    
    simplified[simplifiedCount++] = path->points[0];
    
    vec2_t lastDir = {0, 0};
    for (int i = 1; i < path->count - 1; i++) {
        vec2_t dir = {
            path->points[i+1].x - path->points[i].x,
            path->points[i+1].y - path->points[i].y
        };
        
        float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
        }
        
        float dot = lastDir.x * dir.x + lastDir.y * dir.y;
        if (dot < 0.99f) {
            simplified[simplifiedCount++] = path->points[i];
            lastDir = dir;
        }
    }
    
    simplified[simplifiedCount++] = path->points[path->count - 1];
    
    I_Free(path->points);
    path->points = simplified;
    path->count = simplifiedCount;
}

void G_DebugDrawPath(path_t* path, uint32_t* framebuffer, int width, int height, 
                     vec2_t offset, float scale) {
    if (!path || !framebuffer) return;
    
    for (int i = 0; i < path->count - 1; i++) {
        int x1 = (int)((path->points[i].x - offset.x) * scale);
        int y1 = (int)((path->points[i].y - offset.y) * scale);
        int x2 = (int)((path->points[i+1].x - offset.x) * scale);
        int y2 = (int)((path->points[i+1].y - offset.y) * scale);
        
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = x1 < x2 ? 1 : -1;
        int sy = y1 < y2 ? 1 : -1;
        int err = dx - dy;
        
        while (true) {
            if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height) {
                framebuffer[y1 * width + x1] = 0xFF00FF00;
            }
            
            if (x1 == x2 && y1 == y2) break;
            
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }
        }
    }
}

static void resetPathfinder(pathfinder_t* pathfinder) {
    for (int i = 0; i < pathfinder->width * pathfinder->height; i++) {
        pathfinder->nodes[i].g = FLT_MAX;
        pathfinder->nodes[i].h = 0;
        pathfinder->nodes[i].f = FLT_MAX;
        pathfinder->nodes[i].parent = NULL;
        pathfinder->nodes[i].inOpenSet = false;
        pathfinder->nodes[i].inClosedSet = false;
    }
    pathfinder->openSetCount = 0;
}

static path_node_t* getNode(pathfinder_t* pathfinder, int x, int y) {
    if (x < 0 || x >= pathfinder->width || y < 0 || y >= pathfinder->height) {
        return NULL;
    }
    return &pathfinder->nodes[y * pathfinder->width + x];
}

static void addToOpenSet(pathfinder_t* pathfinder, path_node_t* node) {
    if (!node || node->inOpenSet) return;
    
    pathfinder->openSet[pathfinder->openSetCount++] = node;
    node->inOpenSet = true;
}

static path_node_t* getLowestFNode(pathfinder_t* pathfinder) {
    if (pathfinder->openSetCount == 0) return NULL;
    
    int lowestIndex = 0;
    float lowestF = pathfinder->openSet[0]->f;
    
    for (int i = 1; i < pathfinder->openSetCount; i++) {
        if (pathfinder->openSet[i]->f < lowestF) {
            lowestF = pathfinder->openSet[i]->f;
            lowestIndex = i;
        }
    }
    
    return pathfinder->openSet[lowestIndex];
}

static void removeFromOpenSet(pathfinder_t* pathfinder, path_node_t* node) {
    if (!node || !node->inOpenSet) return;
    
    for (int i = 0; i < pathfinder->openSetCount; i++) {
        if (pathfinder->openSet[i] == node) {
            pathfinder->openSet[i] = pathfinder->openSet[--pathfinder->openSetCount];
            node->inOpenSet = false;
            break;
        }
    }
}

static float heuristic(int x1, int y1, int x2, int y2) {
    float dx = (float)abs(x2 - x1);
    float dy = (float)abs(y2 - y1);
    return sqrtf(dx * dx + dy * dy);
}

static path_t* reconstructPath(path_node_t* endNode) {
    if (!endNode) return NULL;
    
    int pathLength = 0;
    path_node_t* current = endNode;
    while (current) {
        pathLength++;
        current = current->parent;
    }
    
    path_t* path = (path_t*)I_Malloc(sizeof(path_t));
    path->points = (vec2_t*)I_Malloc(pathLength * sizeof(vec2_t));
    path->count = pathLength;
    path->capacity = pathLength;
    
    current = endNode;
    for (int i = pathLength - 1; i >= 0; i--) {
        path->points[i].x = (current->x + 0.5f) * TILE_SIZE;
        path->points[i].y = (current->y + 0.5f) * TILE_SIZE;
        current = current->parent;
    }
    
    return path;
}