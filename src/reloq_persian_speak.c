/* This translation unit is linked into the ECI library itself. Its ECI
 * calls are local definitions, not imports from a second Windows DLL. */
#ifndef RELOQ_ECI_STATIC
#define RELOQ_ECI_STATIC
#endif
#include "reloq_persian_speak.h"
#include "reloq_utf8.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void reloq_fa_prepared_clear(struct reloq_fa_prepared *prepared)
{
    if(!prepared)return;
    reloq_fa_backend_clear(&prepared->backend);
    free(prepared->positions);
    *prepared=(struct reloq_fa_prepared){0};
}

/* The old acoustic parser accepts an explicit group with multiple vowel
 * nuclei under one stress marker, but that representation cannot yet be
 * reconstructed exactly from language-owned syllable events. Decline it
 * before output so a host can keep its established phonetic-input path. */
static int reloq_fa_explicit_contour_safe(const char *text,size_t bytes)
{
    if(bytes<4 || text[0]!='`' || text[1]!='[' || text[bytes-1]!=']')return 1;
    size_t markers=0,vowels=0;
    for(size_t i=2;i+1<bytes;) {
        if(text[i]=='.' && i+2<bytes && text[i+1]>='0' && text[i+1]<='2') {
            ++markers;i+=2;
        } else if(text[i]=='\'' && i+4<bytes && strchr("AaeiouOy",text[i+1]) &&
                  text[i+2]==':' && text[i+3]=='\'') {
            ++vowels;i+=4;
        } else {
            if(strchr("AaeiouOy",text[i]))++vowels;
            ++i;
        }
    }
    return vowels<=1 || markers>=vowels;
}

int reloq_fa_prepare_native_runs(const char *text,size_t bytes,int variant,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,size_t index_count,size_t *error_offset,
    struct reloq_fa_prepared *result,char **donor_text)
{
    if(error_offset)*error_offset=0;
    if(donor_text)*donor_text=NULL;
    if(!result)return -1;
    *result=(struct reloq_fa_prepared){0};
    if(!text || !error_offset || bytes==SIZE_MAX || (variant!=0 && variant!=1) ||
       (index_count && !source_offsets) || index_count>SIZE_MAX/sizeof(size_t) ||
       memchr(text,0,bytes))return -1;
    unsigned char *types=index_count?malloc(index_count):NULL;
    size_t *positions=index_count?malloc(index_count*sizeof *positions):NULL;
    if(index_count && (!types || !positions)){free(types);free(positions);return -2;}
    if(types)memset(types,1,index_count);
    struct reloq_fa_plan_event *events=NULL;size_t count=0;
    int status=eciPersianAnalyzeRunsCompleteUtf8(text,bytes,variant,
        run_ends,run_modes,run_count,
        source_offsets,types,index_count,&events,&count,positions,error_offset);
    free(types);
    if(status){free(positions);return status==3?-1:status>0?1:status;}
    /* Do not replace a known English Unicode name by a numeric fallback,
     * or give Persian letter names to Latin spelling in the Persian profile.
     * Those cases need a separate English language event. */
    size_t start=0;
    for(size_t r=0;donor_text && r<run_count && !status;++r) {
        size_t end=run_ends[r];
        if(run_modes[r]==2 && !reloq_fa_explicit_contour_safe(text+start,end-start)) {
            *error_offset=start;status=1;
        }
        if(run_modes[r]==0)for(size_t i=start;i+1<end && !status;++i) {
            if(text[i]!='`' || text[i+1]!='[')continue;
            size_t close=i+2;
            while(close<end && text[close]!=']' && text[close]!=' ' &&
                  text[close]!='\t' && text[close]!='\n')++close;
            if(close<end && text[close]==']' &&
               !reloq_fa_explicit_contour_safe(text+i,close+1-i)) {
                *error_offset=i;status=1;break;
            }
            i=close;
        }
        if(run_modes[r]==1)for(size_t at=start;at<end;) {
            size_t first=at;uint32_t cp=0;char name[2048];int kind=0;
            if(reloq_utf8_next((const unsigned char *)text,end,&at,&cp)!=1 ||
               eciPersianCharacterResolve(cp,name,sizeof name,&kind)!=1) {
                *error_offset=first;status=-1;break;
            }
            if(kind || ((variant==0 || memchr(text,'`',bytes)) &&
               ((cp>='A' && cp<='Z') || (cp>='a' && cp<='z')))) {
                *error_offset=first;status=1;break;
            }
        }
        start=end;
    }
    for(size_t i=0;donor_text && i<count && !status;++i)if(events[i].kind==RELOQ_FA_PLAN_ENGLISH_TEXT) {
        *error_offset=events[i].value;status=1;
    }
    struct reloq_fa_backend_plan plan={0},donor={0};size_t event_error=0;
    if(!status)status=reloq_fa_backend_prepare(text,bytes,events,count,&plan,&event_error);
    if(!status && donor_text)
        status=reloq_fa_backend_prepare_donor(text,bytes,events,count,&donor,&event_error);
    eciPersianRelease(events);
    if(status){reloq_fa_backend_clear(&plan);reloq_fa_backend_clear(&donor);free(positions);return status;}
    for(size_t i=0;i<index_count;++i) {
        if(positions[i]>count){reloq_fa_backend_clear(&plan);reloq_fa_backend_clear(&donor);free(positions);return -1;}
        positions[i]=plan.offsets[positions[i]];
    }
    if(donor_text){*donor_text=donor.text;donor.text=NULL;}
    reloq_fa_backend_clear(&donor);
    *result=(struct reloq_fa_prepared){plan,positions,index_count};
    return 0;
}

