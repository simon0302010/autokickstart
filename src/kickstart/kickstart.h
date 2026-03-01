#ifndef KICKSTART_H
#define KICKSTART_H

#include "globals.h"

int clean_temp_dir();
OpenedFile create_temp_ks();
char *write_ks_from_options();
int download_packages_from_options();
char *download_iso(const char *url);
const char *get_pkg_dir();

#endif