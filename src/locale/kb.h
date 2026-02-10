#ifndef KB_H
#define KB_H

const char *get_layout_name(const char* name);
const char **get_layout_names();
int find_layout_index(const char *search);
int find_current_system_layout_index(const char *alternative);

#endif