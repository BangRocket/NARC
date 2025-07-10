#ifndef G_DEFS_H
#define G_DEFS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MAP_WIDTH 256
#define MAX_MAP_HEIGHT 256
#define MAX_TEXTURES 256
#define MAX_ENTITIES 1024
#define MAX_WEAPONS 16
#define MAX_DOORS 128
#define MAX_LIGHTS 256

#define TILE_SIZE 64.0f
#define PLAYER_HEIGHT 32.0f
#define PLAYER_RADIUS 16.0f
#define PLAYER_SPEED 200.0f
#define PLAYER_TURN_SPEED 3.0f

typedef enum {
    TILE_EMPTY = 0,
    TILE_WALL = 1,
    TILE_DOOR = 2,
    TILE_TRIGGER = 3,
    TILE_TELEPORT = 4,
    TILE_SECRET = 5
} tile_type_t;

typedef enum {
    ET_NONE = 0,
    ET_ENEMY_SOLDIER,
    ET_ENEMY_DEMON,
    ET_ITEM_AMMO,
    ET_ITEM_HEALTH,
    ET_ITEM_ARMOR,
    ET_ITEM_KEY,
    ET_TURRET,
    ET_DECORATION
} entity_type_t;

typedef enum {
    ES_IDLE = 0,
    ES_WALKING,
    ES_ATTACKING,
    ES_HURT,
    ES_DYING,
    ES_DEAD
} entity_state_t;

typedef enum {
    WS_READY = 0,
    WS_FIRING,
    WS_RELOADING,
    WS_SWITCHING
} weapon_state_t;

typedef enum {
    DS_CLOSED = 0,
    DS_OPENING,
    DS_OPEN,
    DS_CLOSING
} door_state_t;

typedef struct {
    float x, y;
} vec2_t;

typedef struct {
    float x, y, z;
} vec3_t;

typedef struct {
    int x, y;
} ivec2_t;

typedef struct {
    uint8_t r, g, b, a;
} rgba_t;

typedef struct {
    float left, top, right, bottom;
} rect_t;

typedef struct {
    vec2_t position;
    vec2_t direction;
    vec2_t plane;
    float height;
    float pitch;
    float bob;
    float bobTime;
    
    int health;
    int maxHealth;
    int armor;
    int maxArmor;
    
    int currentWeapon;
    int ammo[MAX_WEAPONS];
    int maxAmmo[MAX_WEAPONS];
    bool hasWeapon[MAX_WEAPONS];
    
    bool keys[4];
    int score;
    int secrets;
    
    float moveSpeed;
    float turnSpeed;
    bool isMoving;
    bool isShooting;
    bool isDead;
} player_t;

typedef struct {
    entity_type_t type;
    vec2_t position;
    vec2_t velocity;
    float angle;
    float radius;
    float height;
    
    entity_state_t state;
    entity_state_t previousState;
    float stateTime;
    
    int health;
    int maxHealth;
    int damage;
    float speed;
    float sightRange;
    float attackRange;
    
    void* target;
    vec2_t lastKnownTargetPos;
    float lastSeenTime;
    
    int currentFrame;
    float frameTime;
    
    bool active;
    bool solid;
    bool visible;
    bool pickupable;
} entity_t;

typedef struct {
    char name[64];
    weapon_state_t state;
    float stateTime;
    
    int damage;
    int ammoPerShot;
    float fireRate;
    float reloadTime;
    float switchTime;
    float range;
    float spread;
    
    int clipSize;
    int currentClip;
    
    bool automatic;
    bool hitscan;
    float projectileSpeed;
} weapon_t;

typedef struct {
    ivec2_t position;
    door_state_t state;
    float stateTime;
    float openTime;
    
    int textureIndex;
    int keyRequired;
    bool locked;
    bool secret;
    
    float openAmount;
} door_t;

typedef struct {
    vec3_t position;
    rgba_t color;
    float intensity;
    float radius;
    bool flicker;
    float flickerSpeed;
} light_t;

typedef struct {
    int width;
    int height;
    uint8_t* tiles;
    uint8_t* floorTextures;
    uint8_t* ceilingTextures;
    uint8_t* wallTextures[4];
    uint8_t* lightMap;
    
    char name[64];
    char nextLevel[64];
    char music[64];
    
    vec2_t playerStart;
    float playerStartAngle;
    
    int entityCount;
    entity_t* entities;
    
    int doorCount;
    door_t* doors;
    
    int lightCount;
    light_t* lights;
    
    float ambientLight;
    rgba_t fogColor;
    float fogDensity;
} level_t;

typedef struct {
    int textureSize;
    int maxTextureSize;
    int renderDistance;
    int fov;
    
    bool useTextures;
    bool useFloorCeiling;
    bool useLighting;
    bool useFog;
    bool useShadows;
    bool useMultithreading;
    
    int threadCount;
    int resolutionScale;
} render_config_t;

#ifdef __cplusplus
}
#endif

#endif