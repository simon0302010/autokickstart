#ifndef ISO_H
#define ISO_H

#include <stdbool.h>

const char **get_fedora_versions();
const char **get_fedora_architectures();
char *find_fedora_iso();
int download_file(const char *url, const char *dest);
void free_fedora_releases();
int create_iso(const char *ks_path, const char *input_iso, const char *output_iso, bool overwrite);

#endif