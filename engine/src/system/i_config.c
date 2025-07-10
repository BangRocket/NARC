#include "../../include/i_config.h"
#include "../../include/i_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct ini_entry_s {
    char* key;
    char* value;
    struct ini_entry_s* next;
} ini_entry_t;

typedef struct ini_section_s {
    char* name;
    ini_entry_t* entries;
    struct ini_section_s* next;
} ini_section_t;

typedef struct ini_file_s {
    ini_section_t* sections;
} ini_file_impl_t;

static char* trim_whitespace(char* str) {
    char* end;
    
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;
    
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
    
    return str;
}

static char* duplicate_string(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char* copy = (char*)I_Malloc(len);
    memcpy(copy, str, len);
    return copy;
}

static ini_section_t* find_section(ini_file_impl_t* ini, const char* name) {
    ini_section_t* section = ini->sections;
    while (section) {
        if (strcmp(section->name, name) == 0) {
            return section;
        }
        section = section->next;
    }
    return NULL;
}

static ini_entry_t* find_entry(ini_section_t* section, const char* key) {
    ini_entry_t* entry = section->entries;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

static ini_section_t* add_section(ini_file_impl_t* ini, const char* name) {
    ini_section_t* section = (ini_section_t*)I_Calloc(1, sizeof(ini_section_t));
    section->name = duplicate_string(name);
    
    if (!ini->sections) {
        ini->sections = section;
    } else {
        ini_section_t* last = ini->sections;
        while (last->next) last = last->next;
        last->next = section;
    }
    
    return section;
}

static ini_entry_t* add_entry(ini_section_t* section, const char* key, const char* value) {
    ini_entry_t* entry = (ini_entry_t*)I_Calloc(1, sizeof(ini_entry_t));
    entry->key = duplicate_string(key);
    entry->value = duplicate_string(value);
    
    if (!section->entries) {
        section->entries = entry;
    } else {
        ini_entry_t* last = section->entries;
        while (last->next) last = last->next;
        last->next = entry;
    }
    
    return entry;
}

ini_file_t I_LoadINI(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        I_Warning("Failed to open INI file: %s", filename);
        return NULL;
    }
    
    ini_file_impl_t* ini = (ini_file_impl_t*)I_Calloc(1, sizeof(ini_file_impl_t));
    ini_section_t* currentSection = NULL;
    
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = trim_whitespace(line);
        
        if (*trimmed == '\0' || *trimmed == ';' || *trimmed == '#') {
            continue;
        }
        
        if (*trimmed == '[') {
            char* end = strchr(trimmed, ']');
            if (end) {
                *end = '\0';
                char* sectionName = trim_whitespace(trimmed + 1);
                currentSection = find_section(ini, sectionName);
                if (!currentSection) {
                    currentSection = add_section(ini, sectionName);
                }
            }
        } else if (currentSection) {
            char* equals = strchr(trimmed, '=');
            if (equals) {
                *equals = '\0';
                char* key = trim_whitespace(trimmed);
                char* value = trim_whitespace(equals + 1);
                
                if (*value == '"' && value[strlen(value) - 1] == '"') {
                    value++;
                    value[strlen(value) - 1] = '\0';
                }
                
                ini_entry_t* existing = find_entry(currentSection, key);
                if (existing) {
                    I_Free(existing->value);
                    existing->value = duplicate_string(value);
                } else {
                    add_entry(currentSection, key, value);
                }
            }
        }
    }
    
    fclose(file);
    return (ini_file_t)ini;
}

void I_UnloadINI(ini_file_t ini) {
    if (!ini) return;
    
    ini_file_impl_t* impl = (ini_file_impl_t*)ini;
    ini_section_t* section = impl->sections;
    
    while (section) {
        ini_section_t* nextSection = section->next;
        ini_entry_t* entry = section->entries;
        
        while (entry) {
            ini_entry_t* nextEntry = entry->next;
            I_Free(entry->key);
            I_Free(entry->value);
            I_Free(entry);
            entry = nextEntry;
        }
        
        I_Free(section->name);
        I_Free(section);
        section = nextSection;
    }
    
    I_Free(impl);
}

