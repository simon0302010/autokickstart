#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Lang {
    const char *name;
    const char *id;
};

struct Lang locales[] = {
    { "Afrikaans (South Africa)", "af_ZA" },
    { "Albanian", "sq_AL.UTF-8" },
    { "Arabic (Algeria)", "ar_DZ.UTF-8" },
    { "Arabic (Bahrain)", "ar_BH.UTF-8" },
    { "Arabic (Egypt)", "ar_EG.UTF-8" },
    { "Arabic (India)", "ar_IN.UTF-8" },
    { "Arabic (Iraq)", "ar_IQ.UTF-8" },
    { "Arabic (Jordan)", "ar_JO.UTF-8" },
    { "Arabic (Kuwait)", "ar_KW.UTF-8" },
    { "Arabic (Lebanon)", "ar_LB.UTF-8" },
    { "Arabic (Libyan Arab Jamahiriya)", "ar_LY.UTF-8" },
    { "Arabic (Morocco)", "ar_MA.UTF-8" },
    { "Arabic (Oman)", "ar_OM.UTF-8" },
    { "Arabic (Qatar)", "ar_QA.UTF-8" },
    { "Arabic (Saudi Arabia)", "ar_SA.UTF-8" },
    { "Arabic (Sudan)", "ar_SD.UTF-8" },
    { "Arabic (Syrian Arab Republic)", "ar_SY.UTF-8" },
    { "Arabic (Tunisia)", "ar_TN.UTF-8" },
    { "Arabic (United Arab Emirates)", "ar_AE.UTF-8" },
    { "Arabic (Yemen)", "ar_YE.UTF-8" },
    { "Assamese (India)", "as_IN.UTF-8" },
    { "Asturian (Spain)", "ast_ES.UTF-8" },
    { "Basque (Spain)", "eu_ES.UTF-8" },
    { "Belarusian", "be_BY.UTF-8" },
    { "Bengali (Bangladesh)", "bn_BD.UTF-8" },
    { "Bengali (India)", "bn_IN.UTF-8" },
    { "Bosnian (Bosnia and Herzegowina)", "bs_BA" },
    { "Breton (France)", "br_FR" },
    { "Bulgarian  -  Български", "bg_BG.UTF-8" },
    { "Catalan (Spain)", "ca_ES.UTF-8" },
    { "Chinese (Hong Kong)", "zh_HK.UTF-8" },
    { "Chinese (P.R. of China)  -  中文(简体)", "zh_CN.UTF-8" },
    { "Chinese (Taiwan)  -  正體中文", "zh_TW.UTF-8" },
    { "Cornish (Britain)", "kw_GB.UTF-8" },
    { "Croatian", "hr_HR.UTF-8" },
    { "Czech  -  Česká republika", "cs_CZ.UTF-8" },
    { "Danish  -  Dansk", "da_DK.UTF-8" },
    { "Dutch (Belgium)", "nl_BE.UTF-8" },
    { "Dutch (Netherlands)", "nl_NL.UTF-8" },
    { "English (Australia)", "en_AU.UTF-8" },
    { "English (Botswana)", "en_BW.UTF-8" },
    { "English (Canada)", "en_CA.UTF-8" },
    { "English (Denmark)", "en_DK.UTF-8" },
    { "English (Great Britain)", "en_GB.UTF-8" },
    { "English (Hong Kong)", "en_HK.UTF-8" },
    { "English (India)", "en_IN.UTF-8" },
    { "English (Ireland)", "en_IE.UTF-8" },
    { "English (New Zealand)", "en_NZ.UTF-8" },
    { "English (Philippines)", "en_PH.UTF-8" },
    { "English (Singapore)", "en_SG.UTF-8" },
    { "English (South Africa)", "en_ZA.UTF-8" },
    { "English (USA)", "en_US.UTF-8" },
    { "English (Zimbabwe)", "en_ZW.UTF-8" },
    { "Estonian", "et_EE.UTF-8" },
    { "Faroese (Faroe Islands)", "fo_FO.UTF-8" },
    { "Finnish", "fi_FI.UTF-8" },
    { "French (Belgium)", "fr_BE.UTF-8" },
    { "French (Canada)", "fr_CA.UTF-8" },
    { "French (France)  -  Français", "fr_FR.UTF-8" },
    { "French (Luxemburg)", "fr_LU.UTF-8" },
    { "French (Switzerland)", "fr_CH.UTF-8" },
    { "Galician (Spain)", "gl_ES.UTF-8" },
    { "German (Austria)", "de_AT.UTF-8" },
    { "German (Belgium)", "de_BE.UTF-8" },
    { "German (Germany) -   Deutsch", "de_DE.UTF-8" },
    { "Low German (Germany) -   Deutsch", "nds_DE.UTF-8" },
    { "German (Luxemburg)", "de_LU.UTF-8" },
    { "German (Switzerland)", "de_CH.UTF-8" },
    { "Greek", "el_GR.UTF-8" },
    { "Greenlandic (Greenland)", "kl_GL.UTF-8" },
    { "Gujarati (India)", "gu_IN.UTF-8" },
    { "Hebrew (Israel)", "he_IL.UTF-8" },
    { "Hindi (India)", "hi_IN.UTF-8" },
    { "Hungarian", "hu_HU.UTF-8" },
    { "Icelandic  -  Íslenska", "is_IS.UTF-8" },
    { "Indonesian", "id_ID.UTF-8" },
    { "Interlingua", "ia_FR.UTF-8" },
    { "Irish", "ga_IE.UTF-8" },
    { "Italian (Italy) Italiano", "it_IT.UTF-8" },
    { "Italian (Switzerland)", "it_CH.UTF-8" },
    { "Japanese  -  日本語", "ja_JP.UTF-8" },
    { "Georgian", "ka_GE.UTF-8" },
    { "Kazakh", "kk_KZ.UTF-8" },
    { "Kazakh", "kk_KZ.UTF-8" },
    { "Kannada (India)", "kn_IN.UTF-8" },
    { "Kashmiri (India)", "ks_IN.UTF-8" },
    { "Korean (Republic of Korea)  -  한국어", "ko_KR.UTF-8" },
    { "Kyrgyz (Kyrgyzstan)", "ky_KG.UTF-8" },
    { "Lao (Laos)", "lo_LA.UTF-8" },
    { "Latvian (Latvia)", "lv_LV.UTF-8" },
    { "Lithuanian", "lt_LT.UTF-8" },
    { "Macedonian", "mk_MK.UTF-8" },
    { "Maithili (India)", "mai_IN.UTF-8" },
    { "Malayalam (India)", "ml_IN.UTF-8" },
    { "Malay (Malaysia)", "ms_MY.UTF-8" },
    { "Maltese (malta)", "mt_MT.UTF-8" },
    { "Manx Gaelic (Britain)", "gv_GB.UTF-8" },
    { "Marathi (India)", "mr_IN.UTF-8" },
    { "Northern Saami (Norway)", "se_NO" },
    { "Nepali (Nepal)", "ne_NP.UTF-8" },
    { "Norwegian  -  Norsk", "nb_NO.UTF-8" },
    { "Norwegian, Nynorsk (Norway)  -  Norsk", "nn_NO.UTF-8" },
    { "Occitan (France)", "oc_FR" },
    { "Odia (India)", "or_IN.UTF-8" },
    { "Persian (Iran)", "fa_IR.UTF-8" },
    { "Polish", "pl_PL.UTF-8" },
    { "Portuguese (Brasil)", "pt_BR.UTF-8" },
    { "Portuguese (Portugal)  -  Português", "pt_PT.UTF-8" },
    { "Punjabi (India)", "pa_IN.UTF-8" },
    { "Romanian", "ro_RO.UTF-8" },
    { "Russian  -  Русский", "ru_RU.UTF-8" },
    { "Russian (Ukraine)", "ru_UA.UTF-8" },
    { "Serbian", "sr_RS.UTF-8" },
    { "Serbian (Latin)", "sr_RS.UTF-8@latin" },
    { "Sinhala", "si_LK.UTF-8" },
    { "Slovak", "sk_SK.UTF-8" },
    { "Slovenian (Slovenia)  -  slovenščina", "sl_SI.UTF-8" },
    { "Spanish (Argentina)", "es_AR.UTF-8" },
    { "Spanish (Bolivia)", "es_BO.UTF-8" },
    { "Spanish (Chile)", "es_CL.UTF-8" },
    { "Spanish (Colombia)", "es_CO.UTF-8" },
    { "Spanish (Costa Rica)", "es_CR.UTF-8" },
    { "Spanish (Dominican Republic)", "es_DO.UTF-8" },
    { "Spanish (El Salvador)", "es_SV.UTF-8" },
    { "Spanish (Equador)", "es_EC.UTF-8" },
    { "Spanish (Guatemala)", "es_GT.UTF-8" },
    { "Spanish (Honduras)", "es_HN.UTF-8" },
    { "Spanish (Mexico)", "es_MX.UTF-8" },
    { "Spanish (Nicaragua)", "es_NI.UTF-8" },
    { "Spanish (Panama)", "es_PA.UTF-8" },
    { "Spanish (Paraguay)", "es_PY.UTF-8" },
    { "Spanish (Peru)", "es_PE.UTF-8" },
    { "Spanish (Puerto Rico)", "es_PR.UTF-8" },
    { "Spanish (Spain)  -  Español", "es_ES.UTF-8" },
    { "Spanish (USA)", "es_US.UTF-8" },
    { "Spanish (Uruguay)", "es_UY.UTF-8" },
    { "Spanish (Venezuela)", "es_VE.UTF-8" },
    { "Swedish (Finland)", "sv_FI.UTF-8" },
    { "Swedish (Sweden)  -  Svenska", "sv_SE.UTF-8" },
    { "Tagalog (Philippines)", "tl_PH" },
    { "Tamil (India)", "ta_IN.UTF-8" },
    { "Telugu (India)", "te_IN.UTF-8" },
    { "Thai", "th_TH.UTF-8" },
    { "Turkish", "tr_TR.UTF-8" },
    { "Ukrainian", "uk_UA.UTF-8" },
    { "Urdu (Pakistan)", "ur_PK" },
    { "Uzbek (Uzbekistan)", "uz_UZ" },
    { "Walloon (Belgium)", "wa_BE@euro" },
    { "Welsh (Great Britain)", "cy_GB.UTF-8" },
    { "Xhosa (South Africa)", "xh_ZA.UTF-8" },
    { "Zulu (South Africa)", "zu_ZA.UTF-8" },
    { NULL, NULL }
};

size_t locales_count = 0;

static void get_locales_count() {
    if (locales_count != 0) return;
    locales_count = 0;
    while (locales[locales_count].name != NULL) {
        locales_count++;
    }
}

const char *get_locale(const char* name) {
    for (size_t i = 0; locales[i].name != NULL; i++) {
        if (strcmp(locales[i].name, name) == 0) {
            return locales[i].id;
        }
    }
    return NULL;
}

const char **get_names() {
    get_locales_count();

    const char **names = malloc((locales_count + 1) * sizeof(char *));
    if (!names) return NULL;

    for (size_t i = 0; i < locales_count; i++) {
        names[i] = locales[i].name;
    }
    names[locales_count] = NULL;

    return names;
}

int get_current_system_locale_index() {
    get_locales_count();

    const char *system_locale = setlocale(LC_ALL, NULL);
    if (!system_locale) return -1;

    for (size_t i = 0; i < locales_count; i++) {
        if (strcmp(locales[i].id, system_locale) == 0) {
            return i;
        }
    }

    return -1;
}