#include "../../include/i_system.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#else
#include <unistd.h>
#include <cpuid.h>
#endif

static FILE* logFile = NULL;
static clock_t startTime;

void I_Init(void) {
    startTime = clock();
    logFile = fopen("NARC.log", "w");
    if (logFile) {
        fprintf(logFile, "NARC Engine Initialized\n");
        fprintf(logFile, "Using raylib %s\n", RAYLIB_VERSION);
        fflush(logFile);
    }
}

void I_Shutdown(void) {
    if (logFile) {
        fprintf(logFile, "NARC Engine Shutdown\n");
        fclose(logFile);
        logFile = NULL;
    }
}

void I_Error(const char* error, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, error);
    vsnprintf(buffer, sizeof(buffer), error, args);
    va_end(args);
    
    if (logFile) {
        fprintf(logFile, "ERROR: %s\n", buffer);
        fflush(logFile);
    }
    
    fprintf(stderr, "ERROR: %s\n", buffer);
    
#ifdef _WIN32
    MessageBoxA(NULL, buffer, "NARC Error", MB_OK | MB_ICONERROR);
#endif
    
    exit(1);
}

void I_Warning(const char* warning, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, warning);
    vsnprintf(buffer, sizeof(buffer), warning, args);
    va_end(args);
    
    if (logFile) {
        fprintf(logFile, "WARNING: %s\n", buffer);
        fflush(logFile);
    }
    
    fprintf(stderr, "WARNING: %s\n", buffer);
}

void I_Log(const char* message, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);
    
    if (logFile) {
        fprintf(logFile, "%s\n", buffer);
        fflush(logFile);
    }
    
    printf("%s\n", buffer);
}

void* I_Malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr && size > 0) {
        I_Error("Out of memory (tried to allocate %zu bytes)", size);
    }
    return ptr;
}

void* I_Calloc(size_t count, size_t size) {
    void* ptr = calloc(count, size);
    if (!ptr && count > 0 && size > 0) {
        I_Error("Out of memory (tried to allocate %zu x %zu bytes)", count, size);
    }
    return ptr;
}

void* I_Realloc(void* ptr, size_t size) {
    void* newPtr = realloc(ptr, size);
    if (!newPtr && size > 0) {
        I_Error("Out of memory (tried to reallocate %zu bytes)", size);
    }
    return newPtr;
}

void I_Free(void* ptr) {
    free(ptr);
}

int I_GetTime(void) {
    return (int)((clock() - startTime) * 1000 / CLOCKS_PER_SEC);
}

void I_Sleep(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

const char* I_GetBasePath(void) {
    static char basePath[512] = {0};
    if (basePath[0] == 0) {
        strncpy(basePath, GetApplicationDirectory(), sizeof(basePath) - 1);
    }
    return basePath;
}

const char* I_GetPrefPath(void) {
    static char prefPath[512] = {0};
    if (prefPath[0] == 0) {
#ifdef _WIN32
        char* appData = getenv("APPDATA");
        if (appData) {
            snprintf(prefPath, sizeof(prefPath), "%s/NARC/", appData);
        }
#else
        char* home = getenv("HOME");
        if (home) {
            snprintf(prefPath, sizeof(prefPath), "%s/.narc/", home);
        }
#endif
    }
    return prefPath;
}

bool I_FileExists(const char* filename) {
    return FileExists(filename);
}

int64_t I_FileSize(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return -1;
    
    fseek(file, 0, SEEK_END);
    int64_t size = ftell(file);
    fclose(file);
    
    return size;
}

void* I_LoadFile(const char* filename, size_t* size) {
    unsigned int dataSize = 0;
    unsigned char* data = LoadFileData(filename, &dataSize);
    if (size) *size = dataSize;
    return data;
}

bool I_SaveFile(const char* filename, const void* data, size_t size) {
    return SaveFileData(filename, (void*)data, (unsigned int)size);
}

bool I_CheckSSE42(void) {
#ifdef _WIN32
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 20)) != 0;
#else
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return (ecx & (1 << 20)) != 0;
    }
    return false;
#endif
}

int I_GetCPUCount(void) {
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}