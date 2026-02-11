#include <stdio.h>

#include "kickstart.h"
#include "../utils/utils.h"

struct OpenedFile create_temp_ks() {
    struct OpenedFile open_file;

    char *ks_name = rand_str(20);
    if (!ks_name) {
        printf("failed to create random filename for kickstart file\n");
        exit(1);
    }
    char *ks_path = malloc(strlen("/tmp/") + strlen(ks_name) + strlen(".ks") + 1);
    if (!ks_path) {
        printf("failed to allocate string for kickstart file path\n");
        free(ks_name);
        exit(1);
    }
    sprintf(ks_path, "/tmp/%s.ks", ks_name);
    free(ks_name);

    open_file.path = ks_path;
    open_file.file = fopen(ks_path, "w");
    if (open_file.file == NULL) {
        printf("failed to create %s\n", ks_path);
        free(ks_path);
        exit(1);
    }

    return open_file;
}