#include "reloq_persian_speak.h"
#include "reloq_persian_backend.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
    unsigned char *types=index_count?malloc(index_count):NULL;
    size_t *positions=index_count?malloc(index_count*sizeof *positions):NULL;
    if(index_count && (!types || !positions)){free(types);free(positions);return -2;}
    if(types)memset(types,1,index_count);
    size_t run_end=bytes;unsigned char run_mode=0;
    struct reloq_fa_plan_event *events=NULL;size_t count=0;
    int status=eciPersianAnalyzeRunsCompleteUtf8(text,bytes,variant,
        bytes?&run_end:NULL,bytes?&run_mode:NULL,bytes?1:0,
        source_offsets,types,index_count,&events,&count,positions,error_offset);
    free(types);
    if(status){free(positions);return status==3?-1:status>0?1:status;}
    struct reloq_fa_backend_plan plan={0};size_t event_error=0;
    status=reloq_fa_backend_prepare(text,bytes,events,count,&plan,&event_error);
    eciPersianRelease(events);
    if(status){free(positions);return status;}
    for(size_t i=0;i<index_count;++i) {
        if(positions[i]>count){status=-1;goto cleanup;}
        positions[i]=plan.offsets[positions[i]];
    }
    int old_tags=eciGetParam(voice,RELOQ_ECI_PROCESS_TAGS);
    if(old_tags<0 || eciSetParam(voice,RELOQ_ECI_PROCESS_TAGS,1)<0){status=-1;goto cleanup;}
    int previous=eciPersianAcousticScope(1);
    if(previous<0){status=-1;goto restore_tags;}
    if(previous!=0){(void)eciPersianAcousticScope(previous);status=-1;goto restore_tags;}
    if(eciPersianNativeOnsets(plan.onsets,plan.onset_count) ||
       eciPersianNativeGeminates(plan.geminates,plan.geminate_count)) {
        status=-1;goto restore_scope;
    }
    size_t cursor=0;
    for(size_t i=0;i<index_count;++i) {
        size_t next=positions[i];
        if(next<cursor || next>plan.length ||
           !eciAddTextUtf8(voice,plan.text+cursor,next-cursor,0) ||
           !eciInsertIndex(voice,index_values[i])) {status=-3;goto discard;}
        cursor=next;
    }
    if(!eciAddTextUtf8(voice,plan.text+cursor,plan.length-cursor,0) ||
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
cleanup:
    reloq_fa_backend_clear(&plan);free(positions);return status;
}
