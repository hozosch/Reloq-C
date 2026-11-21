/* This translation unit is linked into the ECI library itself. Its ECI
 * calls are local definitions, not imports from a second Windows DLL. */
#ifndef RELOQ_ECI_STATIC
#define RELOQ_ECI_STATIC
#endif
#include "reloq_persian_speak.h"
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
        if(positions[i]>count){reloq_fa_backend_clear(&plan);free(positions);return -1;}
        positions[i]=plan.offsets[positions[i]];
    }
    *result=(struct reloq_fa_prepared){plan,positions,index_count};
    return 0;
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
