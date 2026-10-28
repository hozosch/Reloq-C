#ifndef RELOQ_ENGLISH_NUCLEUS_H
#define RELOQ_ENGLISH_NUCLEUS_H

#include <stddef.h>
#include <stdint.h>

/* Independent US acoustic nucleus. Symbols are the public phone alphabet,
 * with Y/W/O expanded to a+y/a+w/c+y by the caller. Syllabic m/n have an
 * explicit flag. quantity distinguishes reduced R from full stressed R.
 * closed says a consonant follows the nucleus in this syllable; word_final
 * and following describe the boundary after it (syllabic n before n has a
 * separate budget). find derives closed/following within the input, keeping
 * caller-supplied context beyond its end. Input is one adjusted syllable. */
struct reloq_english_nucleus_phone {
    unsigned char phone, syllabic, quantity, closed, word_final, following;
};
struct reloq_english_nucleus {
    size_t first, count;
    int32_t base_ticks;
    int32_t closing_ticks[2]; /* member steady parts; transitions are separate */
    int32_t transition_ticks[2]; /* internal vowel/glide/liquid transitions */
};
/* Public native speed 40..250 -> duration-stage integer percent. */
int reloq_english_nucleus_rate_percent(int native_speed);
/* Locate a vowel, optionally its closing y/w/r/l, then one closing r/l. Return
 * 1 on success, 0 for no described nucleus, -1 for invalid input. No output
 * changes on failure. This does not resyllabify or look up words. */
int reloq_english_nucleus_find(const struct reloq_english_nucleus_phone *,
    size_t count,struct reloq_english_nucleus *);
int reloq_english_nucleus_base(const struct reloq_english_nucleus_phone *,
    size_t count,struct reloq_english_nucleus *);
/* Phrase-final primary-stressed, non-reduced multipart nucleus ending the
 * word, with no following coda. Other prosodic contexts are not covered. */
int reloq_english_nucleus_final_percent(
    const struct reloq_english_nucleus_phone *,size_t count);
/* Primary accented word-final nucleus at a full phrase boundary. The
 * optional following coda is one ordinary consonant; complete nuclei are
 * budgeted before their incoming/outgoing intervals are distributed. */
int reloq_english_nucleus_terminal_budget(
    const struct reloq_english_nucleus_phone *,size_t count,
    unsigned char coda,double *ticks);
/* Accented monosyllable before another word in the same phrase. */
int reloq_english_nucleus_medial_budget(
    const struct reloq_english_nucleus_phone *,size_t count,
    unsigned char coda,unsigned char next_word_onset,double *ticks);
/* Same-syllable onset r/l into a full vowel. -1 for other contexts. */
int reloq_english_nucleus_liquid_onset_ticks(unsigned char left,unsigned char head);
/* Sonorant member to an ordinary consonant in the same syllable. */
int reloq_english_nucleus_exit_ticks(unsigned char member,unsigned char following);
/* A vowel word boundary owns two successive transition portions. */
int reloq_english_nucleus_hiatus_ticks(unsigned char left,unsigned char right,
    int32_t *outgoing,int32_t *incoming);

/* Allocate a phrase-adjusted budget to one free portion and fixed acoustic
 * intervals. The two shortage passes and per-interval rounding precede
 * the minima. tick unit is one millisecond at the neutral duration clock.
 * Caller owns storage; input/output arrays must not overlap. */
struct reloq_english_nucleus_interval { int32_t ticks, minimum; };
int reloq_english_nucleus_distribute(
    const struct reloq_english_nucleus_interval *,size_t count,
    double budget,int32_t free_minimum,int32_t *fixed,int32_t *free_ticks);

#endif
