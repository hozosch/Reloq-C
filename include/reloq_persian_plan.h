#ifndef RELOQ_PERSIAN_PLAN_H
#define RELOQ_PERSIAN_PLAN_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Language-owned events. They are independent of a particular acoustic
 * engine and contain no Finnish/English phoneme codes or inline tags. */
enum reloq_fa_plan_event_kind {
    RELOQ_FA_PLAN_WORD_BEGIN=1,
    RELOQ_FA_PLAN_PHONE=2,
    RELOQ_FA_PLAN_WORD_END=3,
    RELOQ_FA_PLAN_PUNCTUATION=4,
    RELOQ_FA_PLAN_WEAK_WORD=5,
    RELOQ_FA_PLAN_GLOTTAL_ONSET=6,
    RELOQ_FA_PLAN_INPUT_VARIANT=7
};

/* Persian follows Aragonese (19) with the next available language number.
 * Selection by `l20.0 or `l20.1 changes only the input variant of the same
 * Persian core. Standard ECI voice registration is not yet implemented. */
enum reloq_fa_input_variant {
    RELOQ_FA_VARIANT_PERSIAN=0,
    RELOQ_FA_VARIANT_PINGLISH=1
};
#define RELOQ_FA_DIALECT_PERSIAN 0x140000u
#define RELOQ_FA_DIALECT_PINGLISH 0x140001u

/* The vowel keeps its acoustic identity and any LONG flag, while its
 * nucleus uses the contextual timing of the same vowel's short version. */
#define RELOQ_FA_PLAN_SHORT_NUCLEUS 2u
/* A weak /i/ between a preceding vowel and a stressed vowel also receives
 * the established compact-hiatus timing, for either vowel-length marker. */
#define RELOQ_FA_PLAN_COMPACT_HIATUS 4u

/* The ordinary Persian inventory has 29 identities. These optional extra
 * identities preserve previously available rounded vowels and glides. */
enum reloq_fa_extra_phone_id {
    RELOQ_FA_PHONE_FRONT_ROUNDED_O=30,
    RELOQ_FA_PHONE_FRONT_ROUNDED_U=31,
    RELOQ_FA_PHONE_OFFGLIDE_I=32,
    RELOQ_FA_PHONE_OFFGLIDE_U=33,
    RELOQ_FA_PHONE_OFFGLIDE_FRONT_U=34
};

struct reloq_fa_plan_event {
    uint32_t kind;       /* enum reloq_fa_plan_event_kind */
    uint32_t value;      /* phone, punctuation scalar, or input variant */
    uint32_t flags;      /* LONG and/or Persian timing annotations */
    uint32_t syllable;   /* zero-based within the word */
    uint32_t stress;     /* 0 unstressed, 1 primary, 2 secondary */
};

/* The caller owns the resulting event array and releases it with free or
 * eciPersianRelease. Target offsets refer to event boundaries. Text and
 * run modes have the same contract as eciPersianProjectRunsUtf8. */
int reloq_fa_analyze_runs(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    struct reloq_fa_plan_event **events,size_t *count,size_t *target_events,size_t *error_offset);

#ifdef __cplusplus
}
#endif
#endif
