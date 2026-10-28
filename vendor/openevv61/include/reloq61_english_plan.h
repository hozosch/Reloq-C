/* Request-local linguistic data. Names are independent of phone-table IDs. */
#ifndef RELOQ61_ENGLISH_PLAN_H
#define RELOQ61_ENGLISH_PLAN_H
#include <stddef.h>
#include <stdint.h>
/* Orthographic context read from the native input projection. */
enum reloq61_english_source_flags {
 RELOQ_SOURCE_AFTER_A=1u<<11, RELOQ_SOURCE_HEAD_O=1u<<14,
 RELOQ_SOURCE_ROOT_AL=1u<<19,
 RELOQ_SOURCE_HEAD_TT=1u<<21
};
struct reloq61_english_segment {char name[4],base[4];int16_t stress;uint8_t origin,word;int32_t left,right;};
/* origin: bits 0..1 are the diaphone origin, 2..6 identify a spelled
 * Latin letter (A=1), and bit 7 is private to the reduction reader. */
struct reloq61_english_letter_mark {int32_t left,right;uint8_t letter;};
struct reloq61_english_spelling {uint8_t letters[256];size_t count;};
struct reloq61_english_capture {
    struct reloq61_english_segment *phones;
    size_t capacity,count;
    int failed,active,captured;
    size_t start;unsigned group;int32_t begin,end;
    struct reloq61_english_letter_mark *letters;size_t letter_count,letter_capacity;
    uint32_t *source_classes;
    unsigned (*classify_source)(const char *);
    int prefer_segments;
};
void *reloq61_english_plan_scope(struct reloq61_english_capture *);
void reloq61_english_lexical(void *,int32_t,int32_t);
int reloq61_english_letter_begin(void *,int32_t,int32_t,struct reloq61_english_letter_mark *);
void reloq61_english_letter_end(const struct reloq61_english_letter_mark *);
void reloq61_english_split_begin(void *,int32_t,int32_t);
void reloq61_english_projection(void *);
void reloq61_english_surface(void *);
int reloq61_english_word_plan(void *,const char *,char *,size_t,
    struct reloq61_english_segment *,size_t,size_t *,struct reloq61_english_spelling *);
int reloq61_english_word_context(void *,const char *,char *,size_t,
    struct reloq61_english_segment *,size_t,size_t *,struct reloq61_english_spelling *,
    uint32_t *,unsigned (*)(const char *),int);
int reloq61_english_structured_ready(void);
#endif
