#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <gtk/gtk.h>
#include <string.h>

#include "cJSON/cJSON.h"
#include "globals.h"
#include "kickstart/kickstart.h"
#include "utils/utils.h"

#define MAX_VERSIONS 64
#define FEDORA_RELEASES_URL "https://fedoraproject.org/releases.json"

const char *fedora_versions[64];
const char *fedora_architectures[64];
char *fedora_releases = NULL;

struct CURLResponse {
    char * html;
    size_t size;
};

static size_t WriteHTMLCallback(void * contents, size_t size, size_t nmemb, void * userp) {
    size_t realsize = size * nmemb;
    struct CURLResponse * mem = (struct CURLResponse * ) userp;
    char * ptr = realloc(mem -> html, mem -> size + realsize + 1);
    if (!ptr) return 0;
    mem -> html = ptr;
    memcpy( & (mem -> html[mem -> size]), contents, realsize);
    mem -> size += realsize;
    mem -> html[mem -> size] = 0;
    return realsize;
}

static struct CURLResponse GetHTML(const char * url) {
    CURL * curl = curl_easy_init();
    struct CURLResponse res = {
        .html = malloc(1),
        .size = 0
    };

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteHTMLCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void * ) & res);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode ret = curl_easy_perform(curl);

    if (ret != CURLE_OK) {
        char curl_error[256];
        sprintf(curl_error, "Failed to get %s: %s\n", url, curl_easy_strerror(ret));
        g_print("%s", curl_error);
        show_alert(main_window, curl_error);
    }

    //long response_code = 0;
    //curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    //g_print("HTTP response code: %ld\n", response_code);
    //g_print("HTML size: %zu bytes\n", res.size);

    curl_easy_cleanup(curl);
    return res;
}

static bool in_str_array(const char **array, const char *value) {
    for (size_t i = 0; array[i] != NULL; i++) {
        if (strcmp(array[i], value) == 0) {
            return true;
        }
    }
    return false;
}

const char **get_fedora_versions() {
    if (fedora_versions[0] != NULL) return fedora_versions;

    if (fedora_releases == NULL) {
        const char url[] = FEDORA_RELEASES_URL;
        struct CURLResponse res = GetHTML(url);
        fedora_releases = res.html;
    }

    cJSON *root = cJSON_Parse(fedora_releases);
    if (!root || !cJSON_IsArray(root)) {
        cJSON_Delete(root);
        fedora_versions[0] = strdup("43");
        fedora_versions[1] = NULL;
        return fedora_versions;
    }

    int versions_count = cJSON_GetArraySize(root);
    unsigned int version_idx = 0;
    for (int i = 0; i < versions_count; i++) {
        cJSON *releases_entry = cJSON_GetArrayItem(root, i);
        if (releases_entry && cJSON_IsObject(releases_entry)) {
            cJSON *rel_version = cJSON_GetObjectItem(releases_entry, "version");

            if (rel_version && cJSON_IsString(rel_version)) {
                if (!in_str_array(fedora_versions, rel_version->valuestring)) {
                    fedora_versions[version_idx] = strdup(rel_version->valuestring);
                    version_idx++;
                }
            }
        }
    }
    fedora_versions[version_idx] = NULL;
    cJSON_Delete(root);

    g_print("found %i versions\n", version_idx);

    return fedora_versions;
}

const char **get_fedora_architectures() {
    if (fedora_architectures[0] != NULL) return fedora_architectures;

    if (fedora_releases == NULL) {
        const char url[] = FEDORA_RELEASES_URL;
        struct CURLResponse res = GetHTML(url);
        fedora_releases = res.html;
    }

    cJSON *root = cJSON_Parse(fedora_releases);
    if (!root || !cJSON_IsArray(root)) {
        cJSON_Delete(root);
        fedora_architectures[0] = strdup("x86_64");
        fedora_architectures[1] = NULL;
        return fedora_architectures;
    }

    int architectures_count = cJSON_GetArraySize(root);
    unsigned int arch_idx = 0;
    for (int i = 0; i < architectures_count; i++) {
        cJSON *releases_entry = cJSON_GetArrayItem(root, i);
        if (releases_entry && cJSON_IsObject(releases_entry)) {
            cJSON *rel_arch = cJSON_GetObjectItem(releases_entry, "arch");

            if (rel_arch && cJSON_IsString(rel_arch)) {
                if (!in_str_array(fedora_architectures, rel_arch->valuestring)) {
                    fedora_architectures[arch_idx] = strdup(rel_arch->valuestring);
                    arch_idx++;
                }
            }
        }
    }
    fedora_architectures[arch_idx] = NULL;
    cJSON_Delete(root);

    g_print("found %i architectures\n", arch_idx);

    return fedora_architectures;
}

