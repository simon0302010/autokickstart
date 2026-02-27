#ifndef GLOBALS_H
#define GLOBALS_H

#include <gtk/gtk.h>

typedef struct Packages {
    GListStore *packages;
    GtkWidget *multilib;
    GtkWidget *nocore;
} Packages;

typedef struct Script {
    GtkWidget *code;
    GtkWidget *interpreter;
    GtkWidget *nochroot;
} Script;

typedef struct User {
    GtkWidget *box;
    GtkWidget *username;
    GtkWidget *gecos;
    GtkWidget *groups;
    GtkWidget *locked;
    GtkWidget *password;
} User;

typedef struct UserList {
    User **list;
    size_t count;
    size_t capacity;
} UserList;

typedef struct KickstartOptions {
    char *path;
    GtkWidget *root_enabled;
    GtkWidget *root_password;
    GtkWidget *graphics_mode;
    GtkWidget *locale;
    GtkWidget *layout;
    GtkWidget *selinux;
    GtkWidget *clearpart_all;
    GtkWidget *clearpart_linux;
    GtkWidget *clearpart_none;
    GtkWidget *autopart;
    GtkWidget *bootloader_location;
    GtkWidget *bootloader_options;
    GtkWidget *initial_setup;
    GtkWidget *timezone;
    GtkWidget *after_install;
    GtkWidget *disk_label;
    GtkWidget *additional_options;
    Packages packages;
    Script pre_install;
    Script post_install;
    UserList users;
} KickstartOptions;

typedef struct OpenedFile {
    char *path;
    FILE *file;
} OpenedFile;

extern GtkWidget *main_window;
extern GtkWidget *label;

extern OpenedFile ks_file;
extern KickstartOptions options;

#endif
