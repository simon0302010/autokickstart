#include "glib-object.h"
#include "globals.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void get_selected_clearpart(char *dest) {
    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.clearpart_all))) {
        strcpy(dest, "all");
    } else if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.clearpart_linux))) {
        strcpy(dest, "linux");
    } else {
        strcpy(dest, "none");
    }
}

int set_selected_clearpart(const char *option) {
    if (strcmp(option, "all") == 0) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(options.clearpart_all), TRUE);
        return 1;
    } else if (strcmp(option, "linux") == 0) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(options.clearpart_linux), TRUE);
        return 1;
    } else if (strcmp(option, "none") == 0) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(options.clearpart_none), TRUE);
        return 1;
    }
    return 0;
}

GtkWidget *create_label(const char *text) {
    label = gtk_label_new(text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    return label;
}

bool is_fedora() {
    FILE *os_release = fopen("/etc/os-release", "r");

    if (os_release == NULL) {
        return false;
    }

    char string[50];
    while (fscanf(os_release, "%49s", string) == 1) {
        if (strcasestr(string, "fedora")) {
            fclose(os_release);
            return true;
        }
    }

    fclose(os_release);
    return false;
}