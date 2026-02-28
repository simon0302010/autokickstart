#define _GNU_SOURCE
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <ftw.h>

#include "kickstart.h"
#include "utils/utils.h"
#include "globals.h"
#include "locale/locales.h"
#include "locale/kb.h"
#include "locale/timezone.h"

const unsigned int FEDORA_VERSION = 43;

char pkg_dir[PATH_MAX];
char dnf_dir[PATH_MAX];
char temp_dir[PATH_MAX];
OpenedFile ks_file;

const char *get_temp_dir() {
    if (temp_dir[0] != '\0') return temp_dir;

    if (getcwd(temp_dir, sizeof(temp_dir)) == NULL) return NULL;

    char *folder_name = rand_str(20);
    if (!folder_name) return NULL;
    strncat(temp_dir, "/AUTOKS_", PATH_MAX - strlen(temp_dir) - 2);
    strncat(temp_dir, folder_name, PATH_MAX - strlen(temp_dir) - 1);
    free(folder_name);

    if (mkdir(temp_dir, 0777) != 0) {
        perror("mkdir");
    }

    return temp_dir;
}

const char *get_pkg_dir() {
    if (pkg_dir[0] != '\0') return pkg_dir;
    snprintf(pkg_dir, sizeof(pkg_dir), "%s/pkg", get_temp_dir());
    return pkg_dir;
}

const char *get_dnf_dir() {
    if (dnf_dir[0] != '\0') return dnf_dir;
    snprintf(dnf_dir, sizeof(dnf_dir), "%s/dnf", get_temp_dir());
    return dnf_dir;
}

static int remove_file(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (remove(fpath) == -1) {
        perror("remove");
        return 1;
    }
    return 0;
}

int clean_temp_dir() {
    if (temp_dir[0] == '\0') return 1;
    
    if (nftw(temp_dir, remove_file, 64, FTW_DEPTH | FTW_PHYS) == -1) {
        perror("nftw");
    }
    
    memset(temp_dir, '\0', sizeof(temp_dir));
    memset(pkg_dir, '\0', sizeof(pkg_dir));
    memset(dnf_dir, '\0', sizeof(dnf_dir));
    return 0;
}

OpenedFile create_temp_ks() {
    OpenedFile open_file;

    char *ks_name = rand_str(20);
    if (!ks_name) {
        printf("failed to create random filename for kickstart file\n");
        exit(1);
    }
    char *ks_path = malloc(strlen("/tmp/") + strlen(ks_name) + strlen(".ks") + 1);
    if (!ks_path) {
        printf("failed to allocate string for kickstart file path\n");
        free(ks_name);
        exit(1);
    }
    sprintf(ks_path, "/tmp/%s.ks", ks_name);
    free(ks_name);

    open_file.path = ks_path;
    open_file.file = fopen(ks_path, "w");
    if (open_file.file == NULL) {
        printf("failed to create %s\n", ks_path);
        free(ks_path);
        exit(1);
    }

    return open_file;
}