char *find_fedora_iso() {
    if (fedora_releases == NULL) {
        const char url[] = FEDORA_RELEASES_URL;
        struct CURLResponse res = GetHTML(url);
        fedora_releases = res.html;
    }

    cJSON *root = cJSON_Parse(fedora_releases);
    if (!root || !cJSON_IsArray(root)) {
        cJSON_Delete(root);
        return NULL;
    }

    int releases_count = cJSON_GetArraySize(root);
    for (int i = 0; i < releases_count; i++) {
        cJSON *releases_entry = cJSON_GetArrayItem(root, i);
        if (releases_entry && cJSON_IsObject(releases_entry)) {
            cJSON *rel_version = cJSON_GetObjectItem(releases_entry, "version");
            cJSON *rel_arch = cJSON_GetObjectItem(releases_entry, "arch");
            cJSON *rel_link = cJSON_GetObjectItem(releases_entry, "link");
            cJSON *rel_variant = cJSON_GetObjectItem(releases_entry, "variant");

            if (
                rel_version && cJSON_IsString(rel_version) && strcmp(rel_version->valuestring, gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.fedora_version))))) == 0
                && rel_arch && cJSON_IsString(rel_arch) && strcmp(rel_arch->valuestring, gtk_string_object_get_string(GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(options.arch))))) == 0
                && rel_link && cJSON_IsString(rel_link)
                && rel_variant && cJSON_IsString(rel_variant) && strcmp(rel_variant->valuestring, "Everything") == 0
            ) {
                char *link = strdup(rel_link->valuestring);
                cJSON_Delete(root);
                return link;
            }
        }
    }

    cJSON_Delete(root);

    g_print("failed to find suitable iso image\n");
    show_alert(main_window, "Failed to find suitable ISO image");

    return NULL;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

static int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    if (dltotal != 0) {
        double percentage = (double)dlnow / dltotal * 100.0;
        static double last_percentage = 0;
        if (percentage - last_percentage >= 1.0 || dlnow == dltotal) {
            g_print("download progress: %.1f%% (%ld/%ld bytes)\n",
                    percentage, dlnow, dltotal);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), percentage / 100.0);
            last_percentage = percentage;
        }
    }
    return 0;
}

int download_file(const char *url, const char *dest) {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(dest, "wb");
        if (!fp) {
            curl_easy_cleanup(curl);
            return 1;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);  // No timeout for large files
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 300L);  // 5 min connection timeout
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1024L);  // 1KB/s minimum
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 300L);  // Allow 5 min at low speed
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
        
        if (res != CURLE_OK) {
            g_print("download failed: %s\n", curl_easy_strerror(res));
            remove(dest);  // Delete incomplete file
            return 1;
        }
    } else {
        return 1;
    }
    return 0;
}

void free_fedora_releases() {
    if (fedora_releases) {
        free(fedora_releases);
        fedora_releases = NULL;
    }

    for (int i = 0; fedora_versions[i] != NULL; i++) {
        free((char*)fedora_versions[i]);
    }
    fedora_versions[0] = NULL;

    for (int i = 0; fedora_architectures[i] != NULL; i++) {
        free((char*)fedora_architectures[i]);
    }
    fedora_architectures[0] = NULL;
}

float parse_xorriso(const char *line) {
    const char *p = strstr(line, "Writing:");
    if (p == NULL) return -1.0;

    char value_str[16];
    int value_str_idx = 0;

    while (*p != 's') {
        p++;
    }

    for (int i = 0; p[i] != '\0'; i++) {
        if (p[i] == '%') {
            value_str[value_str_idx] = '\0';
            break;
        } else if (isdigit(p[i]) || p[i] == '.') {
            if (value_str_idx >= sizeof(value_str) - 1) {
                return -1.0;
            }

            value_str[value_str_idx] = p[i];
            value_str_idx++;
        }
    }

    float res;

    sscanf(value_str, "%f", &res);

    return res / 100.0;
}

int create_iso(const char *ks_path, const char *input_iso, const char *output_iso, bool overwrite) {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 0.0);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "Building ISO Image");

    const char *disk_label = gtk_editable_get_text(GTK_EDITABLE(options.disk_label));

    if (overwrite) {
        remove(output_iso);
    }

    char *create_iso_cmd = NULL;
    asprintf(
        &create_iso_cmd,
        "pkexec --keep-cwd mkksiso --ks \"%s\" --add \"%s/\" -V \"%s\" \"%s\" \"%s\" 2>&1",
        ks_path,
        get_pkg_dir(),
        (disk_label && strlen(disk_label) > 0) ? disk_label : "Fedora-Autokickstart",
        input_iso,
        output_iso
    );
    g_print("executing \"%s\"\n", create_iso_cmd);

    FILE *cmd = popen(create_iso_cmd, "r");
    if (!cmd) {
        perror("popen");
    }
    char *cmd_out = NULL;
    size_t outlen = 0;
    while (getline(&cmd_out, &outlen, cmd) >= 0) {
        g_print("%s", cmd_out);
        float parse_out = parse_xorriso(cmd_out);
        if (parse_out > 0.0 && parse_out <= 1.0) {
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), parse_out);
        }
    }
    int status = pclose(cmd);
    int exit_code = WEXITSTATUS(status);

    free(cmd_out);
    free(create_iso_cmd);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 1.0);
    return exit_code;
}