bool I_INIGetString(ini_file_t ini, const char* section, const char* key,
                    char* buffer, size_t bufferSize, const char* defaultValue) {
    if (!ini || !section || !key || !buffer || bufferSize == 0) {
        if (buffer && bufferSize > 0) {
            strncpy(buffer, defaultValue ? defaultValue : "", bufferSize - 1);
            buffer[bufferSize - 1] = '\0';
        }
        return false;
    }
    
    ini_file_impl_t* impl = (ini_file_impl_t*)ini;
    ini_section_t* sec = find_section(impl, section);
    if (!sec) {
        strncpy(buffer, defaultValue ? defaultValue : "", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
        return false;
    }
    
    ini_entry_t* entry = find_entry(sec, key);
    if (!entry) {
        strncpy(buffer, defaultValue ? defaultValue : "", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
        return false;
    }
    
    strncpy(buffer, entry->value, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';
    return true;
}

bool I_INIGetInt(ini_file_t ini, const char* section, const char* key,
                 int* value, int defaultValue) {
    char buffer[64];
    if (!I_INIGetString(ini, section, key, buffer, sizeof(buffer), NULL)) {
        *value = defaultValue;
        return false;
    }
    
    char* endptr;
    long result = strtol(buffer, &endptr, 10);
    if (*endptr != '\0') {
        *value = defaultValue;
        return false;
    }
    
    *value = (int)result;
    return true;
}

bool I_INIGetFloat(ini_file_t ini, const char* section, const char* key,
                   float* value, float defaultValue) {
    char buffer[64];
    if (!I_INIGetString(ini, section, key, buffer, sizeof(buffer), NULL)) {
        *value = defaultValue;
        return false;
    }
    
    char* endptr;
    float result = strtof(buffer, &endptr);
    if (*endptr != '\0') {
        *value = defaultValue;
        return false;
    }
    
    *value = result;
    return true;
}

bool I_INIGetBool(ini_file_t ini, const char* section, const char* key,
                  bool* value, bool defaultValue) {
    char buffer[64];
    if (!I_INIGetString(ini, section, key, buffer, sizeof(buffer), NULL)) {
        *value = defaultValue;
        return false;
    }
    
    if (strcmp(buffer, "true") == 0 || strcmp(buffer, "1") == 0 || 
        strcmp(buffer, "yes") == 0 || strcmp(buffer, "on") == 0) {
        *value = true;
        return true;
    } else if (strcmp(buffer, "false") == 0 || strcmp(buffer, "0") == 0 || 
               strcmp(buffer, "no") == 0 || strcmp(buffer, "off") == 0) {
        *value = false;
        return true;
    }
    
    *value = defaultValue;
    return false;
}

bool I_INIGetColor(ini_file_t ini, const char* section, const char* key,
                   uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a,
                   uint8_t defaultR, uint8_t defaultG, uint8_t defaultB, uint8_t defaultA) {
    char buffer[64];
    if (!I_INIGetString(ini, section, key, buffer, sizeof(buffer), NULL)) {
        *r = defaultR;
        *g = defaultG;
        *b = defaultB;
        *a = defaultA;
        return false;
    }
    
    int ir, ig, ib, ia = 255;
    int count = sscanf(buffer, "%d,%d,%d,%d", &ir, &ig, &ib, &ia);
    if (count < 3) {
        *r = defaultR;
        *g = defaultG;
        *b = defaultB;
        *a = defaultA;
        return false;
    }
    
    *r = (uint8_t)(ir < 0 ? 0 : (ir > 255 ? 255 : ir));
    *g = (uint8_t)(ig < 0 ? 0 : (ig > 255 ? 255 : ig));
    *b = (uint8_t)(ib < 0 ? 0 : (ib > 255 ? 255 : ib));
    *a = (uint8_t)(ia < 0 ? 0 : (ia > 255 ? 255 : ia));
    return true;
}

bool I_INISetString(ini_file_t ini, const char* section, const char* key, const char* value) {
    if (!ini || !section || !key) return false;
    
    ini_file_impl_t* impl = (ini_file_impl_t*)ini;
    ini_section_t* sec = find_section(impl, section);
    if (!sec) {
        sec = add_section(impl, section);
    }
    
    ini_entry_t* entry = find_entry(sec, key);
    if (entry) {
        I_Free(entry->value);
        entry->value = duplicate_string(value);
    } else {
        add_entry(sec, key, value);
    }
    
    return true;
}

bool I_INISetInt(ini_file_t ini, const char* section, const char* key, int value) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d", value);
    return I_INISetString(ini, section, key, buffer);
}

bool I_INISetFloat(ini_file_t ini, const char* section, const char* key, float value) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.6f", value);
    return I_INISetString(ini, section, key, buffer);
}

bool I_INISetBool(ini_file_t ini, const char* section, const char* key, bool value) {
    return I_INISetString(ini, section, key, value ? "true" : "false");
}

bool I_INISetColor(ini_file_t ini, const char* section, const char* key,
                   uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d,%d,%d,%d", r, g, b, a);
    return I_INISetString(ini, section, key, buffer);
}

bool I_INISave(ini_file_t ini, const char* filename) {
    if (!ini || !filename) return false;
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        I_Warning("Failed to open INI file for writing: %s", filename);
        return false;
    }
    
    ini_file_impl_t* impl = (ini_file_impl_t*)ini;
    ini_section_t* section = impl->sections;
    
    while (section) {
        fprintf(file, "[%s]\n", section->name);
        
        ini_entry_t* entry = section->entries;
        while (entry) {
            fprintf(file, "%s=%s\n", entry->key, entry->value);
            entry = entry->next;
        }
        
        fprintf(file, "\n");
        section = section->next;
    }
    
    fclose(file);
    return true;
}