char *write_ks_from_options() {
    ks_file = create_temp_ks();

    GtkTextIter start, end;

    char *graphics_mode = (char *)gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.graphics_mode))));
    for (char *p = graphics_mode; *p; ++p) *p = tolower(*p);
    fprintf(ks_file.file, "%s\n", graphics_mode);

    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.root_enabled))) {
        const char *rootpw = gtk_editable_get_text(GTK_EDITABLE(options.root_password));
        fprintf(ks_file.file, "rootpw --plaintext %s\n", rootpw);
    } else {
        fprintf(ks_file.file, "rootpw --lock\n");
    }

    const char *lang_str = get_locale_id(gtk_drop_down_get_selected(GTK_DROP_DOWN(options.locale)));
    fprintf(ks_file.file, "lang %s\n", lang_str);

    const char *layout_str = get_layout_id(gtk_drop_down_get_selected(GTK_DROP_DOWN(options.layout)));
    fprintf(ks_file.file, "keyboard --xlayouts=%s\n", layout_str);

    const char *timezone_str = get_timezone_from_idx(gtk_drop_down_get_selected(GTK_DROP_DOWN(options.timezone)));
    fprintf(ks_file.file, "timezone %s --utc\n", timezone_str);

    char *selinux_mode = (char *)gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.selinux))));
    for (char *p = selinux_mode; *p; ++p) *p = tolower(*p);
    fprintf(ks_file.file, "selinux --%s\n", selinux_mode);

    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.autopart))) {
        fprintf(ks_file.file, "autopart\n");
    }

    char clearpart[6];
    get_selected_clearpart(clearpart);
    fprintf(ks_file.file, "clearpart --%s\n", clearpart);

    char *bootloader_location = (char *)gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.bootloader_location))));
    for (char *p = bootloader_location; *p; ++p) *p = tolower(*p);
    const char *bootloader_options = gtk_editable_get_text(GTK_EDITABLE(options.bootloader_options));
    if (strlen(bootloader_options) == 0) {
        fprintf(ks_file.file, "bootloader --location=%s\n", bootloader_location);
    } else {
        fprintf(ks_file.file, "bootloader --location=%s --append=\"%s\"\n", bootloader_location, bootloader_options);
    }

    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.initial_setup))) {
        fprintf(ks_file.file, "firstboot --enable\n");
    } else {
        fprintf(ks_file.file, "firstboot --disable\n");
    }

    int after_install_idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(options.after_install));
    const char *after_install_options[] = {"halt", "poweroff", "reboot", "reboot --eject", "shutdown"};
    fprintf(ks_file.file, "%s\n", after_install_options[after_install_idx]);

    GtkTextBuffer *additional_options_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(options.additional_options));
    gtk_text_buffer_get_bounds(additional_options_buffer, &start, &end);
    char *additional_options = gtk_text_buffer_get_text(additional_options_buffer, &start, &end, FALSE);
    fprintf(ks_file.file, "%s\n", additional_options);

    // packages
    fprintf(ks_file.file, "%%packages");
    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.packages.multilib))) {
        fprintf(ks_file.file, " --multilib");
    }
    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.packages.nocore))) {
        fprintf(ks_file.file, " --nocore");
    }
    fprintf(ks_file.file, "\n");
    guint packages_count = g_list_model_get_n_items(G_LIST_MODEL(options.packages.packages));
    for (guint i = 0; i < packages_count; i++) {
        GtkStringObject *pkg = GTK_STRING_OBJECT(g_list_model_get_item(G_LIST_MODEL(options.packages.packages), i));
        fprintf(ks_file.file, "%s\n", gtk_string_object_get_string(pkg));
        g_object_unref(pkg);
    }
    fprintf(ks_file.file, "%%end\n");

    // users (TODO: remember to check if username is empty)
    for (size_t i = 0; i < options.users.count; i++) {
        User *user = options.users.list[i];
        const char *username = gtk_editable_get_text(GTK_EDITABLE(user->username));

        if (strcmp(username, "") == 0) {
            g_print("username field is empty\n");
            continue;
        }

        const char *password = gtk_editable_get_text(GTK_EDITABLE(user->password));
        const char *groups = gtk_editable_get_text(GTK_EDITABLE(user->groups));
        const char *gecos = gtk_editable_get_text(GTK_EDITABLE(user->gecos));
        bool locked = gtk_check_button_get_active(GTK_CHECK_BUTTON(user->locked));

        char username_esc[strlen(username) * 2 + 1];
        char password_esc[strlen(password) * 2 + 1];
        char groups_esc[strlen(groups) * 2 + 1];
        char gecos_esc[strlen(gecos) * 2 + 1];

        escape_quotes(username, username_esc);
        escape_quotes(password, password_esc);
        escape_quotes(groups, groups_esc);
        escape_quotes(gecos, gecos_esc);

        fprintf(
            ks_file.file,
            "user --name=\"%s\" --password=\"%s\"",
            username_esc, password_esc
        );

        if (strcmp(groups, "") != 0) {
            fprintf(ks_file.file, " --groups=\"%s\"", groups_esc);
        }
        if (strcmp(gecos, "") != 0) {
            fprintf(ks_file.file, " --gecos=\"%s\"", gecos_esc);
        }
        if (locked) {
            fprintf(ks_file.file, " --lock");
        }

        fprintf(ks_file.file, "\n");
    }

    // configuring local repo
    const char *disk_label = gtk_editable_get_text(GTK_EDITABLE(options.disk_label));
    fprintf(ks_file.file, "harddrive --partition=LABEL=%s --dir=/packages/\n", (disk_label && strlen(disk_label) > 0) ? disk_label : "Fedora-Autokickstart");
    fprintf(ks_file.file, "repo --name=local-packages --baseurl=file:///run/install/repo/packages/\n");

    // pre install script
    fprintf(ks_file.file, "%%pre");
    fprintf(ks_file.file, " --interpreter=%s\n", gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.pre_install.interpreter)))));

    GtkTextBuffer *pre_install_script_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(options.pre_install.code));
    gtk_text_buffer_get_bounds(pre_install_script_buffer, &start, &end);
    fprintf(ks_file.file, "%s\n", gtk_text_buffer_get_text(pre_install_script_buffer, &start, &end, FALSE));

    fprintf(ks_file.file, "%%end\n");

    // post install script
    fprintf(ks_file.file, "%%post");
    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(options.post_install.nochroot))) {
        fprintf(ks_file.file, " --nochroot");
    }
    fprintf(ks_file.file, " --interpreter=%s\n", gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.post_install.interpreter)))));

    GtkTextBuffer *post_install_script_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(options.post_install.code));
    gtk_text_buffer_get_bounds(post_install_script_buffer, &start, &end);
    fprintf(ks_file.file, "%s\n", gtk_text_buffer_get_text(post_install_script_buffer, &start, &end, FALSE));

    fprintf(ks_file.file, "%%end\n");

    if (ks_file.file != NULL) {
        fclose(ks_file.file);
    }

    return ks_file.path;
}

