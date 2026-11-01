#include "../../include/common.h"
#include <ctype.h>

bool validation_is_safe_filename(const char* name) {
    if (!name || strlen(name) == 0) return false;
    if (strstr(name, "..") != NULL) return false;
    for (size_t i = 0; name[i] != '\0'; i++) {
        char c = name[i];
        if (c < 32 || c == 127) return false;
    }
    return true;
}

bool validation_is_safe_path(const char* path) {
    if (!path || strlen(path) == 0) return false;
    if (strstr(path, "..\\..") != NULL || strstr(path, "../..") != NULL) return false;
    return true;
}
