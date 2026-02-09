#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <gtk/gtk.h>

void seed_rng();
char *rand_str(size_t length);
void show_alert(GtkWidget *window, const char* msg);

#endif