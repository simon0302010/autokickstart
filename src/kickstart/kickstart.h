#ifndef KICKSTART_H
#define KICKSTART_H

#include "../globals.h"

int clean_temp_dir();
OpenedFile create_temp_ks();
char *write_ks_from_options();
int download_packages_from_options();

#endif