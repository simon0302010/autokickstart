#ifndef ISO_H
#define ISO_H

const char **get_fedora_versions();
const char **get_fedora_architectures();
char *find_fedora_iso();
int download_file(const char *url, const char *dest);
void free_fedora_releases();

#endif