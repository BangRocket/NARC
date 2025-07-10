#ifdef _WIN32

#include "../../../include/i_system.h"
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#include <io.h>

static HINSTANCE hInstance = NULL;

void I_PlatformInit(void) {
    hInstance = GetModuleHandle(NULL);
    
    SetProcessDPIAware();
    
    UINT codePage = GetConsoleCP();
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    
    I_Log("Windows platform initialized");
}

void I_PlatformShutdown(void) {
    I_Log("Windows platform shutdown");
}

const char* I_GetPlatformName(void) {
    return "Windows";
}

bool I_CreateDirectory(const char* path) {
    return _mkdir(path) == 0 || errno == EEXIST;
}

bool I_GetSpecialFolder(int folder, char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize == 0) return false;
    
    int csidl = 0;
    switch (folder) {
        case 0: csidl = CSIDL_APPDATA; break;
        case 1: csidl = CSIDL_MYDOCUMENTS; break;
        case 2: csidl = CSIDL_DESKTOP; break;
        default: return false;
    }
    
    WCHAR wpath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, csidl, NULL, 0, wpath))) {
        WideCharToMultiByte(CP_UTF8, 0, wpath, -1, buffer, bufferSize, NULL, NULL);
        return true;
    }
    
    return false;
}

void I_ShowMessage(const char* title, const char* message, int type) {
    UINT flags = MB_OK;
    
    switch (type) {
        case 0: flags |= MB_ICONINFORMATION; break;
        case 1: flags |= MB_ICONWARNING; break;
        case 2: flags |= MB_ICONERROR; break;
    }
    
    MessageBoxA(NULL, message, title, flags);
}

void* I_LoadLibrary(const char* name) {
    return (void*)LoadLibraryA(name);
}

void* I_GetProcAddress(void* library, const char* name) {
    return (void*)GetProcAddress((HMODULE)library, name);
}

void I_UnloadLibrary(void* library) {
    if (library) {
        FreeLibrary((HMODULE)library);
    }
}

int64_t I_GetHighResTime(void) {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
}

int64_t I_GetHighResFrequency(void) {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return frequency.QuadPart;
}

void I_SetThreadName(const char* name) {
    typedef HRESULT (WINAPI *SetThreadDescriptionFunc)(HANDLE, PCWSTR);
    static SetThreadDescriptionFunc setThreadDescription = NULL;
    static bool initialized = false;
    
    if (!initialized) {
        HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
        if (kernel32) {
            setThreadDescription = (SetThreadDescriptionFunc)GetProcAddress(kernel32, "SetThreadDescription");
        }
        initialized = true;
    }
    
    if (setThreadDescription) {
        WCHAR wname[256];
        MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 256);
        setThreadDescription(GetCurrentThread(), wname);
    }
}

bool I_SetProcessPriority(int priority) {
    DWORD priorityClass = NORMAL_PRIORITY_CLASS;
    
    switch (priority) {
        case -2: priorityClass = IDLE_PRIORITY_CLASS; break;
        case -1: priorityClass = BELOW_NORMAL_PRIORITY_CLASS; break;
        case 0: priorityClass = NORMAL_PRIORITY_CLASS; break;
        case 1: priorityClass = ABOVE_NORMAL_PRIORITY_CLASS; break;
        case 2: priorityClass = HIGH_PRIORITY_CLASS; break;
    }
    
    return SetPriorityClass(GetCurrentProcess(), priorityClass) != 0;
}

void I_GetSystemInfo(char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize == 0) return;
    
    OSVERSIONINFOEXW osvi = { sizeof(osvi) };
    
    typedef LONG (WINAPI *RtlGetVersionFunc)(PRTL_OSVERSIONINFOW);
    RtlGetVersionFunc rtlGetVersion = (RtlGetVersionFunc)GetProcAddress(
        GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
    
    if (rtlGetVersion) {
        rtlGetVersion((PRTL_OSVERSIONINFOW)&osvi);
        snprintf(buffer, bufferSize, "Windows %lu.%lu Build %lu",
                 osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
    } else {
        strncpy(buffer, "Windows (Unknown Version)", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
    }
}

#endif