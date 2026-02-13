#ifndef TIMEZONE_H
#define TIMEZONE_H

#define MAX_ITEMS 2048
#define MAX_STR   128

char **get_timezones(int *num_zones);
int get_current_timezone_idx(const char *alternative);
void free_timezones();

#endif