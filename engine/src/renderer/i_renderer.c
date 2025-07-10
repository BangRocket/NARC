#include "../../include/r_renderer.h"
#include "../../include/i_system.h"
#include "raylib.h"
#include "rlgl.h"
#include <stdlib.h>
#include <string.h>

typedef struct texture_s {
    Texture2D texture;
    int width;
    int height;
} texture_impl_t;

typedef struct font_s {
    Font font;
} font_impl_t;

typedef struct shader_s {
    Shader shader;
} shader_impl_t;

static bool rendererInitialized = false;
static Font defaultFont = { 0 };

void R_Init(void) {
    if (rendererInitialized) return;
    
    defaultFont = GetFontDefault();
    
    I_Log("Renderer initialized");
    rendererInitialized = true;
}

void R_Shutdown(void) {
    if (!rendererInitialized) return;
    
    rendererInitialized = false;
    I_Log("Renderer shutdown");
}

texture_t R_LoadTexture(const char* filename) {
    if (!filename) return NULL;
    
    texture_impl_t* texImpl = (texture_impl_t*)I_Malloc(sizeof(texture_impl_t));
    texImpl->texture = LoadTexture(filename);
    
    if (texImpl->texture.id == 0) {
        I_Warning("Failed to load texture: %s", filename);
        I_Free(texImpl);
        return NULL;
    }
    
    texImpl->width = texImpl->texture.width;
    texImpl->height = texImpl->texture.height;
    
    return (texture_t)texImpl;
}

texture_t R_LoadTextureFromImage(const void* data, int width, int height) {
    if (!data || width <= 0 || height <= 0) return NULL;
    
    Image img = {
        .data = (void*)data,
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    
    texture_impl_t* texImpl = (texture_impl_t*)I_Malloc(sizeof(texture_impl_t));
    texImpl->texture = LoadTextureFromImage(img);
    
    if (texImpl->texture.id == 0) {
        I_Warning("Failed to create texture from image data");
        I_Free(texImpl);
        return NULL;
    }
    
    texImpl->width = width;
    texImpl->height = height;
    
    return (texture_t)texImpl;
}

void R_UnloadTexture(texture_t texture) {
    if (!texture) return;
    
    texture_impl_t* texImpl = (texture_impl_t*)texture;
    UnloadTexture(texImpl->texture);
    I_Free(texImpl);
}

void R_GetTextureSize(texture_t texture, int* width, int* height) {
    if (!texture) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }
    
    texture_impl_t* texImpl = (texture_impl_t*)texture;
    if (width) *width = texImpl->width;
    if (height) *height = texImpl->height;
}

void* R_GetTextureData(texture_t texture) {
    if (!texture) return NULL;
    
    texture_impl_t* texImpl = (texture_impl_t*)texture;
    Image img = LoadImageFromTexture(texImpl->texture);
    void* data = img.data;
    return data;
}

void R_UpdateTexture(texture_t texture, const void* pixels) {
    if (!texture || !pixels) return;
    
    texture_impl_t* texImpl = (texture_impl_t*)texture;
    UpdateTexture(texImpl->texture, pixels);
}

void R_SetTextureFilter(texture_t texture, int filter) {
    if (!texture) return;
    
    texture_impl_t* texImpl = (texture_impl_t*)texture;
    SetTextureFilter(texImpl->texture, filter);
}

void R_SetTextureWrap(texture_t texture, int wrap) {
    if (!texture) return;
    
    texture_impl_t* texImpl = (texture_impl_t*)texture;
    SetTextureWrap(texImpl->texture, wrap);
}

void R_DrawTexture(texture_t texture, int x, int y, color_t tint) {
    if (!texture) return;
    
    texture_impl_t* texImpl = (texture_impl_t*)texture;
    Color rlTint = { tint.r, tint.g, tint.b, tint.a };
    DrawTexture(texImpl->texture, x, y, rlTint);
}

