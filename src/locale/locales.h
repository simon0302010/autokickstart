#ifndef LOCALE_H
#define LOCALE_H

const char *get_locale(const char* name);
const char **get_names();
int get_current_system_locale_index(const char *alternative);

#endif