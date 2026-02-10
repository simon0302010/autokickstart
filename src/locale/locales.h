#ifndef LOCALE_H
#define LOCALE_H

const char *get_locale(const char* name);
const char **get_locale_names();
const char *get_locale_id(int index);
int get_current_system_locale_index(const char *alternative);
const char *find_locale_full_id_string(const char *search);

#endif