void R_DrawTextureRec(texture_t texture, rectangle_t source, vector2_t position, color_t tint) {
    if (!texture) return;
    
    texture_impl_t* texImpl = (texture_impl_t*)texture;
    Rectangle rlSource = { (float)source.x, (float)source.y, (float)source.width, (float)source.height };
    Vector2 rlPosition = { position.x, position.y };
    Color rlTint = { tint.r, tint.g, tint.b, tint.a };
    
    DrawTextureRec(texImpl->texture, rlSource, rlPosition, rlTint);
}

void R_DrawTexturePro(texture_t texture, rectangle_t source, rectangle_t dest,
                      vector2_t origin, float rotation, color_t tint) {
    if (!texture) return;
    
    texture_impl_t* texImpl = (texture_impl_t*)texture;
    Rectangle rlSource = { (float)source.x, (float)source.y, (float)source.width, (float)source.height };
    Rectangle rlDest = { (float)dest.x, (float)dest.y, (float)dest.width, (float)dest.height };
    Vector2 rlOrigin = { origin.x, origin.y };
    Color rlTint = { tint.r, tint.g, tint.b, tint.a };
    
    DrawTexturePro(texImpl->texture, rlSource, rlDest, rlOrigin, rotation, rlTint);
}

void R_ClearBackground(color_t color) {
    Color rlColor = { color.r, color.g, color.b, color.a };
    ClearBackground(rlColor);
}

void R_DrawPixel(int x, int y, color_t color) {
    Color rlColor = { color.r, color.g, color.b, color.a };
    DrawPixel(x, y, rlColor);
}

void R_DrawLine(int x1, int y1, int x2, int y2, color_t color) {
    Color rlColor = { color.r, color.g, color.b, color.a };
    DrawLine(x1, y1, x2, y2, rlColor);
}

void R_DrawCircle(int centerX, int centerY, float radius, color_t color) {
    Color rlColor = { color.r, color.g, color.b, color.a };
    DrawCircle(centerX, centerY, radius, rlColor);
}

void R_DrawRectangle(int x, int y, int width, int height, color_t color) {
    Color rlColor = { color.r, color.g, color.b, color.a };
    DrawRectangle(x, y, width, height, rlColor);
}

void R_DrawRectangleLines(int x, int y, int width, int height, color_t color) {
    Color rlColor = { color.r, color.g, color.b, color.a };
    DrawRectangleLines(x, y, width, height, rlColor);
}

font_t R_LoadFont(const char* filename) {
    if (!filename) return NULL;
    
    font_impl_t* fontImpl = (font_impl_t*)I_Malloc(sizeof(font_impl_t));
    fontImpl->font = LoadFont(filename);
    
    if (fontImpl->font.texture.id == 0) {
        I_Warning("Failed to load font: %s", filename);
        I_Free(fontImpl);
        return NULL;
    }
    
    return (font_t)fontImpl;
}

font_t R_LoadFontEx(const char* filename, int fontSize, int* fontChars, int charCount) {
    if (!filename) return NULL;
    
    font_impl_t* fontImpl = (font_impl_t*)I_Malloc(sizeof(font_impl_t));
    fontImpl->font = LoadFontEx(filename, fontSize, fontChars, charCount);
    
    if (fontImpl->font.texture.id == 0) {
        I_Warning("Failed to load font: %s", filename);
        I_Free(fontImpl);
        return NULL;
    }
    
    return (font_t)fontImpl;
}

void R_UnloadFont(font_t font) {
    if (!font) return;
    
    font_impl_t* fontImpl = (font_impl_t*)font;
    UnloadFont(fontImpl->font);
    I_Free(fontImpl);
}

void R_DrawText(const char* text, int x, int y, int fontSize, color_t color) {
    if (!text) return;
    
    Color rlColor = { color.r, color.g, color.b, color.a };
    DrawText(text, x, y, fontSize, rlColor);
}

void R_DrawTextEx(font_t font, const char* text, vector2_t position,
                  float fontSize, float spacing, color_t color) {
    if (!text) return;
    
    Font rlFont = defaultFont;
    if (font) {
        font_impl_t* fontImpl = (font_impl_t*)font;
        rlFont = fontImpl->font;
    }
    
    Vector2 rlPosition = { position.x, position.y };
    Color rlColor = { color.r, color.g, color.b, color.a };
    
    DrawTextEx(rlFont, text, rlPosition, fontSize, spacing, rlColor);
}

