#include "../../include/common.h"

void time_get_iso8601(char* out_buf, size_t max_len) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    if (tm_info) {
        strftime(out_buf, max_len, "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        snprintf(out_buf, max_len, "UNKNOWN_TIME");
    }
}

void id_generate(const char* prefix, char* out_buf, size_t max_len) {
    static uint32_t counter = 1000;
    time_t now = time(NULL);
    snprintf(out_buf, max_len, "%s-%lx-%04x", prefix ? prefix : "exe", (unsigned long)now, ++counter);
}
