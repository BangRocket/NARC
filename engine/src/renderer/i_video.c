#include "../../include/i_video.h"
#include "../../include/i_system.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>

static uint32_t* framebuffer = NULL;
static Texture2D framebufferTexture;
static int screenWidth = 0;
static int screenHeight = 0;
static bool initialized = false;

bool I_InitVideo(const video_config_t* config) {
    if (initialized) return true;
    
    if (!config) {
        I_Error("I_InitVideo: NULL config");
        return false;
    }
    
    screenWidth = config->width;
    screenHeight = config->height;
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    if (config->fullscreen) {
        SetConfigFlags(FLAG_FULLSCREEN_MODE);
    }
    if (config->vsync) {
        SetConfigFlags(FLAG_VSYNC_HINT);
    }
    
    InitWindow(screenWidth, screenHeight, config->title);
    
    if (!IsWindowReady()) {
        I_Error("Failed to create window");
        return false;
    }
    
    SetTargetFPS(config->targetFPS);
    
    framebuffer = (uint32_t*)I_Calloc(screenWidth * screenHeight, sizeof(uint32_t));
    
    Image img = {
        .data = framebuffer,
        .width = screenWidth,
        .height = screenHeight,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    
    framebufferTexture = LoadTextureFromImage(img);
    SetTextureFilter(framebufferTexture, TEXTURE_FILTER_POINT);
    
    I_Log("Video initialized: %dx%d %s", screenWidth, screenHeight, 
          config->fullscreen ? "fullscreen" : "windowed");
    
    initialized = true;
    return true;
}

void I_ShutdownVideo(void) {
    if (!initialized) return;
    
    if (framebuffer) {
        I_Free(framebuffer);
        framebuffer = NULL;
    }
    
    UnloadTexture(framebufferTexture);
    CloseWindow();
    
    initialized = false;
    I_Log("Video shutdown");
}

void I_BeginDrawing(void) {
    BeginDrawing();
}

void I_EndDrawing(void) {
    UpdateTexture(framebufferTexture, framebuffer);
    
    ClearBackground(BLACK);
    
    float scale = (float)GetScreenWidth() / screenWidth;
    if ((float)GetScreenHeight() / screenHeight < scale) {
        scale = (float)GetScreenHeight() / screenHeight;
    }
    
    int drawWidth = (int)(screenWidth * scale);
    int drawHeight = (int)(screenHeight * scale);
    int drawX = (GetScreenWidth() - drawWidth) / 2;
    int drawY = (GetScreenHeight() - drawHeight) / 2;
    
    Rectangle source = { 0, 0, (float)screenWidth, (float)screenHeight };
    Rectangle dest = { (float)drawX, (float)drawY, (float)drawWidth, (float)drawHeight };
    Vector2 origin = { 0, 0 };
    
    DrawTexturePro(framebufferTexture, source, dest, origin, 0.0f, WHITE);
    
    EndDrawing();
}

bool I_ShouldClose(void) {
    return WindowShouldClose();
}

void I_SetShouldClose(bool close) {
    if (close) {
        CloseWindow();
    }
}

void I_GetScreenSize(int* width, int* height) {
    if (width) *width = screenWidth;
    if (height) *height = screenHeight;
}

float I_GetDeltaTime(void) {
    return GetFrameTime();
}

int I_GetFPS(void) {
    return GetFPS();
}

void I_ToggleFullscreen(void) {
    ToggleFullscreen();
}

void I_SetTargetFPS(int fps) {
    SetTargetFPS(fps);
}

uint32_t* I_GetFramebuffer(void) {
    return framebuffer;
}

void I_UpdateFramebuffer(void) {
    UpdateTexture(framebufferTexture, framebuffer);
}

void I_SetWindowIcon(const char* iconPath) {
    if (FileExists(iconPath)) {
        Image icon = LoadImage(iconPath);
        SetWindowIcon(icon);
        UnloadImage(icon);
    }
}