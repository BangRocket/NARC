#ifndef R_RENDERER_H
#define R_RENDERER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} color_t;

typedef struct {
    float x;
    float y;
} vector2_t;

typedef struct {
    float x;
    float y;
    float z;
} vector3_t;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} rectangle_t;

typedef struct texture_s* texture_t;
typedef struct font_s* font_t;
typedef struct shader_s* shader_t;

#define COLOR_WHITE ((color_t){255, 255, 255, 255})
#define COLOR_BLACK ((color_t){0, 0, 0, 255})
#define COLOR_RED ((color_t){255, 0, 0, 255})
#define COLOR_GREEN ((color_t){0, 255, 0, 255})
#define COLOR_BLUE ((color_t){0, 0, 255, 255})
#define COLOR_TRANSPARENT ((color_t){0, 0, 0, 0})

void R_Init(void);
void R_Shutdown(void);

texture_t R_LoadTexture(const char* filename);
texture_t R_LoadTextureFromImage(const void* data, int width, int height);
void R_UnloadTexture(texture_t texture);
void R_GetTextureSize(texture_t texture, int* width, int* height);
void* R_GetTextureData(texture_t texture);
void R_UpdateTexture(texture_t texture, const void* pixels);
void R_SetTextureFilter(texture_t texture, int filter);
void R_SetTextureWrap(texture_t texture, int wrap);

void R_DrawTexture(texture_t texture, int x, int y, color_t tint);
void R_DrawTextureRec(texture_t texture, rectangle_t source, vector2_t position, color_t tint);
void R_DrawTexturePro(texture_t texture, rectangle_t source, rectangle_t dest, 
                      vector2_t origin, float rotation, color_t tint);

void R_ClearBackground(color_t color);
void R_DrawPixel(int x, int y, color_t color);
void R_DrawLine(int x1, int y1, int x2, int y2, color_t color);
void R_DrawCircle(int centerX, int centerY, float radius, color_t color);
void R_DrawRectangle(int x, int y, int width, int height, color_t color);
void R_DrawRectangleLines(int x, int y, int width, int height, color_t color);

font_t R_LoadFont(const char* filename);
font_t R_LoadFontEx(const char* filename, int fontSize, int* fontChars, int charCount);
void R_UnloadFont(font_t font);
void R_DrawText(const char* text, int x, int y, int fontSize, color_t color);
void R_DrawTextEx(font_t font, const char* text, vector2_t position, 
                  float fontSize, float spacing, color_t color);
int R_MeasureText(const char* text, int fontSize);
vector2_t R_MeasureTextEx(font_t font, const char* text, float fontSize, float spacing);

shader_t R_LoadShader(const char* vsFileName, const char* fsFileName);
shader_t R_LoadShaderFromMemory(const char* vsCode, const char* fsCode);
void R_UnloadShader(shader_t shader);
void R_BeginShaderMode(shader_t shader);
void R_EndShaderMode(void);
void R_SetShaderValue(shader_t shader, int locIndex, const void* value, int uniformType);
void R_SetShaderValueV(shader_t shader, int locIndex, const void* value, int uniformType, int count);
int R_GetShaderLocation(shader_t shader, const char* uniformName);

void R_BeginScissorMode(int x, int y, int width, int height);
void R_EndScissorMode(void);

void R_SetLineWidth(float width);

uint32_t R_ColorToInt(color_t color);
color_t R_IntToColor(uint32_t value);
color_t R_Fade(color_t color, float alpha);

#ifdef __cplusplus
}
#endif

#endif