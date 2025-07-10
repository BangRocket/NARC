#ifdef __APPLE__

#include "../../../include/i_system.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

static mach_timebase_info_data_t timebase = { 0 };

void I_PlatformInit(void) {
    mach_timebase_info(&timebase);
    
    I_Log("macOS platform initialized");
}

void I_PlatformShutdown(void) {
    I_Log("macOS platform shutdown");
}

const char* I_GetPlatformName(void) {
    return "macOS";
}

bool I_CreateDirectory(const char* path) {
    return mkdir(path, 0755) == 0 || errno == EEXIST;
}

bool I_GetSpecialFolder(int folder, char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize == 0) return false;
    
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }
    
    if (!home) return false;
    
    switch (folder) {
        case 0:
            snprintf(buffer, bufferSize, "%s/Library/Application Support", home);
            break;
        case 1:
            snprintf(buffer, bufferSize, "%s/Documents", home);
            break;
        case 2:
            snprintf(buffer, bufferSize, "%s/Desktop", home);
            break;
        default:
            return false;
    }
    
    return true;
}

void I_ShowMessage(const char* title, const char* message, int type) {
    char command[4096];
    const char* icon = "note";
    
    switch (type) {
        case 0: icon = "note"; break;
        case 1: icon = "caution"; break;
        case 2: icon = "stop"; break;
    }
    
    snprintf(command, sizeof(command),
             "osascript -e 'display dialog \"%s\" with title \"%s\" with icon %s buttons {\"OK\"} default button \"OK\"'",
             message, title, icon);
    
    system(command);
}

void* I_LoadLibrary(const char* name) {
    return dlopen(name, RTLD_LAZY);
}

void* I_GetProcAddress(void* library, const char* name) {
    return dlsym(library, name);
}

void I_UnloadLibrary(void* library) {
    if (library) {
        dlclose(library);
    }
}

int64_t I_GetHighResTime(void) {
    return mach_absolute_time();
}

int64_t I_GetHighResFrequency(void) {
    return 1000000000LL * timebase.denom / timebase.numer;
}

void I_SetThreadName(const char* name) {
    pthread_setname_np(name);
}

bool I_SetProcessPriority(int priority) {
    int nice_value = 0;
    
    switch (priority) {
        case -2: nice_value = 19; break;
        case -1: nice_value = 10; break;
        case 0: nice_value = 0; break;
        case 1: nice_value = -10; break;
        case 2: nice_value = -20; break;
    }
    
    return nice(nice_value) != -1;
}

void I_GetSystemInfo(char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize == 0) return;
    
    char osversion[256] = {0};
    size_t size = sizeof(osversion);
    
    if (sysctlbyname("kern.osproductversion", osversion, &size, NULL, 0) == 0) {
        char osname[256] = {0};
        size = sizeof(osname);
        sysctlbyname("kern.osversion", osname, &size, NULL, 0);
        
        snprintf(buffer, bufferSize, "macOS %s (%s)", osversion, osname);
    } else {
        strncpy(buffer, "macOS (Unknown Version)", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
    }
}

#endif