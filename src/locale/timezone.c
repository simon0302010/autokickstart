#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "timezone.h"

static const char *base_path = "/usr/share/zoneinfo/posix";
static int count = 0;
static char **timezones;

void free_timezones() {
    if (timezones) {
        for (int i = 0; i < count; i++) {
            free(timezones[i]);
        }
        free(timezones);
        timezones = NULL;
        count = 0;
    }
}

static int callback(const char *fpath, const struct stat *sb, 
                   int typeflag, struct FTW *ftwbuf) {
    (void)sb;
    (void)ftwbuf;
    
    if (typeflag == FTW_F) {
        const char *relative = fpath + strlen(base_path);
        if (*relative == '/') {
            relative++;
        }
        
        if (count >= MAX_ITEMS) return 1;
        
        if (*relative != '\0') {
            timezones[count] = strdup(relative);
            if (!timezones[count]) return 1;
            count++;
        }
    }
    return 0;
}

char **get_timezones(int *num_zones) {
    if (timezones) {
        if (num_zones) *num_zones = count;
        return timezones;
    }

    if (num_zones) *num_zones = 0;

    count = 0;
    timezones = malloc(MAX_ITEMS * sizeof(char *));
    if (!timezones) return NULL;
    
    if (nftw(base_path, callback, 20, FTW_PHYS) != 0) {
        free_timezones();
        return NULL;
    }

    char **tmp = realloc(timezones, count * sizeof(char *));
    if (tmp) timezones = tmp;

    if (num_zones) *num_zones = count;
    return timezones;
}

const char *get_current_timezone() {
    char buf[512];
    int rl_count = readlink("/etc/localtime", buf, sizeof(buf) - 1);
    if (rl_count == -1) {
        return NULL;
    }
    buf[rl_count] = '\0';

    const char *zoneinfo = "zoneinfo/";
    char *pos = strstr(buf, zoneinfo);
    if (pos == NULL) {
        return NULL;
    }

    char *timezone_str = malloc(strlen(pos + strlen(zoneinfo)) + 1);
    if (!timezone_str) {
        return NULL;
    }

    strcpy(timezone_str, pos + strlen(zoneinfo));
    return timezone_str;
}

int get_timezone_idx(const char *name, const char *alternative) {
    for (int i = 0; i < count; i++) {
        if (strcmp(timezones[i], name) == 0) {
            return i;
        }
    }
    if (alternative) {
        for (int i = 0; i < count; i++) {
            if (strcmp(timezones[i], alternative) == 0) {
                return i;
            }
        }
    }
    return -1;
}

const char *get_timezone_from_idx(int i) {
    if (i >= 0 && i < count) {
        return timezones[i];
    } else {
        return NULL;
    }
}

int get_current_timezone_idx(const char *alternative) {
    const char *tz = get_current_timezone();
    if (tz) {
        int result = get_timezone_idx(tz, alternative);
        free((char *)tz);
        return result;
    } else {
        return get_timezone_idx("UTC", NULL);
    }
}