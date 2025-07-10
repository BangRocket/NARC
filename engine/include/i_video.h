#ifndef I_VIDEO_H
#define I_VIDEO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int width;
    int height;
    bool fullscreen;
    bool vsync;
    int targetFPS;
    const char* title;
} video_config_t;

bool I_InitVideo(const video_config_t* config);
void I_ShutdownVideo(void);

void I_BeginDrawing(void);
void I_EndDrawing(void);

bool I_ShouldClose(void);
void I_SetShouldClose(bool close);

void I_GetScreenSize(int* width, int* height);
float I_GetDeltaTime(void);
int I_GetFPS(void);

void I_ToggleFullscreen(void);
void I_SetTargetFPS(int fps);

uint32_t* I_GetFramebuffer(void);
void I_UpdateFramebuffer(void);

void I_SetWindowIcon(const char* iconPath);

#ifdef __cplusplus
}
#endif

#endif