const char** I_INIGetSections(ini_file_t ini, int* count) {
    if (!ini) {
        if (count) *count = 0;
        return NULL;
    }
    
    ini_file_impl_t* impl = (ini_file_impl_t*)ini;
    int sectionCount = 0;
    ini_section_t* section = impl->sections;
    
    while (section) {
        sectionCount++;
        section = section->next;
    }
    
    if (count) *count = sectionCount;
    if (sectionCount == 0) return NULL;
    
    const char** sections = (const char**)I_Malloc((sectionCount + 1) * sizeof(char*));
    section = impl->sections;
    int i = 0;
    
    while (section) {
        sections[i++] = section->name;
        section = section->next;
    }
    sections[i] = NULL;
    
    return sections;
}

const char** I_INIGetKeys(ini_file_t ini, const char* sectionName, int* count) {
    if (!ini || !sectionName) {
        if (count) *count = 0;
        return NULL;
    }
    
    ini_file_impl_t* impl = (ini_file_impl_t*)ini;
    ini_section_t* section = find_section(impl, sectionName);
    if (!section) {
        if (count) *count = 0;
        return NULL;
    }
    
    int keyCount = 0;
    ini_entry_t* entry = section->entries;
    
    while (entry) {
        keyCount++;
        entry = entry->next;
    }
    
    if (count) *count = keyCount;
    if (keyCount == 0) return NULL;
    
    const char** keys = (const char**)I_Malloc((keyCount + 1) * sizeof(char*));
    entry = section->entries;
    int i = 0;
    
    while (entry) {
        keys[i++] = entry->key;
        entry = entry->next;
    }
    keys[i] = NULL;
    
    return keys;
}

void I_INIFreeStringArray(const char** array) {
    if (array) {
        I_Free(array);
    }
}

bool I_LoadConfigFile(const char* filename, char** buffer, size_t* size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        I_Warning("Failed to open config file: %s", filename);
        return false;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (fileSize <= 0) {
        fclose(file);
        return false;
    }
    
    *buffer = (char*)I_Malloc(fileSize + 1);
    size_t readSize = fread(*buffer, 1, fileSize, file);
    (*buffer)[readSize] = '\0';
    
    if (size) *size = readSize;
    
    fclose(file);
    return true;
}

const char* I_GetConfigLine(const char** buffer) {
    if (!buffer || !*buffer || **buffer == '\0') return NULL;
    
    const char* start = *buffer;
    const char* end = strchr(start, '\n');
    
    if (end) {
        size_t len = end - start;
        char* line = (char*)I_Malloc(len + 1);
        memcpy(line, start, len);
        line[len] = '\0';
        
        *buffer = end + 1;
        if (**buffer == '\r') (*buffer)++;
        
        return line;
    } else {
        size_t len = strlen(start);
        if (len == 0) return NULL;
        
        char* line = duplicate_string(start);
        *buffer = start + len;
        return line;
    }
}