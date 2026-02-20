#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <gtk/gtk.h>

void seed_rng();
char *rand_str(size_t length);
void show_alert(GtkWidget *window, const char* msg);
void get_selected_clearpart(char *dest);
int set_selected_clearpart(const char *option);
GtkWidget *create_label(const char *text);

#endif