#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <gtk/gtk.h>

#include "kickstart.h"

unsigned int fedora_versions[64];
const char *fedora_releases = NULL;

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
    
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    g_print("HTTP response code: %ld\n", response_code);
    g_print("HTML size: %zu bytes\n", res.size);
    
    curl_easy_cleanup(curl);
    return res;
}

unsigned int *get_fedora_versions() {
    const char url[] = "https://fedoraproject.org/releases.json";
    struct CURLResponse res = GetHTML(url);

    FILE *debug_file = fopen("/tmp/releases.json", "w");
    if (debug_file) {
        fwrite(res.html, 1, res.size, debug_file);
        fclose(debug_file);
        g_print("HTML saved to /tmp/releases.json\n");
    }

    free(res.html);

    return fedora_versions;
}