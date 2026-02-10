#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "locales.h"

struct Layout {
    const char *name;
    const char *id;
};

struct Layout layouts[] = {
    { "Albanian", "al" },
    { "Amharic", "et" },
    { "Arabic (Egypt)", "eg" },
    { "Arabic (Iraq)", "iq" },
    { "Arabic (Morocco)", "ma" },
    { "Arabic (Syria)", "sy" },
    { "Armenian", "am" },
    { "Azerbaijani", "az" },
    { "Bambara", "ml" },
    { "Bangla", "bd" },
    { "Belarusian", "by" },
    { "Belgian", "be" },
    { "Berber (Algeria, Latin)", "dz" },
    { "Bosnian", "ba" },
    { "Bulgarian", "bg" },
    { "Burmese", "mm" },
    { "Chinese", "cn" },
    { "Croatian", "hr" },
    { "Czech", "cz" },
    { "Danish", "dk" },
    { "Dari", "af" },
    { "Dhivehi", "mv" },
    { "Dutch", "nl" },
    { "Dzongkha", "bt" },
    { "English (Australia)", "au" },
    { "English (Cameroon)", "cm" },
    { "English (Ghana)", "gh" },
    { "English (New Zealand)", "nz" },
    { "English (Nigeria)", "ng" },
    { "English (South Africa)", "za" },
    { "English (UK)", "gb" },
    { "English (US)", "us" },
    { "Estonian", "ee" },
    { "Faroese", "fo" },
    { "Filipino", "ph" },
    { "Finnish", "fi" },
    { "French", "fr" },
    { "French (Canada)", "ca" },
    { "French (Democratic Republic of the Congo)", "cd" },
    { "French (Togo)", "tg" },
    { "Georgian", "ge" },
    { "German", "de" },
    { "German (Austria)", "at" },
    { "German (Switzerland)", "ch" },
    { "Greek", "gr" },
    { "Hebrew", "il" },
    { "Hungarian", "hu" },
    { "Icelandic", "is" },
    { "Indian", "in" },
    { "Indonesian (Latin)", "id" },
    { "Irish", "ie" },
    { "Italian", "it" },
    { "Japanese", "jp" },
    { "Kazakh", "kz" },
    { "Khmer (Cambodia)", "kh" },
    { "Korean", "kr" },
    { "Kyrgyz", "kg" },
    { "Lao", "la" },
    { "Latvian", "lv" },
    { "Lithuanian", "lt" },
    { "Macedonian", "mk" },
    { "Malay (Jawi, Arabic Keyboard)", "my" },
    { "Maltese", "mt" },
    { "Moldavian", "md" },
    { "Mongolian", "mn" },
    { "Montenegrin", "me" },
    { "N'Ko (AZERTY)", "gn" },
    { "Nepali", "np" },
    { "Norwegian", "no" },
    { "Persian", "ir" },
    { "Polish", "pl" },
    { "Portuguese", "pt" },
    { "Portuguese (Brazil)", "br" },
    { "Romanian", "ro" },
    { "Russian", "ru" },
    { "Serbian", "rs" },
    { "Sinhala (phonetic)", "lk" },
    { "Slovak", "sk" },
    { "Slovenian", "si" },
    { "Spanish", "es" },
    { "Swahili (Kenya)", "ke" },
    { "Swahili (Tanzania)", "tz" },
    { "Swedish", "se" },
    { "Taiwanese", "tw" },
    { "Tajik", "tj" },
    { "Thai", "th" },
    { "Tswana", "bw" },
    { "Turkish", "tr" },
    { "Turkmen", "tm" },
    { "Ukrainian", "ua" },
    { "Urdu (Pakistan)", "pk" },
    { "Uzbek", "uz" },
    { "Vietnamese", "vn" },
    { "Wolof", "sn" },
};

size_t layout_count = 0;

static void get_layout_count() {
    if (layout_count != 0) return;
    layout_count = 0;
    while (layouts[layout_count].name != NULL) {
        layout_count++;
    }
}

const char *get_layout_name(const char* name) {
    for (size_t i = 0; layouts[i].name != NULL; i++) {
        if (strcmp(layouts[i].name, name) == 0) {
            return layouts[i].id;
        }
    }
    return NULL;
}

int get_layout_idx_by_id(const char* id) {
    for (size_t i = 0; layouts[i].id != NULL; i++) {
        if (strcmp(layouts[i].id, id) == 0) {
            return i;
        }
    }
    return -1;
}

const char **get_layout_names() {
    get_layout_count();

    const char **names = malloc((layout_count + 1) * sizeof(char *));
    if (!names) return NULL;

    for (size_t i = 0; i < layout_count; i++) {
        names[i] = layouts[i].name;
    }
    names[layout_count] = NULL;

    return names;
}

const char **get_layout_ids() {
    get_layout_count();

    const char **ids = malloc((layout_count + 1) * sizeof(char *));
    if (!ids) return NULL;

    for (size_t i = 0; i < layout_count; i++) {
        ids[i] = layouts[i].id;
    }
    ids[layout_count] = NULL;

    return ids;
}

int find_layout_index(const char *search) {
    get_layout_count();

    const char **ids = get_layout_ids();
    if (!ids) return -1;

    for (size_t i = 0; i < layout_count; i++) {
        if (strstr(ids[i], search)) {
            free(ids);
            return i;
        }
    }

    free(ids);
    return -1;
}

int find_current_system_layout_index(const char *alternative) {
    int system_locale = get_current_system_locale_index(alternative);
    if (system_locale == -1) return find_layout_index(alternative);
    const char *system_locale_id = get_locale_id(system_locale);
    if (!system_locale_id) return find_layout_index(alternative);

    if (strlen(system_locale_id) < 5) return find_layout_index(alternative);

    char layout_str[3];
    layout_str[0] = tolower(system_locale_id[3]);
    layout_str[1] = tolower(system_locale_id[4]);
    layout_str[2] = '\0';

    int layout_idx = get_layout_idx_by_id(layout_str);
    if (layout_idx == -1) return find_layout_index(alternative);
    return layout_idx;
}