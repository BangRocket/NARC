#ifdef __linux__

#include "../../../include/i_system.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <pthread.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>

void I_PlatformInit(void) {
    I_Log("Linux platform initialized");
}

void I_PlatformShutdown(void) {
    I_Log("Linux platform shutdown");
}

const char* I_GetPlatformName(void) {
    return "Linux";
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
        case 0: {
            const char* xdg_config = getenv("XDG_CONFIG_HOME");
            if (xdg_config) {
                strncpy(buffer, xdg_config, bufferSize - 1);
                buffer[bufferSize - 1] = '\0';
            } else {
                snprintf(buffer, bufferSize, "%s/.config", home);
            }
            break;
        }
        case 1: {
            const char* xdg_documents = getenv("XDG_DOCUMENTS_DIR");
            if (xdg_documents) {
                strncpy(buffer, xdg_documents, bufferSize - 1);
                buffer[bufferSize - 1] = '\0';
            } else {
                snprintf(buffer, bufferSize, "%s/Documents", home);
            }
            break;
        }
        case 2: {
            const char* xdg_desktop = getenv("XDG_DESKTOP_DIR");
            if (xdg_desktop) {
                strncpy(buffer, xdg_desktop, bufferSize - 1);
                buffer[bufferSize - 1] = '\0';
            } else {
                snprintf(buffer, bufferSize, "%s/Desktop", home);
            }
            break;
        }
        default:
            return false;
    }
    
    return true;
}

void I_ShowMessage(const char* title, const char* message, int type) {
    char command[4096];
    const char* icon = "info";
    
    switch (type) {
        case 0: icon = "info"; break;
        case 1: icon = "warning"; break;
        case 2: icon = "error"; break;
    }
    
    if (system("which zenity > /dev/null 2>&1") == 0) {
        snprintf(command, sizeof(command),
                 "zenity --%s --text=\"%s\" --title=\"%s\" 2>/dev/null",
                 icon, message, title);
        system(command);
    } else if (system("which kdialog > /dev/null 2>&1") == 0) {
        snprintf(command, sizeof(command),
                 "kdialog --%s \"%s\" --title \"%s\" 2>/dev/null",
                 type == 2 ? "error" : (type == 1 ? "sorry" : "msgbox"),
                 message, title);
        system(command);
    } else if (system("which notify-send > /dev/null 2>&1") == 0) {
        const char* urgency = type == 2 ? "critical" : (type == 1 ? "normal" : "low");
        snprintf(command, sizeof(command),
                 "notify-send -u %s \"%s\" \"%s\" 2>/dev/null",
                 urgency, title, message);
        system(command);
    } else {
        fprintf(stderr, "%s: %s\n", title, message);
    }
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
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int64_t I_GetHighResFrequency(void) {
    return 1000000000LL;
}

void I_SetThreadName(const char* name) {
    pthread_setname_np(pthread_self(), name);
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
    
    return setpriority(PRIO_PROCESS, 0, nice_value) == 0;
}

void I_GetSystemInfo(char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize == 0) return;
    
    FILE* fp = fopen("/etc/os-release", "r");
    if (fp) {
        char line[256];
        char* name = NULL;
        char* version = NULL;
        
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                char* start = line + 12;
                if (*start == '"') start++;
                char* end = strchr(start, '\n');
                if (end) *end = '\0';
                end = strchr(start, '"');
                if (end) *end = '\0';
                
                strncpy(buffer, start, bufferSize - 1);
                buffer[bufferSize - 1] = '\0';
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }
    
    struct utsname uts;
    if (uname(&uts) == 0) {
        snprintf(buffer, bufferSize, "%s %s", uts.sysname, uts.release);
    } else {
        strncpy(buffer, "Linux (Unknown Distribution)", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
    }
}

#endif