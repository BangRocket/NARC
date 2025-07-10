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

#ifdef __cplusplus
}
#endif

#endif