int reloq_fa_prepare_native(const char *text,size_t bytes,int variant,
    const size_t *source_offsets,size_t index_count,size_t *error_offset,
    struct reloq_fa_prepared *result)
{
    if(error_offset)*error_offset=0;
    if(!result)return -1;
    *result=(struct reloq_fa_prepared){0};
    if(!text || !error_offset || bytes==SIZE_MAX || (variant!=0 && variant!=1) ||
       (index_count && !source_offsets) || index_count>SIZE_MAX/sizeof(size_t) ||
       memchr(text,0,bytes))return -1;
    if(memchr(text,'`',bytes))return 1;
    size_t end=bytes;unsigned char mode=0;
    return reloq_fa_prepare_native_runs(text,bytes,variant,bytes?&end:NULL,
        bytes?&mode:NULL,bytes?1:0,source_offsets,index_count,error_offset,result,NULL);
}

int reloq_fa_speak_prepared(ECIHand voice,const struct reloq_fa_prepared *prepared,
    const size_t *positions,const int *index_values,const unsigned char *pause,
    size_t index_count)
{
    if(!voice || !prepared || !prepared->backend.text ||
       (index_count && (!positions || !index_values)))return -1;
    const struct reloq_fa_backend_plan *plan=&prepared->backend;
    for(size_t i=0;i<index_count;++i)
        if(positions[i]>plan->length || (i && positions[i]<positions[i-1]))return -1;
    int dialect=eciGetParam(voice,RELOQ_ECI_LANGUAGE_DIALECT);
    if(dialect<0 || (dialect&~0xff00)!=0x90000 || eciSpeaking(voice))return -1;
    int status=0;
    int old_tags=eciGetParam(voice,RELOQ_ECI_PROCESS_TAGS);
    if(old_tags<0 || eciSetParam(voice,RELOQ_ECI_PROCESS_TAGS,1)<0)return -1;
    int previous=eciPersianAcousticScope(1);
    if(previous<0){status=-1;goto restore_tags;}
    if(previous!=0){(void)eciPersianAcousticScope(previous);status=-1;goto restore_tags;}
    if(eciPersianNativeOnsets(plan->onsets,plan->onset_count) ||
       eciPersianNativeGeminates(plan->geminates,plan->geminate_count)) {
        status=-1;goto restore_scope;
    }
    size_t cursor=0;
    for(size_t i=0;i<index_count;++i) {
        size_t next=positions[i];
        if(next<cursor || next>plan->length ||
           !eciAddTextUtf8(voice,plan->text+cursor,next-cursor,0) ||
           !eciInsertIndex(voice,index_values[i])) {status=-3;goto discard;}
        if(pause && pause[i] && !eciAddTextUtf8(voice," ",1,0)) {status=-3;goto discard;}
        cursor=next;
    }
    if(!eciAddTextUtf8(voice,plan->text+cursor,plan->length-cursor,0) ||
       !eciSynthesize(voice)){status=-3;goto discard;}
    status=0;goto restore_scope;
discard:
    (void)eciClearInput(voice);
restore_scope:
    (void)eciPersianNativeOnsets(NULL,0);
    (void)eciPersianNativeGeminates(NULL,0);
    (void)eciPersianAcousticScope(previous);
restore_tags:
    (void)eciSetParam(voice,RELOQ_ECI_PROCESS_TAGS,old_tags);
    return status;
}

int reloq_fa_speak_native(ECIHand voice,const char *text,size_t bytes,int variant,
    const size_t *source_offsets,const int *index_values,size_t index_count,
    size_t *error_offset)
{
    if(error_offset)*error_offset=0;
    if(!voice || !text || !error_offset || bytes==SIZE_MAX || (variant!=0 && variant!=1) ||
       (index_count && (!source_offsets || !index_values)) ||
       index_count>SIZE_MAX/sizeof(size_t) || memchr(text,0,bytes))return -1;
    if(memchr(text,'`',bytes))return 1;
    int dialect=eciGetParam(voice,RELOQ_ECI_LANGUAGE_DIALECT);
    if(dialect<0 || (dialect&~0xff00)!=0x90000 || eciSpeaking(voice))return -1;
    struct reloq_fa_prepared prepared={0};
    int status=reloq_fa_prepare_native(text,bytes,variant,source_offsets,index_count,
        error_offset,&prepared);
    if(status)return status;
    status=reloq_fa_speak_prepared(voice,&prepared,prepared.positions,index_values,NULL,index_count);
    reloq_fa_prepared_clear(&prepared);return status;
}
