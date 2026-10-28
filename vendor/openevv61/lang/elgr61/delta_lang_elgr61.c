/* Greek owns a language identity and state. The Castilian acoustic rules and
 * immutable tables are shared; character names are replaced inside its native
 * text analysis, rather than added to the caller's input. */
#include <string.h>
#include "delta_lang.h"
extern delta_language delta_lang_eses61;
delta_language delta_lang_elgr61;
static char greek_settings[4096];
void delta_lang_bind_elgr61(void)
{
    delta_lang_elgr61=delta_lang_eses61;
    delta_lang_elgr61.tag="elgr61";
    delta_lang_elgr61.name="Greek";
    delta_lang_elgr61.id=0x120000;
    delta_lang_elgr61.library_name="Static Engine Greek";
    /* Keep every voice and phoneme setting; only the section selecting this
       independently addressed language differs from the acoustic parent. */
    const char *s=delta_lang_eses61.ini;
    size_t used=0;
    for(int i=0;i<delta_lang_eses61.ini_size;) {
        if(i+5<=delta_lang_eses61.ini_size&&!memcmp(s+i,"[2.0]",5)) {
            memcpy(greek_settings+used,"[18.0]",6);used+=6;i+=5;
        } else greek_settings[used++]=s[i++];
    }
    delta_lang_elgr61.ini=greek_settings;
    delta_lang_elgr61.ini_size=(int32_t)used;
}
