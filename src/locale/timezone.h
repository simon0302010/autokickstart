#ifndef TIMEZONE_H
#define TIMEZONE_H

#define MAX_ITEMS 2048
#define MAX_STR   128

char **get_timezones(int *num_zones);
int get_timezone_idx(const char *name, const char *alternative);
const char *get_timezone_from_idx(int i);
int get_current_timezone_idx(const char *alternative);
void free_timezones();

#endif