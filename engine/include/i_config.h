#ifndef I_CONFIG_H
#define I_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ini_file_s* ini_file_t;

ini_file_t I_LoadINI(const char* filename);
void I_UnloadINI(ini_file_t ini);

bool I_INIGetString(ini_file_t ini, const char* section, const char* key, 
                    char* buffer, size_t bufferSize, const char* defaultValue);
bool I_INIGetInt(ini_file_t ini, const char* section, const char* key, 
                 int* value, int defaultValue);
bool I_INIGetFloat(ini_file_t ini, const char* section, const char* key, 
                   float* value, float defaultValue);
bool I_INIGetBool(ini_file_t ini, const char* section, const char* key, 
                  bool* value, bool defaultValue);
bool I_INIGetColor(ini_file_t ini, const char* section, const char* key,
                   uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a,
                   uint8_t defaultR, uint8_t defaultG, uint8_t defaultB, uint8_t defaultA);

bool I_INISetString(ini_file_t ini, const char* section, const char* key, const char* value);
bool I_INISetInt(ini_file_t ini, const char* section, const char* key, int value);
bool I_INISetFloat(ini_file_t ini, const char* section, const char* key, float value);
bool I_INISetBool(ini_file_t ini, const char* section, const char* key, bool value);
bool I_INISetColor(ini_file_t ini, const char* section, const char* key,
                   uint8_t r, uint8_t g, uint8_t b, uint8_t a);

bool I_INISave(ini_file_t ini, const char* filename);

const char** I_INIGetSections(ini_file_t ini, int* count);
const char** I_INIGetKeys(ini_file_t ini, const char* section, int* count);
void I_INIFreeStringArray(const char** array);

bool I_LoadConfigFile(const char* filename, char** buffer, size_t* size);
const char* I_GetConfigLine(const char** buffer);

#ifdef __cplusplus
}
#endif

#endif