// TODO: fetch fedora version
int download_packages_from_options() {
    if (!is_fedora()) return 2;

    if (mkdir(get_pkg_dir(), 0755) != 0 && errno != EEXIST) {
        perror("mkdir pkg_dir");
        return 1;
    }
    if (mkdir(get_dnf_dir(), 0755) != 0 && errno != EEXIST) {
        perror("mkdir dnf_dir");
        return 1;
    }

    g_print("pkg dir: %s", get_pkg_dir());
    g_print("dnf dir: %s", get_dnf_dir());

    GString *packages_str = g_string_new("");

    char *dnf_base_cmd = NULL;
    asprintf(
        &dnf_base_cmd,
        "dnf install --downloadonly --installroot=%s --use-host-config --releasever=%u --forcearch=%s --setopt=keepcache=True -y",
        get_dnf_dir(),
        FEDORA_VERSION,
        gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.arch))))
    );

    g_string_append(packages_str, dnf_base_cmd);
    g_string_append(packages_str, " @core");

    guint packages_count = g_list_model_get_n_items(G_LIST_MODEL(options.packages.packages));
    for (guint i = 0; i < packages_count; i++) {
        GtkStringObject *pkg = GTK_STRING_OBJECT(g_list_model_get_item(G_LIST_MODEL(options.packages.packages), i));
        g_string_append_printf(packages_str, " %s", gtk_string_object_get_string(pkg));
        g_object_unref(pkg);
    }

    // TODO: improve error handling

    if (system(packages_str->str) != 0) {
        perror("system");
    }

    char *cpy_pkg_cmd = NULL;
    asprintf(&cpy_pkg_cmd, "find \"%s\" -type f -name '*.rpm' -exec cp -n -t \"%s\" {} +", get_dnf_dir(), get_pkg_dir());
    if (system(cpy_pkg_cmd) != 0) {
        perror("system");
    }
    free(cpy_pkg_cmd);

    char *cpy_comps_cmd = NULL;
    asprintf(&cpy_comps_cmd, "find /var/cache/libdnf5 /var/cache/dnf -path \"*/fedora-*/*-comps-*.xml.zst\" -exec zstd -d -f {} -o \"%s/comps.xml\" \\;", get_temp_dir());
    if (system(cpy_comps_cmd) != 0) {
        perror("system");
    }
    free(cpy_comps_cmd);

    char *createrepo_c_cmd = NULL;
    asprintf(&createrepo_c_cmd, "createrepo_c -g \"%s/comps.xml\" \"%s/\"", get_temp_dir(), get_pkg_dir());
    if (system(createrepo_c_cmd) != 0) {
        perror("system");
    }
    free(createrepo_c_cmd);

    g_string_free(packages_str, TRUE);
    free(dnf_base_cmd);

    return 0;
}

