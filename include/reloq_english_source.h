#ifndef RELOQ_ENGLISH_SOURCE_H
#define RELOQ_ENGLISH_SOURCE_H

#include <stddef.h>
#include <stdint.h>

/* Phonetic facts and already planned clocks. No word spellings or model
 * labels enter the source planner. A boundary belongs to its right phone. */
struct reloq_english_source_phone {
    unsigned char phone,stress,aspirated,word_start,syllable_start,pause,before_h;
    int64_t start,contact,end,release;
};

struct reloq_english_source_span {
    size_t owner,last;
    int64_t first,end;
    unsigned char nucleus,early,unaspirated_stop,voiced_sonorant;
};

/* One span per phone. Closing liquids/glides refer to their nucleus owner.
 * Invalid input leaves the output untouched. */
int reloq_english_source_plan(const struct reloq_english_source_phone *,size_t,
    struct reloq_english_source_span *);

/* An unaspirated stop closing the preceding word has no 50/70-ms h segment.
 * Negative means this branch does not determine the transition. */
int reloq_english_source_boundary_contact(unsigned char stop,int word_boundary);

/* Nominal closure voicing of b/d/g in the 11-kHz US source profile.
 * Frication on either side and preceding voiceless closure suppress it;
 * a preceding nasal retains the stronger voice bar. The d-after-l branch
 * uses the lateral closure. Zero neighbors are phrase boundaries.
 * Returns -1 for other phones or an unrepresented h boundary. */
int reloq_english_stop_closure_av(unsigned char phone,unsigned char previous,
    unsigned char following);

/* Complete do_son_amps window from scalar duration intervals, including its
 * one-tick delayed onset. Useful independently of the production clock plan. */
int reloq_english_source_window(int64_t contact,int64_t before_contact,
    int64_t span_ticks,int64_t *first,int64_t *end);

/* Compatibility with sparse amplitude targets: a nucleus member inherits
 * its owner's envelope; an independently voiced consonant needs a plateau
 * when its envelope is absent before silence or contradicts its source role. */
int reloq_english_source_needs_plateau(unsigned char phone,
    const struct reloq_english_source_span *,int has_positive,int has_zero,
    int next_silent,int previous_sonorant);

#endif
