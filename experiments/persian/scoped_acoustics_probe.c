/* Isolated experiment: per-calling-thread selection of the approved Finnish
 * acoustic probe. It is not an ECI language or an NVDA integration. The
 * caller must restore the returned previous scope even after an error. */
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#define FA_EXPORT __declspec(dllexport)
#else
#define FA_EXPORT __attribute__((visibility("default")))
#endif

static _Thread_local int fa_acoustics;
/* Word ordinals with a reference-pause onset, private to the calling
 * synthesis thread. The Finnish word record uses a signed 16-bit ordinal. */
static _Thread_local unsigned char fa_onsets[4096];

FA_EXPORT int reloq_fa_acoustic_scope(int active)
{
    if(active!=0 && active!=1)return -1;
    int previous=fa_acoustics;
    fa_acoustics=active;
    return previous;
}

FA_EXPORT int reloq_fa_native_onsets(const uint32_t *words,size_t count)
{
    if(!fa_acoustics || (count && !words) || count>32767)return -1;
    for(size_t i=0;i<count;++i)
        if(words[i]==0 || words[i]>32767)return -1;
    memset(fa_onsets,0,sizeof fa_onsets);
    for(size_t i=0;i<count;++i)
        fa_onsets[words[i]>>3]|=(unsigned char)(1u<<(words[i]&7));
    return 0;
}

int reloq_fa_scoped_native_onset(int word)
{
    return fa_acoustics && word>0 && word<=32767 &&
           (fa_onsets[(unsigned)word>>3] & (1u<<((unsigned)word&7)))!=0;
}

/* The generated provider sources ask for their existing probe keys through
 * this function. Nothing is read from or written to the process environment. */
const char *reloq_fa_scoped_get(const char *name)
{
    static const struct {const char *key,*value;} selected[]={
        {"RELOQ_FA_PROBE_A","1"}, {"RELOQ_FA_PROBE_Q","7"},
        {"RELOQ_FA_PROBE_X","1"}, {"RELOQ_FA_PROBE_RISE","2"},
        {"RELOQ_FA_PROBE_H","5"}, {"RELOQ_FA_PROBE_L","4"},
        {"RELOQ_FA_PROBE_VOWEL","1"}, {"RELOQ_FA_PROBE_S","1"},
        {"RELOQ_FA_PROBE_Z","2"}, {"RELOQ_FA_PROBE_D","2"},
        {"RELOQ_FA_PROBE_A_TARGET","salam-fixed"}, {"RELOQ_FA_PROBE_FINAL","1"},
        {"RELOQ_FA_PROBE_GLOTTAL","2"}, {"RELOQ_FA_PROBE_END_H","1"},
        {"RELOQ_FA_PROBE_A_NASAL","1"}, {"RELOQ_FA_PROBE_GLOT_ONSET","1"},
        {"RELOQ_FA_PROBE_END_H_DB","9"}, {"RELOQ_FA_PROBE_K_SHAPE","3"},
        {"RELOQ_FA_E_MODE","1"}, {"RELOQ_FA_E_CONTEXT","22"},
        {"RELOQ_FA_E_TARGET","450,1875,2525"},
        {"RELOQ_FA_PROBE_NO_AUTO_GLOTTAL","1"},
    };
    if(!fa_acoustics || !name)return 0;
    for(unsigned i=0;i<sizeof selected/sizeof selected[0];++i)
        if(!strcmp(name,selected[i].key))return selected[i].value;
    return 0;
}
