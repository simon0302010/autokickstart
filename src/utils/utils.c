#include "glib-object.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <limits.h>
#include <gtk/gtk.h>

void seed_rng() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    uint64_t millis64 = (uint64_t)tp.tv_sec * 1000u + (uint64_t)tp.tv_usec / 1000u;
    unsigned int millis = (unsigned int)(millis64 % (uint64_t)UINT_MAX);
    srand(millis);
}

char *rand_str(size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    char *result = malloc(length + 1);
    if (!result) return NULL;

    for (size_t i = 0; i < length; i++) {
        size_t index = rand() % (sizeof charset - 1);
        result[i] = charset[index];
    }

    result[length] = '\0';
    return result;
}

void show_alert(GtkWidget *window, const char* msg) {
    GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", msg);
    gtk_alert_dialog_set_buttons(dialog, (const char *[]) { "Ok", NULL });
    gtk_alert_dialog_show(dialog, GTK_WINDOW(window));
    g_object_unref(dialog);
}