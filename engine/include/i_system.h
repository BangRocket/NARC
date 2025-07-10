#ifndef I_SYSTEM_H
#define I_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void I_Init(void);
void I_Shutdown(void);

void I_Error(const char* error, ...);
void I_Warning(const char* warning, ...);
void I_Log(const char* message, ...);

void* I_Malloc(size_t size);
void* I_Calloc(size_t count, size_t size);
void* I_Realloc(void* ptr, size_t size);
void I_Free(void* ptr);

int I_GetTime(void);
void I_Sleep(int ms);

const char* I_GetBasePath(void);
const char* I_GetPrefPath(void);

bool I_FileExists(const char* filename);
int64_t I_FileSize(const char* filename);
void* I_LoadFile(const char* filename, size_t* size);
bool I_SaveFile(const char* filename, const void* data, size_t size);

bool I_CheckSSE42(void);
int I_GetCPUCount(void);

void I_PlatformInit(void);
void I_PlatformShutdown(void);
const char* I_GetPlatformName(void);

bool I_CreateDirectory(const char* path);
bool I_GetSpecialFolder(int folder, char* buffer, size_t bufferSize);

void I_ShowMessage(const char* title, const char* message, int type);

void* I_LoadLibrary(const char* name);
void* I_GetProcAddress(void* library, const char* name);
void I_UnloadLibrary(void* library);

int64_t I_GetHighResTime(void);
int64_t I_GetHighResFrequency(void);

void I_SetThreadName(const char* name);
bool I_SetProcessPriority(int priority);

void I_GetSystemInfo(char* buffer, size_t bufferSize);

#ifdef __cplusplus
}
#endif

#endif