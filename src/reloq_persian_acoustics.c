/* Per-calling-thread selection of the approved Persian acoustic rules.
 * Callers must restore the returned scope even when synthesis fails. */
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(RELOQ_FA_PROBE_EXPORT) && defined(_WIN32)
#define FA_EXPORT __declspec(dllexport)
#elif defined(RELOQ_FA_PROBE_EXPORT)
#define FA_EXPORT __attribute__((visibility("default")))
#else
#define FA_EXPORT
#endif

static _Thread_local int fa_acoustics;
/* Word ordinals with a reference-pause onset, private to the calling
 * synthesis thread. The Finnish word record uses a signed 16-bit ordinal. */
static _Thread_local unsigned char fa_onsets[4096];
/* Sorted word/affricate ordinals, owned by the calling synthesis thread. */
static _Thread_local uint32_t *fa_geminates;
static _Thread_local size_t fa_geminate_count;

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

FA_EXPORT int reloq_fa_native_geminates(const uint32_t *pairs,size_t count)
{
    if(!fa_acoustics || (count && !pairs) || count>32767)return -1;
    for(size_t i=0;i<count;++i) {
        unsigned word=pairs[i]>>16,phone=pairs[i]&65535u;
        if(!word || word>32767 || !phone || (i && pairs[i]<=pairs[i-1]))return -1;
    }
    uint32_t *copy=count?malloc(count*sizeof *copy):NULL;
    if(count&&!copy)return -1;
    if(count)memcpy(copy,pairs,count*sizeof *copy);
    free(fa_geminates);
    fa_geminates=copy;fa_geminate_count=count;
    return 0;
}

int reloq_fa_scoped_has_geminates(void)
{return fa_acoustics && fa_geminate_count>0;}

int reloq_fa_scoped_native_geminate(unsigned word,unsigned phone)
{
    if(!fa_acoustics || !word || word>32767 || !phone || phone>65535)return 0;
    uint32_t key=(word<<16)|phone;
    size_t lo=0,hi=fa_geminate_count;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if(fa_geminates[mid]<key)lo=mid+1;else hi=mid;
    }
    return lo<fa_geminate_count && fa_geminates[lo]==key;
}

/* The generated provider sources ask for their existing probe keys through
 * this function. Nothing is read from or written to the process environment. */
const char *reloq_fa_scoped_get(const char *name)
{
    static const struct {const char *key,*value;} selected[]={
        {"RELOQ_FA_PROBE_A","1"}, {"RELOQ_FA_PROBE_Q","7"},
        {"RELOQ_FA_PROBE_X","1"}, {"RELOQ_FA_PROBE_RISE","2"},
        {"RELOQ_FA_PROBE_H","0"}, {"RELOQ_FA_PROBE_L","4"},
        {"RELOQ_FA_PROBE_VOWEL","1"}, {"RELOQ_FA_PROBE_S","3"},
        {"RELOQ_FA_PROBE_TIMING","0"},
        {"RELOQ_FA_FRONT_A_ACH","1"},
        {"RELOQ_FA_BRITISH_FRONT_A","1"},
        {"RELOQ_FA_PROBE_Z","3"}, {"RELOQ_FA_PROBE_D","2"},
        {"RELOQ_FA_PROBE_A_TARGET","salam-fixed"}, {"RELOQ_FA_PROBE_FINAL","1"},
        {"RELOQ_FA_PROBE_GLOTTAL","2"}, {"RELOQ_FA_PROBE_END_H","1"},
        {"RELOQ_FA_PROBE_A_NASAL","0"}, {"RELOQ_FA_PROBE_GLOT_ONSET","1"},
        {"RELOQ_FA_VOWEL_DAMPING","1"}, {"RELOQ_FA_FRONT_A_F3","1"},
        {"RELOQ_FA_US_ARTICULATION","1"}, {"RELOQ_FA_US_STOPS","1"},
        {"RELOQ_FA_US_FRICATIVES","1"}, {"RELOQ_FA_US_NASALS","1"},
        {"RELOQ_FA_US_GLIDE","1"},
        {"RELOQ_FA_BRAZILIAN_CODA_R","1"},
        {"RELOQ_FA_UNCOLORED_R","1"},
        {"RELOQ_FA_PROBE_END_H_DB","9"}, {"RELOQ_FA_PROBE_K_SHAPE","3"},
        /* Reproduce the previous "se" (three) E trajectory, including its
         * darker onset. Preserve the inherited upper formants and timing. */
        {"RELOQ_FA_E_MODE","3"}, {"RELOQ_FA_E_CONTEXT","0"},
        {"RELOQ_FA_E_TARGET","450,1900,2000,2550,2700"},
        {"RELOQ_FA_PROBE_NO_AUTO_GLOTTAL","1"},
    };
    if(!fa_acoustics || !name)return 0;
    for(unsigned i=0;i<sizeof selected/sizeof selected[0];++i)
        if(!strcmp(name,selected[i].key))return selected[i].value;
    return 0;
}
