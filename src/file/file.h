#ifndef FILE_H
#define FILE_H

#include <gtk/gtk.h>

int load_file(const char *path);
int save_file(const char *path);
void on_save_file();
void open_file_dialog(GtkWidget *window);

#endif