int R_MeasureText(const char* text, int fontSize) {
    if (!text) return 0;
    return MeasureText(text, fontSize);
}

vector2_t R_MeasureTextEx(font_t font, const char* text, float fontSize, float spacing) {
    vector2_t result = { 0, 0 };
    if (!text) return result;
    
    Font rlFont = defaultFont;
    if (font) {
        font_impl_t* fontImpl = (font_impl_t*)font;
        rlFont = fontImpl->font;
    }
    
    Vector2 rlSize = MeasureTextEx(rlFont, text, fontSize, spacing);
    result.x = rlSize.x;
    result.y = rlSize.y;
    
    return result;
}

shader_t R_LoadShader(const char* vsFileName, const char* fsFileName) {
    shader_impl_t* shaderImpl = (shader_impl_t*)I_Malloc(sizeof(shader_impl_t));
    shaderImpl->shader = LoadShader(vsFileName, fsFileName);
    
    if (shaderImpl->shader.id == 0) {
        I_Warning("Failed to load shader: %s, %s", 
                  vsFileName ? vsFileName : "default",
                  fsFileName ? fsFileName : "default");
        I_Free(shaderImpl);
        return NULL;
    }
    
    return (shader_t)shaderImpl;
}

shader_t R_LoadShaderFromMemory(const char* vsCode, const char* fsCode) {
    shader_impl_t* shaderImpl = (shader_impl_t*)I_Malloc(sizeof(shader_impl_t));
    shaderImpl->shader = LoadShaderFromMemory(vsCode, fsCode);
    
    if (shaderImpl->shader.id == 0) {
        I_Warning("Failed to load shader from memory");
        I_Free(shaderImpl);
        return NULL;
    }
    
    return (shader_t)shaderImpl;
}

void R_UnloadShader(shader_t shader) {
    if (!shader) return;
    
    shader_impl_t* shaderImpl = (shader_impl_t*)shader;
    UnloadShader(shaderImpl->shader);
    I_Free(shaderImpl);
}

void R_BeginShaderMode(shader_t shader) {
    if (!shader) return;
    
    shader_impl_t* shaderImpl = (shader_impl_t*)shader;
    BeginShaderMode(shaderImpl->shader);
}

void R_EndShaderMode(void) {
    EndShaderMode();
}

void R_SetShaderValue(shader_t shader, int locIndex, const void* value, int uniformType) {
    if (!shader || !value) return;
    
    shader_impl_t* shaderImpl = (shader_impl_t*)shader;
    SetShaderValue(shaderImpl->shader, locIndex, value, uniformType);
}

void R_SetShaderValueV(shader_t shader, int locIndex, const void* value, int uniformType, int count) {
    if (!shader || !value) return;
    
    shader_impl_t* shaderImpl = (shader_impl_t*)shader;
    SetShaderValueV(shaderImpl->shader, locIndex, value, uniformType, count);
}

int R_GetShaderLocation(shader_t shader, const char* uniformName) {
    if (!shader || !uniformName) return -1;
    
    shader_impl_t* shaderImpl = (shader_impl_t*)shader;
    return GetShaderLocation(shaderImpl->shader, uniformName);
}

void R_BeginScissorMode(int x, int y, int width, int height) {
    BeginScissorMode(x, y, width, height);
}

void R_EndScissorMode(void) {
    EndScissorMode();
}

void R_SetLineWidth(float width) {
    rlSetLineWidth(width);
}

uint32_t R_ColorToInt(color_t color) {
    return (color.a << 24) | (color.b << 16) | (color.g << 8) | color.r;
}

color_t R_IntToColor(uint32_t value) {
    color_t color;
    color.r = value & 0xFF;
    color.g = (value >> 8) & 0xFF;
    color.b = (value >> 16) & 0xFF;
    color.a = (value >> 24) & 0xFF;
    return color;
}

color_t R_Fade(color_t color, float alpha) {
    color_t result = color;
    result.a = (uint8_t)(color.a * alpha);
    return result;
}