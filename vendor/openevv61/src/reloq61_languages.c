#include "delta_lang.h"

extern delta_language delta_lang_enus61;
void delta_lang_bind_enus61(void);
extern delta_language delta_lang_dede61;
void delta_lang_bind_dede61(void);
extern delta_language delta_lang_ptbr;
void delta_lang_bind_ptbr(void);
extern delta_language delta_lang_fifi;
void delta_lang_bind_fifi(void);
extern delta_language delta_lang_engb61;
void delta_lang_bind_engb61(void);
extern delta_language delta_lang_eses61;
void delta_lang_bind_eses61(void);
extern delta_language delta_lang_esmx61;
void delta_lang_bind_esmx61(void);
extern delta_language delta_lang_frfr61;
void delta_lang_bind_frfr61(void);
extern delta_language delta_lang_frca61;
void delta_lang_bind_frca61(void);
extern delta_language delta_lang_itit61;
void delta_lang_bind_itit61(void);
extern delta_language delta_lang_jajp61;
void delta_lang_bind_jajp61(void);
extern delta_language delta_lang_kokr61;
void delta_lang_bind_kokr61(void);
extern delta_language delta_lang_zhcn61;
void delta_lang_bind_zhcn61(void);
extern delta_language delta_lang_zhtw61;
void delta_lang_bind_zhtw61(void);

extern delta_language delta_lang_elgr61;
void delta_lang_bind_elgr61(void);

const delta_language *const delta_languages[] = {
    &delta_lang_enus61,
    &delta_lang_dede61,
    &delta_lang_ptbr,
    &delta_lang_fifi,
    &delta_lang_engb61,
    &delta_lang_eses61,
    &delta_lang_esmx61,
    &delta_lang_frfr61,
    &delta_lang_frca61,
    &delta_lang_itit61,
    &delta_lang_jajp61,
    &delta_lang_kokr61,
    &delta_lang_zhcn61,
    &delta_lang_zhtw61,
    &delta_lang_elgr61,
    0,
};

void delta_lang_bind_all(void)
{
    delta_lang_bind_enus61();
    delta_lang_bind_dede61();
    delta_lang_bind_ptbr();
    delta_lang_bind_fifi();
    delta_lang_bind_engb61();
    delta_lang_bind_eses61();
    delta_lang_bind_esmx61();
    delta_lang_bind_frfr61();
    delta_lang_bind_frca61();
    delta_lang_bind_itit61();
    delta_lang_bind_jajp61();
    delta_lang_bind_kokr61();
    delta_lang_bind_zhcn61();
    delta_lang_bind_zhtw61();
    delta_lang_bind_elgr61();
}

int reloq61_language_supported(int language)
{
    switch(language) {
    case 0x10000:
    case 0x10001:
    case 0x20000:
    case 0x20001:
    case 0x30000:
    case 0x30001:
    case 0x40000:
    case 0x50000:
    case 0x70000:
    case 0x90000:
    case 0x80000:
    case 0xa0000:
    case 0x60000:
    case 0x60001:
    case 0x120000:
        return 1;
    default:return 0;
    }
}
