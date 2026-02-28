#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <gtk/gtk.h>
#include <string.h>

#include "cJSON/cJSON.h"

#define MAX_VERSIONS 64

const char *fedora_versions[64];
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
        g_print("CURL error: %s\n", curl_easy_strerror(ret));
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
        const char url[] = "https://fedoraproject.org/releases.json";
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
    g_print("found %i versions\n", versions_count);
    unsigned int version_idx = 0;
    for (int i = 0; i < versions_count; i++) {
        cJSON *releases_entry = cJSON_GetArrayItem(root, i);
        if (releases_entry && cJSON_IsObject(releases_entry)) {
            cJSON *rel_version = cJSON_GetObjectItem(releases_entry, "version");
            cJSON *rel_arch = cJSON_GetObjectItem(releases_entry, "arch");
            cJSON *rel_link = cJSON_GetObjectItem(releases_entry, "link");
            cJSON *rel_variant = cJSON_GetObjectItem(releases_entry, "variant");

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

    for (int i = 0; fedora_versions[i] != NULL; i++) {
        g_print("%s\n", fedora_versions[i]);
    }

    return fedora_versions;
}